<?php
/**
 *  Object.php
 * 
 *  Check if we can return objects from javascript
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copenica BV
 */

$context = new JS\Context();

class MyClass1
{
    public $x = 10;
    public $y = 12;
    
}

class MyClass2 implements ArrayAccess
{
    public $x = 10;
    public $y = 12;
    
    public function offsetGet($x)
    {
        return 'abc';
    }
    public function offsetSet($x, $y) {}
    public function offsetExists($x) { return true; }
    public function offsetUnset($x) {}
}

$context->assign('x', new MyClass1());
$context->assign('y', new MyClass2);
$context->assign('z', array(3,4,5));

$result = $context->evaluate(file_get_contents(__DIR__.'/object.js'));
var_dump($result);
