/**
 *  Exception.h
 * 
 *  Utility class to turn a PHP space exception into a JS exception
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Exception : public v8::Local<v8::Value>
{
public:
    /**
     *  Constructor
     *  @param  isolate
     *  @param  exception
     */
    Exception(v8::Isolate *isolate, const Php::Exception &exception) :
        v8::Local<v8::Value>(v8::Exception::Error(v8::String::NewFromUtf8(isolate, exception.what()).ToLocalChecked())) {}
};
    
/**
 *  End of namespace
 */
}

