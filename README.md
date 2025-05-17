# PHP-JS

A library to integrate the Google V8 Javascript Engine in PHP

## ABOUT

The PHP-JS library is created and maintained by [Copernica](https://www.copernica.com). This extension
gives you the power to execute javascript right from your PHP script.
This javascript code is executed using the Google V8 engine - the same engine
that powers the Google Chrome browser and Node.js.

A typical use case is server-side execution of user-defined JavaScript, for example in
applications that allow end users to write custom scripts or automation logic.

## HOW DOES IT WORK?

PHP-JS allows you to execute JavaScript from PHP code, and expose PHP variables
and functions to a Javascript environment. 

```
// create a javascript context
$context = new JS\Context();

// assign a javascript function to show a variable
$context->assign('show', function($data) {
    var_dump($data);
});

// assign a function to summarize
$context->assign('add', function($x, $y) {
    return $x + $y;
});

// assign some additional variables
$context->assign('x', 10);
$context->assign('y', 'abc');
$context->assign('z', new DateTime());

// evaluate a javscript
try
{
    $context->evaluate("show(x); show(y); show(z);");
}
catch (Exception $exception)
{
    echo("Error: ".$exception->getMessage()."\n");
}
```

PHP-JS can map PHP variables to JS variables and vica versa. Scalars, arrays, objects,
and traversable structures (like iterators) are converted between both environment. Note,
however, that due to fundamental differences between Javascript and PHP, not all
constructs can be 1-to-1 mapped. It is recommended to stick to straightforward
data types and structures.


## DEPENDENCIES

PHP-JS relies on [PHP-CPP](http://www.php-cpp.com) and the [Google V8 engine](https://v8.dev/).
It is known to work with V8 version 13.6. We recommend compiling V8 using GCC and your system
libraries, rather than the ones bundled with V8. This avoids conflicts whtn installing PHP-JS
as a PHP extension, as PHP itself also uses system libraries.

Use the following flags when building V8:

```
is_debug=false
is_component_build=true
v8_enable_i18n_support=false
is_clang=false
use_sysroot=false
use_custom_libcxx=false
```

These flags ensure:

- GCC is used instead of Clang (`is_clang=false`)
- System libraries are used instead of bundled versions (`use_custom_libcxx=false`)
- Unnecessary features like ICU (internationalization) are disabled for a leaner build

For more details on building V8, visit [https://v8.dev/docs/build-gn](https://v8.dev/docs/build-gn).


## EXTRA INFO

If you appreciate our work and are you looking for other high quality solutions, take
a look at:

* PHP-CPP (www.php-cpp.com)
* Copernica Marketing Suite (www.copernica.com)
* MailerQ MTA (www.mailerq.com)
