/**
 *  ToPhp.h
 * 
 *  Class that we use to convert a variable from javascript context
 *  back into PHP context
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
#include <phpcpp.h>
#include <v8.h>

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class ToPhp
{
private:
    /**
     *  The PHP value
     *  @var Php::Value
     */
    Php::Value _value;

public:
    /**
     *  Constructor
     *  @param  isolate
     *  @param  value
     */
    ToPhp(v8::Isolate *isolate, const v8::Handle<v8::Value> &value);
    
    /**
     *  Destructor
     */
    virtual ~ToPhp() = default;
    
    /**
     *  Cast to the underlying PHP value
     *  @return Php::Value
     */
    operator const Php::Value& () const { return _value; }
};
    
/**
 *  End of namespace
 */
}

