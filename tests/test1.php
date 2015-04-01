<?php
/**
 *  Test file
 *
 */
 
/**
 *  Helper class
 */
class MyClass
{
    /**
     *  The one and only member
     *  @var string
     */
    public $member;
    
    /**
     *  Constructor
     *  @param  string
     */
    public function __construct($param)
    {
        $this->member = $param;
    }
    
    /**
     *  A regular method
     *  @return string
     */
    public function method()
    {
        return "method called";
    }
    
    /**
     *  Check if a property exists
     *  @param  name
     *  @return bool
     */
//    public function __isset($name)
//    {
//        return !method_exists($this, $name);
//    }
    
    /**
     *  Access to random properties
     *  @param  name
     *  @return string
     */
    public function __get($name)
    {
        return "property $name";
    }
    
    /**
     *  Call a random method
     *  @param  name
     *  @param  params
     *  @return mixed
     */
    public function __call($method, $params)
    {
        return "call method $method";
    }
    
    /**
     *  Use the variable as function
     *  @param  array
     *  @return mixed
     */
    public function __invoke($params)
    {
        return "invoke";
    }
}


$context = new JS\Context();
$context->assign('alert', function($output) {
    
    echo($output."\n");
    
});

$context->assign('object', new MyClass("abcd"));


$context->evaluate(file_get_contents(__DIR__.'/input1.js'));

