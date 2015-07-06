#!/usr/bin/env node

var solo = require('../index');

function soloEval(cmd, context, filename, callback) {
  try {
    var result = eval(solo.write(solo.parse(cmd)));
    callback(null, result);
  } catch(e) {
    callback(e);
  }
}

function repl() {
  require('repl').start({
    prompt: 'solo> ',
    eval: soloEval
  });
}

function compile() {
  var source = require('fs').readFileSync(process.argv[2], 'utf8');
  var result = solo.write(solo.parse(source));

  console.log("'use strict';");
  console.log(result);
}

if (process.argv.length === 2) {
  // REPL
  repl();
} else if (process.argv[2] === '-h' || process.argv[2] === '--help') {
  console.log('Solo Usage:\n\n\tsolo => repl\n\n\tsolo file.sl => compile file and output to stdout');
} else if (process.argv.length === 3) {
  // Compile
  compile();
}
