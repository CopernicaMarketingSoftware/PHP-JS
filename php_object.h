/**
 *  PhpObject.h
 *
 *  Class that wraps around an ecmascript object and makes it available to PHP
 *  userspace.
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2025 Copernica B.V.
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "php_base.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Class definition
 */
class PhpObject : public PhpBase, public Php::Traversable
{
public:
    /**
     *  Constructor
     *  @param  isolate     The isolate
     *  @param  object      The ecmascript object
     */
    PhpObject(v8::Isolate *isolate, const v8::Local<v8::Object> &object) :
        PhpBase(isolate, object) {}

    /**
     *  No copying
     *  @param  that
     */
    PhpObject(const PhpObject &that) = delete;

    /**
     *  Destructor
     */
    virtual ~PhpObject() = default;
    
    /**
     *  Retrieve a property
     *  @param  name    Name of the property
     *  @return The requested property
     */
    Php::Value __get(const Php::Value &name) const;

    /**
     *  Change a property
     *  @param  name        Name of the property
     *  @param  property    New value for the property
     */
    void __set(const Php::Value &name, const Php::Value &property);

    /**
     *  Check if a property is set
     *  @param  name        Name of the property
     *  @return Is the property set
     */
    bool __isset(const Php::Value &name);

    /**
     *  Call a function
     *  @param  name        Name of the function to call
     *  @param  params      The input parameters
     *  @return The result of the function call
     */
    Php::Value __call(const char *name, Php::Parameters &params);

    /**
     *  Cast to a string
     *  @return The result of the string conversion
     */
    Php::Value __toString();

    /**
     *  Retrieve the iterator
     *  @return The iterator
     */
    virtual Php::Iterator *getIterator() override;
};

/**
 *  End namespace
 */
}

