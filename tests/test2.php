<?php

$context = new JS\Context();

$context->assign("alert", function($output) {

    echo ($output.PHP_EOL);

});

var_dump($context->evaluate("3 + 2;"));

$context->evaluate('alert("TEST");');