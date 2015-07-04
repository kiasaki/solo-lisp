#!/usr/bin/env node

var solo = require('../index');

var source = require('fs').readFileSync(__dirname + '/../test.sl', 'utf8');
console.log(solo.write(solo.parse(source)));

