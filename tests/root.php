<?php
/**
 *  root.php
 * 
 *  Script to test contexts with a custom root
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2026 Copernica BV
 */

/**
 *  Our root object
 */
class MyRoot
{
    public $prop1 = "abc";
    public $prop2 = "123";
}

$root = new MyRoot();

/**
 *  Create a context with a custom root
 */
$context = new JS\Context($root);
$context->assign('assigned', 'check!');

print_r($root);

/**
 *  Check if properties exists as expected
 */
echo($context->evaluate("parseInt('1234');")."\n");
echo($context->evaluate("assigned")."\n");
echo($context->evaluate("prop1")."\n");
echo($context->evaluate("prop2")."\n");
