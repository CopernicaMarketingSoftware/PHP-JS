/**
 *  FromIterator.h
 * 
 *  Class that turns a PHP traversable object into something that
 *  is iterable in a javascript environment too.
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
#include "fromphp.h"

/**
 *  Begin of namespace
 */
namespace JS {
    
/**
 *  Class definition
 */
class FromIterator
{
private:
    /**
     *  The variable that represents the iterator
     *  @var v8::Local<v8::Object>
     */
    v8::Local<v8::Object> _iterator;
    
    /**
     *  Structure that holds the data associated with the iterator,
     *  and that is added via a symbol in the javascript-object
     */
    class Data
    {
    private:
        /**
         *  Reference to the isolate
         *  @var v8::Isolate
         */
        v8::Isolate *_isolate;

        /**
         *  The PHP space Iterator object
         *  @var Php::Value
         */
        Php::Value _value;
    
    public:
        /**
         *  Constructor
         *  @param  isolate
         *  @param  iterator
         */
        Data(v8::Isolate *isolate, const Php::Value &value) :
            _isolate(isolate), _value(value) {}
            
        /**
         *  Destructor
         */
        virtual ~Data() = default;
        
        /**
         *  Is the iterator still valid?
         *  @return bool
         */
        bool valid() { return _value.call("valid"); }
        
        /**
         *  Is the iterator done?
         *  @return bool
         */
        bool done() { return !valid(); }
        
        /**
         *  Get the current value
         *  @return Php::Value
         */
        Php::Value value() { return _value.call("current"); }
        
        /**
         *  Get the current value
         *  @return v8::Local
         */
        v8::Local<v8::Value> current() { return FromPhp(_isolate, _value.call("current")); }
        
        /**
         *  Proceed to the next record
         *  @return bool
         */
        bool next() { _value.call("next"); return valid(); }
    };


    /**
     *  Helper method to get access to the underlying data
     *  @param  isolate
     *  @param  obj
     *  @return Data*
     */
    static Data *restore(v8::Isolate *isolate, const v8::Local<v8::Object> &obj);
    
    /**
     *  Destruct internally stored data
     *  @param  isolate
     *  @param  obj
     *  @return Data*
     */
    static void destruct(v8::Isolate *isolate, const v8::Local<v8::Object> &obj);

    /**
     *  Method that is called by v8 when the next item is requested
     *  @param  args
     */
    static void nxtmethod(const v8::FunctionCallbackInfo<v8::Value> &args);
    
    /**
     *  Method that is called when the iterator leaps out prematurely
     *  @param  args
     */
    static void retmethod(const v8::FunctionCallbackInfo<v8::Value> &args);


public:
    /**
     *  Constructor
     *  @param  isolate     the isolate
     *  @param  value       iterable php object
     */
    FromIterator(v8::Isolate *isolate, const Php::Value &value);

    /**
     *  Private destructor (object is self-destructing)
     */
    virtual ~FromIterator() = default;

    /**
     *  Cast to the local object
     *  @return v8::Local<v8::Object>
     */
    v8::Local<v8::Object> &value() { return _iterator; }
};
    
/**
 *  End of namespace
 */
}

