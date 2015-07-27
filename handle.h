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
     *  Nested class holding both the object
     *  and the persistent handle to it
     */
    class Object
    {
    private:
        /**
         *  The allocated object
         *  @var    T
         */
        T _object;

        /**
         *  The persistent handle
         *  @var    v8::Persistent<v8::Value>
         */
        v8::Persistent<v8::Value> _persistent;
    public:
        /**
         *  Constructor
         *
         *  @param  object  The object to keep in memory
         */
        Object(T &&object) :
            _object(std::move(object))
            // persistent will be initialized later
        {}

        /**
         *  Destructor
         */
        ~Object()
        {
            /**
             *  Reset the persistent handle
             *
             *  One would assume the fact that the persistent is destructed
             *  would be enough indication to v8 that the handle is now garbage,
             *  but alas, if we don't call this v8 will trip over and assume
             *  that the object is still alive and then complain about it.
             */
            _persistent.Reset();
        }

        /**
         *  Initialize the persistent handle
         */
        void initialize(const v8::Local<v8::External> &handle)
        {
            // create the persistent handle and make it weak
            _persistent.Reset(isolate(), handle);
            _persistent.SetWeak<Object>(this, &destructor);
        }

        /**
         *  Get pointer to the underlying object
         *
         *  @return T*
         */
        T *get()
        {
            // return the pointer
            return &_object;
        }
    };

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
    static void destructor(const v8::WeakCallbackData<v8::Value, Object> &data)
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
    Handle(T object)
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
        auto *copy= new Object(std::move(object));

        // create the v8 handle around it
        _handle = v8::External::New(isolate(), copy->get());

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
