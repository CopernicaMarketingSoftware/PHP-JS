<?php


class MyClass2
{
    public $x = 1;
    
    public function __isset($name)
    {
        return $name == 'y';
    }
}

$object = new MyClass2();

if (property_exists($object, 'x')) echo("x exists\n");
if (property_exists($object, 'y')) echo("y exists\n");
if (property_exists($object, 'z')) echo("z exists\n");

if (isset($object->x)) echo("x isset\n");
if (isset($object->y)) echo("y isset\n");
if (isset($object->z)) echo("z isset\n");

if (has_property($object, 'x')) echo("has x\n");
if (has_property($object, 'y')) echo("has y\n");
if (has_property($object, 'z')) echo("has z\n");

