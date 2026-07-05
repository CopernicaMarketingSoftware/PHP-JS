<?php

$script = new JS\Script("globalThis.a = a + parseInt(globalThis.a, 10); 'result:'+a");

$script->assign("a", 12); 

echo($script->execute()."\n");
$script->reset();
$script->assign("a", 77); 
echo($script()."\n");
echo($script()."\n");
echo($script()."\n");
echo($script()."\n");


