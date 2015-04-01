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
#include "isolate.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Handle class
 */
template <class T>
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
    static void destructor(const v8::WeakCallbackData<v8::Value, T>& data)
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
    Handle(const T &object)
    {
        // make a copy of the object
        auto *copy = new T(object);

        // create the v8 handle around it
        _handle = v8::External::New(isolate(), copy);

        // create a persistent handle
        v8::Persistent<v8::Value> persistent(isolate(), _handle);

        // and make it a 'weak' persistent handle, so it gets garbage collectde
        persistent.SetWeak<T>(copy, &destructor);
    }

    /**
     *  Constructor
     *
     *  @param  object  the object to handle
     */
    Handle(T &&object)
    {
        // make a copy of the object
        auto *copy = new T(std::move(object));

        // create the v8 handle around it
        _handle = v8::External::New(isolate(), copy);

        // create a persistent handle
        v8::Persistent<v8::Value> persistent(isolate(), _handle);

        // and make it a 'weak' persistent handle, so it gets garbage collectde
        persistent.SetWeak<T>(copy, &destructor);
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
     *  @return T*
     */
    T *operator->()
    {
        // retrieve the handled value and cast it
        return static_cast<T*>(_handle->Value());
    }

    /**
     *  Cast the object to to the original object
     *
     *  @return T
     */
    T &operator*()
    {
        // retrieve the handled value and cast it
        return *(operator->());
    }

    /**
     *  Cast the object to to the original object
     *
     *  @return T
     */
    operator T &()
    {
        // retrieve the handled value and cast it
        return *(operator->());
    }
};

/**
 *  End namespace
 */
}
