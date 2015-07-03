var solo = require('./index');

var source = require('fs').readFileSync('test.sl', 'utf8');

function clean(ast) {
  for (var x in ast) {
    var node = ast[x];
    delete node.location;
    if ('items' in node) {
      node.items = clean(node.items);
    }
  }
  return ast;
}

console.log(
  JSON.stringify(
    clean(
      solo.parse(source, 'test.sl')
    )
  , null, 2)
);
