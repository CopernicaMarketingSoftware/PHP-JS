/**
 *  value.cpp
 *
 *  Simple value casting functions for casting values
 *  between php and ecmascript runtime values.
 *
 *  @copyright 2015 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "value.h"
#include "isolate.h"
#include "handle.h"
#include "object.h"
#include "array.h"
#include "jsobject.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Callback function to be used when invoking functions
 *  defined from the PHP side
 *
 *  @param  info    callback information
 */
static void callback(const v8::FunctionCallbackInfo<v8::Value> &info)
{
    // create a local handle, so properties "fall out of scope"
    v8::HandleScope     scope(Isolate::get());

    // retrieve handle to the original object
    Handle<Php::Value>  handle(info.Data());

    // an array to hold all the arguments
    Php::Array arguments;

    // add all the arguments
    for (int i = 0; i < info.Length(); ++i) arguments.set(i, value(info[i]));

    // catch any exceptions the PHP code might throw
    try
    {
        // now execute the function
        Php::Value result(Php::call("call_user_func_array", handle, arguments));

        // cast the value and set it as return parameter
        info.GetReturnValue().Set(value(result));
    }
    catch (const Php::Exception& exception)
    {
        // pass the exception on to javascript userspace
        Isolate::get()->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(Isolate::get(), exception.what())));
    }
}

/**
 *  Cast a PHP runtime value to an ecmascript value
 *
 *  @param  input   the value to cast
 *  @return v8::Handle<v8::Value>
 */
v8::Handle<v8::Value> value(const Php::Value &input)
{
    // create a handle that we can return a value from
    v8::EscapableHandleScope    scope(Isolate::get());

    // the result value we are assigning
    v8::Local<v8::Value>        result;

    // are we dealing with a value originally from ecmascript?
    if (input.instanceOf("JS\\Object"))
    {
        // cast the input to the original object
        result = static_cast<JSObject*>(input.implementation())->object();
    }
    else
    {
        // the value can be of many types
        switch (input.type())
        {
            case Php::Type::Null:       /* don't set anything, let it be empty */                                                               break;
            case Php::Type::Numeric:    result = v8::Integer::New(Isolate::get(), input);                                                       break;
            case Php::Type::Float:      result = v8::Number::New(Isolate::get(), input);                                                        break;
            case Php::Type::Bool:       result = v8::Boolean::New(Isolate::get(), input);                                                       break;
            case Php::Type::String:     result = v8::String::NewFromUtf8(Isolate::get(), input);                                                break;
            case Php::Type::Object:     result = Object(input);                                                                                 break;
            case Php::Type::Callable:   result = v8::FunctionTemplate::New(Isolate::get(), callback, Handle<Php::Value>(input))->GetFunction(); break;
            case Php::Type::Array:      result = Array(input);                                                                                  break;
            default:
                break;
        }
    }

    // return the value by "escaping" it
    return scope.Escape(result);
}

/**
 *  Cast an ecmascript value to a PHP runtime value
 *
 *  @note   The value cannot be const, as retrieving properties
 *          from arrays and objects cannot be done on const values
 *
 *  @param  input   the value to cast
 *  @return Php::Value
 */
Php::Value value(v8::Handle<v8::Value> input)
{
    // if we received an invalid input we simply return an empty PHP value
    if (input.IsEmpty())            return nullptr;

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
                Handle<Php::Object> handle(field);

                // dereference and return it
                return *handle;
            }
        }

        // create a new js object and convert it to userspace
        return Php::Object("JS\\Object", new JSObject(object));
    }

    // we sadly don't support this type of value
    return nullptr;
}

/**
 *  End namespace
 */
}
