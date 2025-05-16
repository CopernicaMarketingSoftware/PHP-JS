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
#include "core.h"

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
    case Php::Type::Null:       operator=(v8::Null(isolate)); return;
    case Php::Type::Numeric:    operator=(v8::Integer::New(isolate, value)); return;
    case Php::Type::Float:      operator=(v8::Number::New(isolate, value)); return;
    case Php::Type::Bool:       operator=(v8::Boolean::New(isolate, value)); return;
    case Php::Type::True:       operator=(v8::Boolean::New(isolate, true)); return;
    case Php::Type::False:      operator=(v8::Boolean::New(isolate, false)); return;
    case Php::Type::String:     operator=(v8::String::NewFromUtf8(isolate, value).ToLocalChecked()); return;
    case Php::Type::Object:     operator=(Core::upgrade(isolate)->wrap(value)); return;
    case Php::Type::Array:      operator=(Core::upgrade(isolate)->wrap(value)); return;
    default:                    operator=(v8::Undefined(isolate)); return;
    }
    
    // @todo do we also need Callable? the old code did support this!
}

/**
 *  End of namespace
 */
}

