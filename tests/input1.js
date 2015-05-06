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

// iterate over the properties
//for (var x in object)
//{
//    alert('found property '+x);
//}
