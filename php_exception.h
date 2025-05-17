/**
 *  PhpException.h
 * 
 *  Helper class to convert an exception from javascript-space into an
 *  exception for php-space
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
class PhpException : public Php::Exception
{
private:
    /**
     *  Constructor
     *  @param  message
     */
    PhpException(const v8::String::Utf8Value &message) :
        Php::Exception(std::string(*message, message.length())) {}

    /**
     *  Constructor
     *  @param  isolate
     *  @param  message
     */
    PhpException(v8::Isolate *isolate, const v8::Local<v8::String> &message) :
        PhpException(v8::String::Utf8Value(isolate, message)) {}

    /**
     *  Constructor
     *  @param  isolate
     *  @param  message
     */
    PhpException(v8::Isolate *isolate, const v8::Local<v8::Message> &message) :
        PhpException(isolate, message->Get()) {}
    
public:
    /**
     *  Constructor
     *  @param  isolate
     *  @param  catcher
     */
    PhpException(v8::Isolate *isolate, const v8::TryCatch &catcher) :
        PhpException(isolate, catcher.Message()) {}
        
    /**
     *  Destructor
     */
    virtual ~PhpException() = default;
};
    
/**
 *  End of namespace
 */
}

