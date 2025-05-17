/**
 *  Core.cpp
 * 
 *  Implementation file for the context class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "core.h"
#include "fromphp.h"
#include "php_variable.h"
#include "scope.h"
#include "php_object.h"
#include "timeout.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 */
Core::Core() : _platform(Platform::instance()), _isolate(this)
{
    // when we access the isolate, we need a scope
    v8::HandleScope scope(_isolate);
    
    // create a context
    v8::Local<v8::Context> context(v8::Context::New(_isolate));

    // we want to persist the context
    _context.Reset(_isolate, context);
    
    // symbol for linking js and php objects together
    v8::Local<v8::Private> key = v8::Private::ForApi(_isolate, v8::String::NewFromUtf8Literal(_isolate, "js2php"));
    
    // persist the key
    _symbol.Reset(_isolate, key);
}

/**
 *  Destructor
 */
Core::~Core()
{
    // @todo cleanup of global members?
    
    
}

/**
 *  Release the context after it is no longer in use by a Js\Context object
 */
void Core::release()
{
    // @todo some initial cleanup?
    
    
}

/**
 *  Wrap a certain PHP object into a javascript object
 *  @param  object      MUST be an array or object!
 *  @return v8::Local<v8::Value>
 */
v8::Local<v8::Value> Core::wrap(const Php::Value &object)
{
    // if the object is already known to be a JS\Object
    auto *instance = PhpObject::unwrap(this, object);
    
    // was this possible? then we reuse the original handle
    if (instance != nullptr) return instance->handle();
    
    // check the prototypes that we have
    for (auto &prototype : _templates)
    {
        // is this one compatible with the object
        if (!prototype->matches(object)) continue;
        
        // we can apply this prototype
        return prototype->apply(object);
    }
    
    // we need a new template
    _templates.emplace_back(new Template(_isolate, object));
    
    // use it
    return _templates.back()->apply(object);
}

/**
 *  Assign a variable to the javascript context
 *  @param  name        name of property to assign  required
 *  @param  value       value to be assigned
 *  @param  attribytes  property attributes
 *  @return Php::Value
 */
Php::Value Core::assign(const Php::Value &name, const Php::Value &value, const Php::Value &attributes)
{
    // @todo avoid that other contexts are assigned
    
    // scope for the context
    Scope scope(shared_from_this());

    // retrieve the global object from the context
    v8::Local<v8::Object> global(scope.global());

    // the attribute for the newly assigned property
    // @todo enable this?
    //v8::PropertyAttribute   attribute(v8::None);
    //
    //// if an attribute was given, assign it
    //if (params.size() > 2)
    //{
    //    // check the attribute that was selected
    //    switch ((int16_t)params[2])
    //    {
    //        case v8::None:          attribute = v8::None;       break;
    //        case v8::ReadOnly:      attribute = v8::ReadOnly;   break;
    //        case v8::DontDelete:    attribute = v8::DontDelete; break;
    //        case v8::DontEnum:      attribute = v8::DontEnum;   break;
    //    }
    //}

    // get the value
    // @todo why this var?
    FromPhp value2(_isolate, value);

    // and store the value
    v8::Maybe<bool> result = global->Set(scope, FromPhp(_isolate, name), value2); //FromPhp(shared_from_this(), value));
    
    // check for success
    return result.IsJust() && result.FromJust();
}

/**
 *  Parse a piece of javascript code
 *  @param  script      the code to execute
 *  @param  timeout     possible timeout in seconds
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value Core::evaluate(const Php::Value &script, const Php::Value &timeout)
{
    // retrieve the optional timeout variable
    // @todo optionally enable timeouts
    //int timeout = (params.size() >= 2 ? params[1].numericValue() : 0);

    // scope for the isolate
    v8::Isolate::Scope iscope(_isolate);

    // stack-allocated handle scope
    v8::HandleScope hscope(_isolate);
    
    // we need the context in a local handle
    v8::Local<v8::Context> context(_context.Get(_isolate));
    
    // enter the context for compiling and running the script
    v8::Context::Scope cscope(context);

    // catch any errors that occur while either compiling or running the script
    v8::TryCatch catcher(_isolate);
    
    // @todo force script to be a string?
    
    // compile the code into a script
    // @todo what does the ToLocalChecked() stuff? what happens on failure?
    v8::Local<v8::String> source = v8::String::NewFromUtf8(_isolate, script.rawValue()).ToLocalChecked();

    // get the compiled program
    // @todo should we be checking for errors?
    v8::Local<v8::Script> compiled = v8::Script::Compile(context, source).ToLocalChecked();

    // install a timeout
    Timeout timer(_isolate, timeout);

    // Run the script to get the result.
    v8::MaybeLocal<v8::Value> result = compiled->Run(context);

    // if no exception occured we're done
    if (!catcher.HasCaught()) return result.IsEmpty() ? Php::Value(nullptr) : PhpVariable(_isolate, result.ToLocalChecked());

    // if we have terminated we just throw a fixed error message as the catcher.Message()
    // method won't return anything useful (in fact it'll return nothing meaning we just segfault)
    if (catcher.HasTerminated()) throw Php::Exception("Execution timed out");

    // retrieve the message describing the problem
    v8::Local<v8::Message>  message(catcher.Message());
    v8::Local<v8::String>   description(message->Get());

    // convert the description to utf so we can dump it
    v8::String::Utf8Value   string(_isolate, description);

    // pass this exception on to PHP userspace
    throw Php::Exception(std::string(*string, string.length()));
}
    
/**
 *  End of namespace
 */
}

