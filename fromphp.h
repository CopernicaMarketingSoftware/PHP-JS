/**
 *  FromPhp.h
 * 
 *  Class to convert a variable from php space into javascript context
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <v8.h>
#include <phpcpp.h>

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class FromPhp : public v8::Local<v8::Value>
{
   public:
    /**
     *  Constructor
     *  @param  isolate
     *  @param  value
     */
    FromPhp(v8::Isolate *isolate, const Php::Value &value);
    
    /**
     *  Destructor
     */
    virtual ~FromPhp() = default;
    
    /**
     *  Expose base class assignment
     */
    using v8::Local<v8::Value>::operator=;
};
    
/**
 *  End of namespace
 */
}

