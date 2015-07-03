var solo = require('./index');

var source = require('fs').readFileSync('test.sl', 'utf8');

console.log(
  JSON.stringify(
    solo.parse(source, 'test.sl')));
