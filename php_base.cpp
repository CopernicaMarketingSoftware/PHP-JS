/**
 *  PhpBase.cpp
 *
 *  Implementation file for the PhpBase class
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2025 Copernica B.V.
 */

/**
 *  Dependencies
 */
#include "php_base.h"
#include "names.h"
#include "core.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  isolate     The isolate
 *  @param  object      The ecmascript object
 */
PhpBase::PhpBase(v8::Isolate *isolate, const v8::Local<v8::Value> &object) :
    _core(Core::upgrade(isolate)),
    _object(isolate, object) {}

/**
 *  Destructor
 */
PhpBase::~PhpBase()
{
    // forget the associated javascript object
    _object.Reset();
}

/**
 *  Helper method to unwrap an object
 *  @param  value
 *  @return Php::Object
 */
PhpBase *PhpBase::unwrap(const Php::Value &value)
{
    // must be the right class
    if (!value.instanceOf(Names::Object) && !value.instanceOf(Names::Function)) return nullptr;

    // get self-pointer
    return (PhpBase *)value.implementation();
}

/**
 *  Get the v8 handle
 *  @return v8::Local<v8::Value>
 */
v8::Local<v8::Value> PhpBase::handle()
{
    // get the local handle back
    return _object.Get(_core->isolate());
}

/**
 *  End namespace
 */
}

