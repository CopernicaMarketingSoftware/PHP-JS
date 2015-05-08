/**
 *  extension.cpp
 *
 *  Startup file for the PHP extension
 *
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include <phpcpp.h>
#include "context.h"
#include "jsobject.h"

/**
 *  The VERSION macro is going to be used as string with surrounded quotes
 */
#define STR_VALUE(arg)      #arg
#define VERSION_NAME(name)  STR_VALUE(name)
#define THE_VERSION         VERSION_NAME(VERSION)

/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {

    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module()
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("PxJavascript", THE_VERSION);

        // declare the accessor attributes
        extension.add(Php::Constant("JS\\None",         v8::None));
        extension.add(Php::Constant("JS\\ReadOnly",     v8::ReadOnly));
        extension.add(Php::Constant("JS\\DontDelete",   v8::DontDelete));
        extension.add(Php::Constant("JS\\DontEnumerate",v8::DontEnum));

        // create our context class
        Php::Class<JS::Context> context("JS\\Context");

        // properties can be assigned
        context.method("assign", &JS::Context::assign, {
            Php::ByVal("name", Php::Type::String, true),
            Php::ByVal("value", Php::Type::Null, true),
            Php::ByVal("attribute", Php::Type::Numeric, false)
        });

        // add a method to execute some script
        context.method("evaluate", &JS::Context::evaluate, {
            Php::ByVal("script", Php::Type::String, true)
        });

        // an empty class for exporting object from ecmascript
        Php::Class<JS::JSObject> object("JS\\Object");

        // add the classes to the extension
        extension.add(std::move(context));
        extension.add(std::move(object));

        // return the extension
        return extension;
    }
}
