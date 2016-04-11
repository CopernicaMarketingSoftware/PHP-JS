<?php
/**
 *  Test file
 *
 */
require_once('PxCollections/autoload.php');

/**
 *  Helper class
 */
class MyClass extends PxWrappedCollection
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
        
        parent::__construct(array("a","b","c","d"));
    }
    
    /**
     *  Clone constructor
     */
    public function __clone()
    {
        $this->member = "clone from ".$this->member;
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
     *  Get self-reference
     *  @return object
     */
    public function self()
    {
        return clone $this;
    }
    
    /**
     *  Check if a property exists
     *  @param  name
     *  @return bool
     */
    public function __isset($name)
    {
//        return true;
        return !method_exists($this, $name);
    }
    
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
    public function __callx($method, $params)
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

$context->assign('create', function($x) {
    
    
    //throw new Exception("exception");
    
    return new MyClass($x);
});


try
{
    $context->evaluate(file_get_contents(__DIR__.'/input1.js'));
}
catch (Exception $exception)
{
    // this was the expected place to catch the "123" exception
}

// @todo
//      call __invoke()
//      iterate over object
//      indien method bestaat, dan moet object->method niet een verwijzing naar een property zijn (zelfs als dat property ook bestaat)
//      exceptions in twee richtingen
//      solution for library dependencies

