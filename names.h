/**
 *  Names.h
 * 
 *  One central place in which we define the classnames
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
 *  All classnames
 */
struct Names {
    // class names
    inline static const char *Script = "JS\\Script";
    inline static const char *Object = "JS\\Object";
    inline static const char *Context = "JS\\Context";
    inline static const char *Function = "JS\\Function";
    
    // constants
    inline static const char *None = "JS\\None";
    inline static const char *ReadOnly = "JS\\ReadOnly";
    inline static const char *DontDelete = "JS\\DontDelete";
    inline static const char *DontEnumerate = "JS\\DontEnumerate";
    
    
};

/**
 *  End of namespace
 */
}

