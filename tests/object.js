/**
 *  Object.js
 * 
 *  Javascript that checks if we can reuse a javascript-to-php object
 *  conversion.
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2025 Copernica BV
 */

function() { hallo(); hallo(); }

x;

// @todo this was causing a crash
//const i = y[Symbol.iterator];
//
//i.next();


//hallo();


//for (let v of y) 
//{
//
//}


/**
 *  We need an object
 *  @var Object
 */
//const obj = new Object();

/**
 *  Last expression is the return-value
 */
//[obj, obj];


// @todo when there is a syntax error it sigsegv
//x.hello();

/*

const p = new Promise(function(resolve, reject) {

    hallo();
    resolve("yes!");
});

p.then(function() {
   
    hallo(); 
    
});
*/

/*
const f = function() {
    hallo();
    hallo();
}


*/
