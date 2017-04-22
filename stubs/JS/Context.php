<?php

namespace JS;

/**
 * Stub file that allows IDE to "read" the contents of the JS\Context class.
 * This class holds no implementations for methods as it is only used to offer
 * an "interface" for the actual funtionality implemented in the library.
 *
 * @file        Context.php
 * @package     JS
 */
class Context
{
    /**
     * Used to inject data into the javascript engine.
     * Supports injecting variables and funcions.
     *
     * @param string $name  The name
     * @param mixed  $value The value
     */
    public function assign($name, $value)
    {
    }

    /**
     * Evaluates the given string and returns the result
     *
     * @param string $expression The expression that should be evaluated
     *
     * @return mixed
     */
    public function evaluate($expression)
    {
    }
}
