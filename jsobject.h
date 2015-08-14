/**
 *  jsobject.h
 *
 *  Class that wraps around an ecmascript object
 *  and makes it available to PHP userspace.
 *
 *  @copyright 2015 Copernica B.V.
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
#include "stack.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Class definition
 */
class JSObject :
    public Php::Base,
    public Php::Traversable
{
private:
    /**
     *  The underlying ecmascript object
     *  @var    Stack<v8::Object>
     */
    Stack<v8::Object> _object;
public:
    /**
     *  Class definition
     */
    class Iterator : public Php::Iterator
    {
    private:
        /**
         *  The underlying ecmascript object
         *  @var    Stack<v8::Object>
         */
        Stack<v8::Object> _object;

        /**
         *  All properties in the object
         *  @var    Stack<v8::Array>
         */
        Stack<v8::Array> _keys;

        /**
         *  Current position in the object
         *  @var    uint32_t
         */
        uint32_t _position;

        /**
         *  Number of valid keys
         *  @var    uint32_t
         */
        uint32_t _size;
    public:
        /**
         *  Constructor
         *
         *  @param  base    The base that PHP-CPP insists on
         *  @param  object  The object to iterate
         */
        Iterator(Php::Base *base, const Stack<v8::Object> &object);

        /**
         *  Is the iterator still valid?
         *
         *  @return is an element present at the current offset
         */
        bool valid() override;

        /**
         *  Retrieve the current value
         *
         *  @return value at current offset
         */
        Php::Value current() override;

        /**
         *  Retrieve the current key
         *
         *  @return the current key
         */
        Php::Value key() override;

        /**
         *  Move ahead to the next item
         */
        void next() override;

        /**
         *  Start over at the beginning
         */
        void rewind() override;
    };

    /**
     *  Constructor
     *
     *  @param  object  The ecmascript object
     */
    JSObject(v8::Handle<v8::Object> object);

    /**
     *  Retrieve a property
     *
     *  @param  name    Name of the property
     *  @return The requested property
     */
    Php::Value __get(const Php::Value &name) const;

    /**
     *  Change a property
     *
     *  @param  name        Name of the property
     *  @param  property    New value for the property
     */
    void __set(const Php::Value &name, const Php::Value &property);

    /**
     *  Check if a property is set
     *
     *  @param  name        Name of the property
     *  @return Is the property set
     */
    bool __isset(const Php::Value &name);

    /**
     *  Call a function
     *
     *  @param  name        Name of the function to call
     *  @param  params      The input parameters
     *  @return The result of the function call
     */
    Php::Value __call(const char *name, Php::Parameters &params);

    /**
     *  Retrieve the iterator
     *
     *  @return The iterator
     */
    Php::Iterator *getIterator() override;

    /**
     *  Retrieve the original ecmascript value
     *
     *  @return original ecmascript value
     */
    v8::Local<v8::Object> object() const;
};

/**
 *  End namespace
 */
}
