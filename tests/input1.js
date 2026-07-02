/**
 *  Input1.js
 */
 
try
{
    var object = create("xyz");
}
catch (exception)
{
    throw 123;
}
 
alert('object.member: '+object.member);
alert('object.method: '+object.method());
alert('object.randomproperty: ' +object.randomproperty);
//alert('object.randommethod: ' +object.randommethod());
//alert('object(): '+object());

var other = object.self();

alert('other.member: '+other.member);
alert('traverse 1');

// iterate over the properties
for (var x in object)
{
    alert('found property '+x);
}

// iterate over the properties
alert('traverse 2');
for (var x of object)
{
    alert('found property '+x);
}

alert('traverse 3');
for (var i = 0; i < object.length; ++i)
{
    alert('found index '+i+' '+object[i]);
}

alert('traverse 4');
for (var x in array)
{
    alert('found index '+i+' '+array[i]);
}
