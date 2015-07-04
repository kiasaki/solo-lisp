#!/usr/bin/env node

var solo = require('../index');

var source = require('fs').readFileSync(process.argv[2], 'utf8');
console.log("'use strict';");
console.log(solo.write(solo.parse(source)));

