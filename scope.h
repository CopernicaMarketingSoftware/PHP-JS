/**
 *  Scope.h
 * 
 *  Utility class to set up the handle scope based on a Context. This
 *  class make sure we do not have to repeat the same code over and over
 *  again.
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "context.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Scope
{
private:
    /**
     *  Scope for the isolate
     *  @var v8::Isolate::Scope
     */
    v8::Isolate::Scope _iscope;

    /**
     *  Stack-allocated handle scope
     *  @var v8::HandleScope
     */
    v8::HandleScope _hscope;

    /**
     *  We need the context in a local handle
     *  @var v8::Local<v8::Context>
     */
    v8::Local<v8::Context> _context;

    /**
     *  Enter the context for this call
     *  @var v8::Context::Scope
     */
    v8::Context::Scope _cscope;
    
public:
    /**
     *  Constructor
     *  @param  context
     */
    Scope(const std::shared_ptr<Context> &context) :
        _iscope(context->isolate()),
        _hscope(context->isolate()),
        _context(context->context(_hscope)),
        _cscope(_context) {}

    /**
     *  Constructor
     *  @param  isolate
     */
    Scope(v8::Isolate *isolate) :
        _iscope(isolate),
        _hscope(isolate),
        _context(isolate->GetCurrentContext()),
        _cscope(_context) {}
    
    /**
     *  Destructor
     */
    virtual ~Scope() = default;
    
    /**
     *  Cast to the context
     *  @return v8::Local<v8::Context>
     */
    operator v8::Local<v8::Context>& () { return _context; }
    
    /**
     *  Get the global object
     *  @return v8::Local<v8::Object>
     */
    v8::Local<v8::Object> global() const { return _context->Global(); }

};
    
/**
 *  End of namespace
 */
}

