/**
 *  stack.h
 *
 *  This class is used for subverting the default v8 memory
 *  management. Normally, when you create a variable, it is
 *  created in the last "scoped handle" that was constructed.
 *
 *  Since this makes it tricky to know in which "handle" a
 *  variable is defined (and there is no way to force a
 *  variable to belong to a certain "handle") we do not
 *  want to use this when declaring long-lived member
 *  variables.
 *
 *  This class will make a v8 variable behave like a proper
 *  C++ variable. It will exist until it is destructed,
 *  regardless of any handle that may or may not have been
 *  defined elsewhere.
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
 *  Stack variable class
 */
template <typename T>
class Stack
{
private:
    /**
     *  The persistent handle
     *  @var    v8::UniquePersistent<T>
     */
    v8::UniquePersistent<T> _handle;
public:
    /**
     *  Empty constructor
     */
    Stack() {}

    /**
     *  Constructor
     *
     *  @param  value   the value to hold on to
     */
    Stack(v8::Handle<T> value) :
        _handle(Isolate::get(), value)
    {}

    /**
     *  Copy constructor
     *
     *  @param  that    stack instance to copy
     */
    Stack(const Stack &that) :
        _handle(Isolate::get(), (v8::Local<T>)that)
    {}

    /**
     *  Assign a different handle
     *
     *  @param  value   the value to hold on to
     */
    Stack &operator=(v8::Handle<T> value)
    {
        // store the new handle and value
        _handle.Reset(Isolate::get(), value);

        // allow chaining
        return *this;
    }

    /**
     *  Cast to the underlying handle that is accepted
     *  by most underlying v8 methods
     *
     *  @return v8::Local<T>
     */
    operator const v8::Local<T> () const
    {
        // create the value
        return v8::Local<T>::New(Isolate::get(), _handle);
    }

    /**
     *  For direct access, this class can be used as a pointer
     *  so that all methods on the underlying object can be
     *  called directly
     */
    const v8::Local<T> operator->() const { return *this; }
};

/**
 *  End namespace
 */
}
