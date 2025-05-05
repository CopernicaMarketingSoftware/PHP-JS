/**
 *  ToPhp.cpp
 * 
 *  Implementation file to convert a v8/javascript variable into its
 *  PHP counterpart
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "tophp.h"
#include "jsobject.h"
#include "context.h"
#include "linker.h"
#include "phparray.h"


#include <iostream>

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  context
 *  @param  input
 */
ToPhp::ToPhp(const std::shared_ptr<Context> &context, const v8::Local<v8::Value> &input)
{
    // if we received an invalid input we simply keep empty PHP value
    if (input.IsEmpty() || input->IsNull() || input->IsUndefined()) return;

    // check for boolean true/false or a javascript "Boolean" instance
    if (input->IsBoolean() || input->IsBooleanObject()) 
    {
        // convert the value to a boolean
        v8::Local<v8::Boolean> value(v8::Local<v8::Boolean>::Cast(input));

        // expose the value
        _value = value->Value();
    }
    else if (input->IsInt32())
    {
        // convert the value to an int32
        v8::Local<v8::Int32> value(v8::Local<v8::Int32>::Cast(input));

        // expose the value
        _value = value->Value();
    }
    else if (input->IsNumber() || input->IsNumberObject())
    {
        // convert the value to a number
        v8::Local<v8::Number> value(v8::Local<v8::Number>::Cast(input));
        
        // expose the value
        _value = value->Value();
    }
    else if (input->IsString())
    {
        // convert input to a string
        v8::Local<v8::String> value(v8::Local<v8::String>::Cast(input));

        // convert to 
        v8::String::Utf8Value utf8(context->isolate(), value);
        
        // expose to the php value
        _value = Php::Value(*utf8, utf8.length());
    }
    else if (input->IsStringObject())
    {
        // convert input to a string
        v8::Local<v8::StringObject> value(v8::Local<v8::StringObject>::Cast(input));

        // convert to 
        v8::String::Utf8Value utf8(context->isolate(), value);
        
        // expose to the php value
        _value = Php::Value(*utf8, utf8.length());
    }
    else if (input->IsRegExp())
    {
        // convert input to a regexp
        v8::Local<v8::RegExp> value(v8::Local<v8::RegExp>::Cast(input));

        // convert to 
        v8::String::Utf8Value utf8(context->isolate(), value);
        
        // expose to the php value
        _value = Php::Value(*utf8, utf8.length());
    }
    else if (input->IsFunction())
    {
        // @todo to be implemented
        
    }
    else if (input->IsArray())
    {
        // @todo this is a new option, previously handled via IsObject, maybe add feature-flag?
        
        
        // convert input to a string
        v8::Local<v8::Array> value(v8::Local<v8::Array>::Cast(input));

        // we have a helper class for filling arrays
        _value = PhpArray(context, value);
    }
    else if (input->IsObject())
    {
        // retrieve the object
        auto object = input.As<v8::Object>();

        // use a linker to check if the object was already associated with a php::value
        Linker linker(context->isolate(), context->symbol(), object);
        
        // if already linked
        if (linker.valid()) _value = linker.value();

        // otherwise we associate the object now
        else _value = linker.attach(Php::Object("JS\\Object", new JSObject(context, object)));
    }



    /*

    // as is typical in javascript, a value can be of many types
    // check the type of value that we have received so we can cast
    if (input->IsBoolean())         return input->BooleanValue();
    if (input->IsBooleanObject())   return input->BooleanValue();
    if (input->IsInt32())           return input->Int32Value();
    if (input->IsNumber())          return input->NumberValue();
    if (input->IsNumberObject())    return input->NumberValue();
    if (input->IsNull())            return nullptr;
    if (input->IsUndefined())       return nullptr;

    // special treatment for string-like types
    // TODO: javascript dates might possibly be cast to a DateTime object
    if (input->IsString() || input->IsStringObject() || input->IsRegExp())
    {
        // create the utf8 value (the only way to retrieve the content)
        v8::String::Utf8Value   utf8(input->ToString());

        // and create the value to return
        return {*utf8, utf8.length()};
    }

    // it could be callable too
    if (input->IsFunction())
    {
        // create the function as a pointer that can be captured
        auto function = std::make_shared<Stack<v8::Function>>(input.As<v8::Function>());

        // the result to return
        Php::Function result([function](Php::Parameters &params) {
            // create a "scope", so variables get destructed, retrieve the context and "enter" it
            v8::HandleScope                     scope(Isolate::get());
            v8::Local<v8::Context>              context((*function)->CreationContext());
            v8::Context::Scope                  contextScope(context);

            // catch any errors that occur while either compiling or running the script
            v8::TryCatch                        catcher;

            // create a new array with parameters
            std::vector<v8::Local<v8::Value>>   array;
            array.reserve(params.size());

            // iterate over all the given parameters and add them to the arrau
            for (auto &param: params) array.push_back(value(param));

            // now we can actually call the function
            v8::Local<v8::Value> result((*function)->Call(context->Global(), array.size(), array.data()));

            // did we catch an exception?
            if (catcher.HasCaught())
            {
                // retrieve the message describing the problem
                v8::Local<v8::Message>  message(catcher.Message());
                v8::Local<v8::String>   description(message->Get());

                // convert the description to utf so we can dump it
                v8::String::Utf8Value   string(description);

                // pass this exception on to PHP userspace
                throw Php::Exception(std::string(*string, string.length()));
            }

            // convert the result to a PHP value and return it
            return value(result);
        });

        // now return the result
        return result;
    }

    // or perhaps an object
    if (input->IsObject())
    {
        // retrieve the object and the first internal field
        auto object = input.As<v8::Object>();

        // does the object have internal fields?
        if (object->InternalFieldCount())
        {
            // retrieve the field
            auto field  = object->GetInternalField(0);

            // does it have an internal field and is it external? we are converting back
            // an original PHP object, just retrieve the original thing that came from PHP
            if (!field.IsEmpty() && field->IsExternal())
            {
                // the PHP value is stored in the first internal field,
                // retrieve it and create the handle around it
                Handle handle(field);

                // dereference and return it
                return *handle;
            }
        }

        // create a new js object and convert it to userspace
        return Php::Object("JS\\Object", new JSObject(object));
    }

    // we sadly don't support this type of value
    return nullptr;
    
    */
}

/**
 *  End of namespace
 */
}


