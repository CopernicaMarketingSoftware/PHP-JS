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
#include "php_exception.h"
#include "script.h"
#include "names.h"

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
 *  Wrap a certain PHP object into a javascript object
 *  @param  object      MUST be an array or object!
 *  @return v8::Local<v8::Value>
 */
v8::Local<v8::Value> Core::wrap(const Php::Value &object)
{
    // if the object is already known to be a JS\Object
    auto *instance = PhpBase::unwrap(this, object);
    
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
    // avoid that other contexts are assigned
    if (value.instanceOf(Names::Context)) return false;
    
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

    // store the value
    v8::Maybe<bool> result = global->Set(scope, FromPhp(_isolate, name), FromPhp(_isolate, value));
    
    // check for success
    return result.IsJust() && result.FromJust();
}

/**
 *  Parse a piece of javascript code
 *  @param  source      the code to execute
 *  @param  timeout     possible timeout in seconds
 *  @return Php::Value
 *  @throws Php::Exception
 */
Php::Value Core::evaluate(const Php::Value &source, const Php::Value &timeout)
{
    // create a script
    Script script(shared_from_this(), source.rawValue());
    
    // evaluate the script
    return script.execute(timeout);
}
    
/**
 *  End of namespace
 */
}

