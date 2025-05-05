/**
 *  Linker.cpp
 * 
 *  Implementation file for the Linker class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "linker.h"
#include "link.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  isolate     the active isolate
 *  @param  key         the private symbol that is associated storing external pointers
 *  @param  object      the javascript object to be linked
 */
Linker::Linker(v8::Isolate *isolate, const v8::Global<v8::Private> &key, const v8::Local<v8::Object> &object) :
    _isolate(isolate), _key(key.Get(isolate)), _object(object) {}

/**
 *  Get the internal pointer
 *  @return Link
 */
Link *Linker::pointer() const
{
    // check if the private propery exists
    v8::MaybeLocal<v8::Value> property = _object->GetPrivate(_isolate->GetCurrentContext(), _key);
    
    // if not set, or if not a pointer to an external thing
    if (property.IsEmpty()) return nullptr;
    
    // turn into a local
    v8::Local<v8::Value> value = property.ToLocalChecked();
    
    // should be external
    if (!value->IsExternal()) return nullptr;
    
    // get the external property, and cast to a Php::Value
    return static_cast<Link*>(value.As<v8::External>()->Value());
}

/**
 *  Is the linker associated with a PHP object?
 *  @return bool
 */
bool Linker::valid() const
{
    // check the pointer
    return pointer() != nullptr;
}

/**
 *  Associate the object with a PHP variable
 *  @param  value
 *  @return Php::Value
 */
const Php::Value &Linker::attach(const Php::Value &value)
{
    // remove the old pointer
    detach();
    
    // make a brand new link
    auto *link = new Link(_isolate, _object, value);
    
    // associate property
    _object->SetPrivate(_isolate->GetCurrentContext(), _key, v8::External::New(_isolate, link));
    
    // done, expose the same value
    return value;
}

/**
 *  Detach the PHP object from the javascript object
 */
void Linker::detach()
{
    // get the link pointer
    Link *link = pointer();
    
    // if not even set
    if (link == nullptr) return;
    
    // remove the link
    delete link;
    
    // remove private property
    _object->DeletePrivate(_isolate->GetCurrentContext(), _key);
}

/**
 *  Expose the object in PHP space
 *  @return Php::Value
 */
Php::Value Linker::value() const
{
    // get the link pointer
    Link *link = pointer();
    
    // if not set
    if (link == nullptr) return nullptr;
    
    // get the php value
    return link->value();
}

/**
 *  End of namespace
 */
}

