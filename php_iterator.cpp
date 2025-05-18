/**
 *  PhpIterator.cpp
 * 
 *  Implementation file for the PhpIterator class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "php_iterator.h"
#include "scope.h"
#include "php_variable.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  base        The base that PHP-CPP insists on
 *  @param  core        The javascript core
 *  @param  object      The object to iterate
 */
PhpIterator::PhpIterator(Php::Base *base, const std::shared_ptr<Core> &core, const v8::Local<v8::Object> &object) : Php::Iterator(base),
    _core(core),
    _object(core->isolate(), object)
{
    // get a scope (we already have one when we are called, but ok)
    Scope scope(core);
    
    // get the key (this is a maybe)
    auto maybe = object->GetPropertyNames(scope);
    if (maybe.IsEmpty()) return;
    
    // convert to a v8::Local
    auto keys = maybe.ToLocalChecked();
    
    // store in _keys, which is a v8::Global
    _keys.Reset(core->isolate(), keys);
    
    // size can be helpful to have in a cached form
    _size = keys->Length();
}

/**
 *  Destructor
 */
PhpIterator::~PhpIterator()
{
    // destruct handles
    _object.Reset();
    _keys.Reset();
}

/**
 *  Is the iterator still valid?
 *  @return is an element present at the current offset
 */
bool PhpIterator::valid()
{
    // we should not be out of bounds
    return _position < _size;
}

/**
 *  Retrieve the current value
 *  @return value at current offset
 */
Php::Value PhpIterator::current()
{
    // make sure there is a scope
    Scope scope(_core);
    
    // get the object and keys in a local variables
    v8::Local<v8::Object> object(_object.Get(_core->isolate()));
    v8::Local<v8::Array> keys(_keys.Get(_core->isolate()));
    
    // retrieve the current key
    auto key = keys->Get(scope, _position);
    if (key.IsEmpty()) return nullptr;
    
    // retrieve the current key, the value and convert it
    auto value = object->Get(scope, key.ToLocalChecked());
    if (key.IsEmpty()) return nullptr;
    
    // expose to php space
    return PhpVariable(_core->isolate(), value.ToLocalChecked());
}

/**
 *  Retrieve the current key
 *  @return the current key
 */
Php::Value PhpIterator::key()
{
    // make sure there is a scope
    Scope scope(_core);
    
    // get the keys in a local variable
    v8::Local<v8::Array> keys(_keys.Get(_core->isolate()));

    // retrieve the current key
    auto key = keys->Get(scope, _position);
    if (key.IsEmpty()) return nullptr;
    
    // retrieve the current key, the value and convert it
    return PhpVariable(_core->isolate(), key.ToLocalChecked());
}

/**
 *  Move ahead to the next item
 */
void PhpIterator::next()
{
    // move to the next position
    ++_position;
}

/**
 *  Start over at the beginning
 */
void PhpIterator::rewind()
{
    // move back to the beginning
    _position = 0;
}

/**
 *  End of namespace
 */
}

