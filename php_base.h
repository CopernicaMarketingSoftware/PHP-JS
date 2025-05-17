/**
 *  PhpBase.h
 *
 *  Base class for objects inside v8 that are exposed to PHP space.
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2025 Copernica B.V.
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
 *  Forward declarations
 */
class Core;

/**
 *  Class definition
 */
class PhpBase : public Php::Base
{
protected:
    /**
     *  The core in which we operate (this is a shared pointer because for as long as the
     *  object lives in PHP space, we want to keep the core around (even when the PHP
     *  space JS\Context already fell out of scope)
     *  @var std::shared_ptr<Context>
     */
    std::shared_ptr<Core> _core;

    /**
     *  The underlying ecmascript object
     *  @var    v8::Global<v8::Value>
     */
    v8::Global<v8::Value> _object;


    /**
     *  Constructor
     *  @param  isolate     The isolate
     *  @param  object      The ecmascript object
     */
    PhpBase(v8::Isolate *isolate, const v8::Local<v8::Value> &object);

public:
    /**
     *  No copying
     *  @param  that
     */
    PhpBase(const PhpBase &that) = delete;

    /**
     *  Destructor
     */
    virtual ~PhpBase();
    
    /**
     *  Helper method to unwrap an object
     *  @param  core
     *  @param  value
     *  @return Php::Base
     */
    static PhpBase *unwrap(const Core *core, const Php::Value &value);

    /**
     *  Get the v8 handle
     *  @return v8::Local<v8::Value>
     */
    v8::Local<v8::Value> handle();
};

/**
 *  End namespace
 */
}

