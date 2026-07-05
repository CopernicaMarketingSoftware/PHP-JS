<?php
/**
 *  Share.php
 *
 *  Test to see if we can share variables between contexts
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2026 Copernica BV
 */

/**
 *  Create two contexts
 *  @var JS\Context
 */
$c1 = new JS\Context();
$c2 = new JS\Context();

/**
 *  Assign variables to identify them
 */
$c1->assign('name', 'Context 1');
$c2->assign('name', 'Context 2');

/**
 *  See if the both have a different name
 */
echo($c1->evaluate("name;")."\n");
echo($c2->evaluate("name;")."\n");

/**
 *  Let each context create an object
 */
$o1 = $c1->evaluate("(function() { return { member: name, method: () => 'name: '+name } })();");
$o2 = $c2->evaluate("(function() { return { member: name, method: () => 'name: '+name } })();");

/**
 *  Check the objects
 */
echo($o1->method()."\n");
echo($o2->method()."\n");

/**
 *  Now cross-assign the objects
 */
$c1->assign('other', $o2);
$c2->assign('other', $o1);

/**
 *  Can we call it?
 */
echo("swapped: ".$c1->evaluate("other.method();")."\n");
echo("swapped: ".$c2->evaluate("other.method();")."\n");

