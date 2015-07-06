'use strict';

var readFileSync = require('fs').readFileSync;
var solo = require('../index');

var compilePath = function(path) {
  var source = readFileSync(path, 'utf8');
  var output = solo.write(solo.parse(source));
}

// Register `.solo` file extension so that
// modules can be simply required.
require.extensions['.sl'] = function(module, filename) {
  module._compile(compilePath(filename), filename);
};
