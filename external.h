/**
 *  external.h
 *
 *  Class that tracks a pointer external to the V8
 *  engine that we should clean up if V8 somehow
 *  forgets (which happens a lot).
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
#include "context.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Object holding a reference external to V8
 */
class External
{
private:
    /**
     *  The allocated object
     *  @var    Php::Value
     */
    Php::Value _object;

    /**
     *  The persistent handle
     *  @var    v8::Persistent<v8::Value>
     */
    v8::Persistent<v8::Value> _persistent;

    /**
     *  The destructor callback to clean up the object
     *
     *  @param  data    callback data
     */
    static void destructor(const v8::WeakCallbackInfo<External> &data)
    {
        // stop tracking the external reference
        Context::current()->untrack(data.GetParameter());

        // delete the object
        delete data.GetParameter();
    }
public:
    /**
     *  Constructor
     *
     *  @param  object  The object to keep in memory
     */
    External(Php::Value &&object) :
        _object(std::move(object))
        // persistent will be initialized later
    {
        // start tracking the reference
        Context::current()->track(this);
    }

    /**
     *  Destructor
     */
    ~External()
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
        _persistent.Reset(Isolate::get(), handle);
        _persistent.SetWeak<External>(this, &destructor, v8::WeakCallbackType::kParameter);
    }

    /**
     *  Get pointer to the underlying object
     *
     *  @return The underlying value
     */
    Php::Value *get()
    {
        // return the pointer
        return &_object;
    }
};

/**
 *  End namespace
 */
}
