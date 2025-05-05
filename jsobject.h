/**
 *  jsobject.h
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
#include <phpcpp.h>
#include <v8.h>

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Forward declarations
 */
class Context;

/**
 *  Class definition
 */
class JSObject : public Php::Base, public Php::Traversable
{
private:
    /**
     *  The context in which we operate
     *  @var std::shared_ptr<Context>
     */
    std::shared_ptr<Context> _context;

    /**
     *  The underlying ecmascript object
     *  @var    v8::Global<v8::Object>
     */
    v8::Global<v8::Object> _object;

public:
    /**
     *  Constructor
     *  @param  context     The context
     *  @param  object      The ecmascript object
     */
    JSObject(const std::shared_ptr<Context> &context, const v8::Local<v8::Object> &object);

    /**
     *  No copying
     *  @param  that
     */
    JSObject(const JSObject &that) = delete;

    /**
     *  Destructor
     */
    virtual ~JSObject();

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

