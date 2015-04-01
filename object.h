/**
 *  object.h
 *
 *  Class that wraps around a PHP object to make it available in
 *  ecmascript. As in PHP, properties added to the object, both
 *  from ecmascript and PHP, will become visible on the other
 *  side.
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
class Object
{
private:
    /**
     *  The object template
     *  @var    Stack<v8::ObjectTemplate>
     */
    Stack<v8::ObjectTemplate> _template;
public:
    /**
     *  Constructor
     *
     *  @param  object  The object to wrap
     */
    Object(Php::Object object);

    /**
     *  Retrieve the ecmascript object handle
     *  that can be assigned directly to v8
     *
     *  @return v8::Local<v8::Value>
     */
    operator v8::Local<v8::Value> ();
};

/**
 *  End namespace
 */
}
