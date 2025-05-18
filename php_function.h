/**
 *  PhpFunction.h
 *
 *  Class that wraps around an ecmascript function and makes it available to PHP
 *  userspace.
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica B.V.
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "php_base.h"

/**
 *  Start namespace
 */
namespace JS {

/**
 *  Class definition
 */
class PhpFunction : public PhpBase
{
public:
    /**
     *  Constructor
     *  @param  isolate     The isolate
     *  @param  object      The ecmascript object
     */
    PhpFunction(v8::Isolate *isolate, const v8::Local<v8::Function> &object) :
        PhpBase(isolate, object) {}

    /**
     *  No copying
     *  @param  that
     */
    PhpFunction(const PhpFunction &that) = delete;

    /**
     *  Destructor
     */
    virtual ~PhpFunction() = default;

    /**
     *  Method that is called when the function is invoked
     *  @param  params
     *  @return Php::Value
     */
    Php::Value __invoke(Php::Parameters &params);
};

/**
 *  End namespace
 */
}

