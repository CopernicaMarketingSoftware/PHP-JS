/**
 *  PhpArray.h
 * 
 *  Helper class to turn a v8/javascript array into a php array
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
#include "tophp.h"

/**
 *  Begin of namespace
 */
namespace JS {

/**
 *  Class definition
 */
class PhpArray : public Php::Array
{
public:
    /**
     *  Contructor
     *  @param  isolate
     *  @param  input
     */
    PhpArray(v8::Isolate *isolate, const v8::Local<v8::Array> &input)
    {
        // we need a context
        auto ctx = isolate->GetCurrentContext();

        // iterate over the input array
        for (uint32_t i = 0; i < input->Length(); ++i)
        {
            // get item from the array
            v8::MaybeLocal<v8::Value> maybe = input->Get(ctx, i);
            if (maybe.IsEmpty()) continue;
            
            // the underlying element (arrays can be sparse)
            v8::Local<v8::Value> element = maybe.ToLocalChecked();
            if (element->IsUndefined()) continue;
            
            // set this in the output array
            set(i, ToPhp(isolate, element));
        }
    }
    
    /**
     *  Destructor
     */
    virtual ~PhpArray() = default;
};
    
/**
 *  End of namespace
 */
}
