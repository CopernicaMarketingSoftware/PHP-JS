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
$result = $context->evaluate(file_get_contents(__DIR__.'/object.js'));
var_dump($result);
