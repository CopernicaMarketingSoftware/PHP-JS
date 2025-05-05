/**
 *  FromPhp.cpp
 * 
 *  Implementation file for the FromPhp class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "fromphp.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  isolate
 *  @param  value
 */
FromPhp::FromPhp(v8::Isolate *isolate, const Php::Value &value)
{
    // check the type
    switch (value.type()) {
    case Php::Type::Null:       _value = v8::Null(isolate); return;
    case Php::Type::Numeric:    _value = v8::Integer::New(isolate, value); return;
    case Php::Type::Float:      _value = v8::Number::New(isolate, value); return;
    case Php::Type::Bool:       _value = v8::Boolean::New(isolate, value); return;
    case Php::Type::True:       _value = v8::Boolean::New(isolate, true); return;
    case Php::Type::False:      _value = v8::Boolean::New(isolate, false); return;
    case Php::Type::String:     _value = v8::String::NewFromUtf8(isolate, value).ToLocalChecked(); return;
    default: break;
    }

    // @todo implementation for array, object and callable
}

/**
 *  End of namespace
 */
}

