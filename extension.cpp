/**
 *  extension.cpp
 *
 *  Startup file for the PHP extension
 *
 *  @copyright 2015 - 2025 Copernica BV
 */

/**
 *  Dependencies
 */
#include <phpcpp.h>
#include "php_context.h"
#include "php_object.h"
#include "php_function.h"
#include "php_script.h"
#include "platform.h"
#include "names.h"

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
        static Php::Extension extension("PHP-JS2", THE_VERSION);

        // declare the accessor attributes
        // @todo use constants
        extension.add(Php::Constant(JS::Names::None,          v8::None));
        extension.add(Php::Constant(JS::Names::ReadOnly,      v8::ReadOnly));
        extension.add(Php::Constant(JS::Names::DontDelete,    v8::DontDelete));
        extension.add(Php::Constant(JS::Names::DontEnumerate, v8::DontEnum));

        // create the classes
        Php::Class<JS::PhpContext> context(JS::Names::Context);
        Php::Class<JS::PhpScript> script(JS::Names::Script);
        Php::Class<JS::PhpObject> object(JS::Names::Object);
        Php::Class<JS::PhpFunction> function(JS::Names::Function);

        // properties can be assigned to the context
        context.method<&JS::PhpContext::assign>("assign", {
            Php::ByVal("name", Php::Type::String, true),
            Php::ByVal("value", Php::Type::Null, true),
            Php::ByVal("attribute", Php::Type::Numeric, false)
        });

        // add a method to parse + execute some script
        context.method<&JS::PhpContext::evaluate>("evaluate", {
            Php::ByVal("script", Php::Type::String, true),
            Php::ByVal("timeout", Php::Type::Numeric, false)
        });

        // add a script-method to construct the script
        script.method<&JS::PhpScript::__construct>("__construct", {
            Php::ByVal("script", Php::Type::String, true),
        });

        // add a script-method to execute
        script.method<&JS::PhpScript::execute>("evaluate", {
            Php::ByVal("timeout", Php::Type::Numeric, false)
        });

        // add the classes to the extension
        extension.add(std::move(context));
        extension.add(std::move(object));
        extension.add(std::move(function));

        // the platform needs to be cleaned up on engine shutdown
        extension.onShutdown([]{

            // clean up the platform
            JS::Platform::shutdown();
        });

        // return the extension
        return extension;
    }
}
