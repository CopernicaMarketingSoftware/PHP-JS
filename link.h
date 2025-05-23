/**
 *  Link.h
 * 
 *  Class that makes sure that an associated Php::Value object is 
 *  destructed when it falls out of scope
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class Link
{
private:
    /**
     *  The object to which the deleter is linked
     *  This is a global object because it must stay in scope for as long as the object is not yet destructed,
     *  it is turned into a weak-reference in the constructor (see below) so it will not keep the object in scope
     *  @var v8::Global<v8::Object>
     */
    v8::Global<v8::Object> _object;
    
    /**
     *  The linked Php::Value (can also be a WeakReference instance)
     *  @var Php::Value
     */
    Php::Value _value;
    
    /**
     *  Do we hold a weak reference to the PHP variable?
     *  @var bool
     */
    bool _weak = false;
    
    
    
public:
    /**
     *  Constructor to create a new link
     *  @param  isolate
     *  @param  object
     *  @param  value
     *  @param  weak
     */
    Link(v8::Isolate *isolate, const v8::Local<v8::Object> &object, const Php::Value &value, bool weak) : 
        _object(isolate, object), 
        _value(weak ? Php::call("WeakReference::create", value) : value),
        _weak(weak)
    {
        // install a function that will be called when the object is garbage collected
        _object.SetWeak<Link>(this, [](const v8::WeakCallbackInfo<Link> &info) {
            
            // get the original object
            Link *self = info.GetParameter();
            
            // do the self-destruction
            delete self;

        }, v8::WeakCallbackType::kParameter);
    }
    
    /**
     *  No copying
     *  @param  that
     */
    Link(const Link &that) = delete;
    
    /**
     *  Destructor
     */
    virtual ~Link()
    {
        // remove the persistent object, this will
        _object.Reset();
    }
    
    /**
     *  Get the value
     *  @return Php::Value
     */
    Php::Value value() const
    {
        // non-weak values are stored "as is"
        if (!_weak) return _value;
        
        // the value must be an object (a WeakReference instance)
        if (!_value.isObject()) return nullptr;
        
        // get the underling object
        return _value.call("get");
    }
};
    
/**
 *  End of namespace
 */
}
