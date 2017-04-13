<?php

$context = new JS\Context();

for ($i = 1; $i <= 1000000; $i++)
{
	if ($i % 1000 == 0) echo($i . PHP_EOL);
    $context->evaluate("$i;", 1);
}
