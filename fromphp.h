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
#include "context.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class FromPhp
{
private:
    /**
     *  The wrapped variable
     *  @var 
     */
    v8::Local<v8::Value> _value;
    
public:
    /**
     *  Constructor
     *  @param  context
     *  @param  value
     */
    FromPhp(const std::shared_ptr<Context> &context, const Php::Value &value);
    
    /**
     *  Destructor
     */
    virtual ~FromPhp() = default;
    
    /**
     *  Expose the local value
     *  @return  v8::Local<v8::Value>
     */
    const v8::Local<v8::Value> &local() const { return _value; }
    
    /**
     *  Cast to a local value
     *  @return v8::Local<v8::Value>
     */
    operator const v8::Local<v8::Value>& () const { return _value; }
};
    
/**
 *  End of namespace
 */
}

