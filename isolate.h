/**
 *  isolate.h
 *
 *  Simple getter for the (unique) v8 isolate. This
 *  method makes sure that a single isolate gets
 *  constructed and that the v8 engine is properly
 *  initialized once.
 *
 *  Note that this is explicitly not thread-safe,
 *  but it is fast. Since none of our other extensions
 *  are properly thread-safe, this is an acceptable
 *  limitation
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

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Retrieve the isolate
 *
 *  @return v8::Isolate*
 */
v8::Isolate* isolate();

/**
 *  End namespace
 */
}
