/**
 *  PhpIterator.h
 *  
 *  Class to iterate over a JS\Object. This instance is constructed
 *  by the JSObject::getIterator() method.
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
#include <phpcpp.h>
#include <v8.h>
#include <core.h>

/**
 *  Begin of namespace
 */
namespace JS {
    
/**
 *  Class definition
 */
class PhpIterator : public Php::Iterator
{
private:
    /**
     *  The javascript core
     *  @var std::shared_ptr<Core>
     */
    std::shared_ptr<Core> _core;

    /**
     *  The underlying ecmascript object
     *  @var    Stack<v8::Object>
     */
    v8::Global<v8::Object> _object;

    /**
     *  All properties in the object
     *  @var    Stack<v8::Array>
     */
    v8::Global<v8::Array> _keys;

    /**
     *  Current position in the object
     *  @var    uint32_t
     */
    uint32_t _position = 0;

    /**
     *  Number of valid keys
     *  @var    uint32_t
     */
    uint32_t _size = 0;

public:
    /**
     *  Constructor
     *  @param  base        The base that PHP-CPP insists on
     *  @param  core        The javascript core
     *  @param  object      The object to iterate
     */
    PhpIterator(Php::Base *base, const std::shared_ptr<Core> &core, const v8::Local<v8::Object> &object);

    /**
     *  Destructor
     */
    virtual ~PhpIterator();

    /**
     *  Is the iterator still valid?
     *  @return is an element present at the current offset
     */
    virtual bool valid() override;

    /**
     *  Retrieve the current value
     *  @return value at current offset
     */
    virtual Php::Value current() override;

    /**
     *  Retrieve the current key
     *  @return the current key
     */
    virtual Php::Value key() override;

    /**
     *  Move ahead to the next item
     */
    virtual void next() override;

    /**
     *  Start over at the beginning
     */
    virtual void rewind() override;
};

/**
 *  End of namespace
 */
}

