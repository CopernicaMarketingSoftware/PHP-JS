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


#include <iostream>

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
    // @todo should we be calling rewind() right away?
    
    std::cout << "Iterator::Iterator" << std::endl;
    
    Php::call("var_dump", value);
    
    // we need a scope
    Scope scope(isolate);
    
    // we need the context for looking up the symbol
    auto core = Core::upgrade(isolate);
    
    // create a new object
    _iterator = v8::Object::New(isolate);
    
    // store pointer to state data
    _iterator->SetPrivate(scope, core->symbol().Get(isolate), v8::External::New(isolate, new Data(isolate, value)));
    
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
 *  Helper method to get access to underlying data
 *  @param  isolate
 *  @param  obj
 *  @return Data*
 */
FromIterator::Data *FromIterator::restore(v8::Isolate *isolate, const v8::Local<v8::Object> &obj)
{
    // we need the symbol for linking pointers to objects
    auto symbol = Core::upgrade(isolate)->symbol().Get(isolate);
    
    // we need a local value
    v8::Local<v8::Value> val;

    // get the symbol value
    if (!obj->GetPrivate(isolate->GetCurrentContext(), symbol).ToLocal(&val)) return nullptr;
    
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

    // we need the symbol for linking pointers to objects
    auto symbol = Core::upgrade(isolate)->symbol().Get(isolate);

    // unset the symbol value
    obj->DeletePrivate(isolate->GetCurrentContext(), symbol);
}

/**
 *  Method that is called by v8 when the next item is requested
 *  @param  args
 */
void FromIterator::nxtmethod(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    std::cout << "nxtmethod" << std::endl;
    
    // the current isolate
    auto isolate = args.GetIsolate();
    
    // get a handle scope
    Scope scope(isolate);
    
    // the object that is being called
    auto obj = args.This();

    // pointer to data
    // @todo FunctionCallbackInfo also has a "Data" property
    auto *data = restore(isolate, obj);
    
    // if iterator was already destructed
    if (data == nullptr || data->done()) return retmethod(args);
    
    // create a new return object
    auto result = v8::Object::New(isolate);
    
    std::cout << "value = " << data->value() << std::endl;
    
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
    std::cout << "retmethod" << std::endl;
    
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

