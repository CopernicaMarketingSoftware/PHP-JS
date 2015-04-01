/**
 *  value.h
 *
 *  Simple value casting functions for casting values
 *  between php and ecmascript runtime values.
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

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Cast a PHP runtime value to an ecmascript value
 *
 *  @param  value   the value to cast
 *  @return v8::Handle<v8::Value>
 */
v8::Handle<v8::Value> value(const Php::Value &value);

/**
 *  Cast an ecmascript value to a PHP runtime value
 *
 *  @note   The value cannot be const, as retrieving properties
 *          from arrays and objects cannot be done on const values
 *
 *  @param  value   the value to cast
 *  @return Php::Value
 */
Php::Value value(v8::Handle<v8::Value> value);

/**
 *  End namespace
 */
}
