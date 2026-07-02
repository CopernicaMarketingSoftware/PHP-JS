/**
 *  FromIterator.cpp
 * 
 *  Implementation file of the FromIterator class
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include "fromiterator.h"
#include "scope.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Constructor
 *  @param  isolate     the isolate
 *  @param  object      the object that will be returned
 *  @param  value       iterable php object
 */
FromIterator::FromIterator(v8::Isolate *isolate, const Php::Value &value)
{
    // we need a scope
    Scope scope(isolate);
    
    // create a new object
    _iterator = v8::Object::New(isolate);
    
    // store pointer to state data
    _iterator->SetPrivate(scope, symbol(isolate), v8::External::New(isolate, new Data(isolate, value)));
    
    // we need to set a "next" method and a "return" method on the iterator
    auto nxtlabel = v8::String::NewFromUtf8Literal(isolate, "next");
    auto retlabel = v8::String::NewFromUtf8Literal(isolate, "return");
    
    // we need the next and return methods
    auto nxtmethod = v8::Function::New(scope, &FromIterator::nxtmethod).ToLocalChecked();
    auto retmethod = v8::Function::New(scope, &FromIterator::retmethod).ToLocalChecked();
    
    // install them on the object
    _iterator->Set(scope, nxtlabel, nxtmethod).Check();
    _iterator->Set(scope, retlabel, retmethod).Check();
    
    // we also make the iterator of itself iterable (when the iterator is explicitly iterated) 
    _iterator->Set(scope, v8::Symbol::GetIterator(isolate), _iterator).Check();    
}

/**
 *  Helper method gets the symbol to associate data with an iterator
 *  @param  isolate
 *  @return v8::Local<v8::Private>
 */
v8::Local<v8::Private> FromIterator::symbol(v8::Isolate *isolate)
{
    // lookup the symbol
    return v8::Private::ForApi(isolate, v8::String::NewFromUtf8Literal(isolate, "php-js.iterator"));
}

/**
 *  Helper method to get access to underlying data
 *  @param  isolate
 *  @param  obj
 *  @return Data*
 */
FromIterator::Data *FromIterator::restore(v8::Isolate *isolate, const v8::Local<v8::Object> &obj)
{
    // we need a local value
    v8::Local<v8::Value> val;

    // get the symbol value
    if (!obj->GetPrivate(isolate->GetCurrentContext(), symbol(isolate)).ToLocal(&val)) return nullptr;
    
    // should be external
    if (!val->IsExternal()) return nullptr;
    
    // get the value back
    return static_cast<Data *>(val.As<v8::External>()->Value());
}

/**
 *  Helper method to destruct the underlying data
 *  @param  isolate
 *  @param  obj
 *  @return Data*
 */
void FromIterator::destruct(v8::Isolate *isolate, const v8::Local<v8::Object> &obj)
{
    // get the underlying data
    auto *data = restore(isolate, obj);
    
    // do nothing if not needed
    if (data == nullptr) return;

    // destruct the object
    delete data;

    // unset the symbol value
    obj->DeletePrivate(isolate->GetCurrentContext(), symbol(isolate));
}

/**
 *  Method that is called by v8 when the next item is requested
 *  @param  args
 */
void FromIterator::nxtmethod(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    // the current isolate
    auto isolate = args.GetIsolate();
    
    // get a handle scope
    Scope scope(isolate);
    
    // the object that is being called
    auto obj = args.This();

    // pointer to data
    auto *data = restore(isolate, obj);
    
    // if iterator was already destructed
    if (data == nullptr || data->done()) return retmethod(args);
    
    // create a new return object
    auto result = v8::Object::New(isolate);
    
    // set the value
    result->Set(scope, v8::String::NewFromUtf8Literal(isolate, "value"), data->current()).Check();
    result->Set(scope, v8::String::NewFromUtf8Literal(isolate, "done"), v8::Boolean::New(isolate, !data->next())).Check();
    
    // install the return-value
    args.GetReturnValue().Set(result);
    
    // if the iterator is done by now we can forget it
    if (data->done()) destruct(isolate, obj);
}

/**
 *  Method that is called when the iterator leaps out prematurely
 *  @param  args
 */
void FromIterator::retmethod(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    // the current isolate
    auto isolate = args.GetIsolate();

    // get a handle scope
    Scope scope(isolate);
    
    // the object that is being called
    auto obj = args.This();

    // destruct internally stored data
    destruct(isolate, obj);

    // create a new return object
    auto result = v8::Object::New(isolate);
    
    // set the "done" state
    result->Set(scope, v8::String::NewFromUtf8Literal(isolate, "done"), v8::True(isolate)).Check();

    // install the return-value
    args.GetReturnValue().Set(result);
}
    
/**
 *  End of namespace
 */
}

