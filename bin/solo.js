#!/usr/bin/env node

var solo = require('../index');

var source = require('fs').readFileSync(process.argv[2], 'utf8');
var result = solo.write(solo.parse(source));

console.log("'use strict';");
console.log(result);

