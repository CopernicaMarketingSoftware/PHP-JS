/**
 *  handle.h
 *
 *  A simple class that represents a handle managed by
 *  the v8 engine. Give it any object, and it will be
 *  either copied or moved and destructed when v8 no
 *  longer has any references to it.
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
#include <v8.h>
#include <utility>
#include <phpcpp.h>
#include "isolate.h"
#include "external.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Handle class
 */
class Handle
{
private:

    /**
     *  The v8 handle to the object
     *  @var    v8::Handle<v8::External>
     */
    v8::Local<v8::External> _handle;

    /**
     *  The destructor callback to clean up the object
     *
     *  @param  data    callback data
     */
    static void destructor(const v8::WeakCallbackData<v8::Value, External> &data)
    {
        // delete the object
        delete data.GetParameter();
    }
public:
    /**
     *  Constructor
     *
     *  @param  object  the object to handle
     */
    Handle(Php::Value object)
    {
        /**
         *  Create a copy of the object and a persistent
         *  handle for it. Even though you won't find it
         *  anywhere in the documentation: the persistent
         *  handle _must_ stay in memory, because if it
         *  gets destructed it will forget that it was
         *  made weak. It won't forget to keep a reference
         *  alive though, resulting in memory leaks and
         *  - eventually - failing assertions from within
         *  v8 itself. Yes, it realises it made a mistake
         *  and rewards us by crashing.
         */
        auto *copy= new External(std::move(object));

        // create the v8 handle around it
        _handle = v8::External::New(Isolate::get(), copy->get());

        // initialize the persistent handle
        copy->initialize(_handle);
    }

    /**
     *  Constructor
     *
     *  @param  handle  the value handle to parse
     */
    Handle(v8::Local<v8::Value> handle)
    {
        // cast and store the given handle
        _handle = v8::Local<v8::External>::Cast(handle);
    }

    /**
     *  Cast to the v8 handle that can be registered
     *  with the v8 engine and tracked there.
     *
     *  @return v8::Local<v8::Value>
     */
    operator v8::Local<v8::Value> ()
    {
        // simply return the stored handle
        return _handle;
    }

    /**
     *  Return a pointer to the managed object, so that
     *  methods and properties can be easily retrieved.
     *
     *  @return The managed value
     */
    Php::Value *operator->()
    {
        // retrieve the handled value and cast it
        return static_cast<Php::Value*>(_handle->Value());
    }

    /**
     *  Cast the object to to the original object
     *
     *  @return The original Php::Value
     */
    Php::Value &operator*()
    {
        // retrieve the handled value and cast it
        return *(operator->());
    }

    /**
     *  Cast the object to to the original object
     *
     *  @return The Php::Value we are handling
     */
    operator Php::Value &()
    {
        // retrieve the handled value and cast it
        return *(operator->());
    }
};

/**
 *  End namespace
 */
}
