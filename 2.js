var Parsimmon = require('parsimmon');

var read = (function() {
  function interpretEscapes(str) {
    var escapes = {
      b: '\b',
      f: '\f',
      n: '\n',
      r: '\r',
      t: '\t'
    };

    return str.replace(/\\(u[0-9a-fA-F]{4}|[^u])/, function(_, escape) {
      var type = escape.charAt(0);
      var hex = escape.slice(1);
      if (type === 'u') return String.fromCharCode(parseInt(hex, 16));
      if (escapes.hasOwnProperty(type)) return escapes[type];
      return type;
    });
  }

  var regex = Parsimmon.regex;
  var string = Parsimmon.string;
  var optWhitespace = Parsimmon.optWhitespace;
  var lazy = Parsimmon.lazy;
  var alt = Parsimmon.alt;

  var comment = regex(/;[^\n]*/).skip(string('\n')).desc('comment');
  var ignore = comment.or(regex(/\s*/));
  function lexeme(p) { return p.skip(ignore); }

  var expr = lazy('s-expression', function() { return form.or(atom); });

  var str = lexeme(regex(/"((?:\\.|.)*?)"/, 1))
    .map(interpretEscapes)
    .desc('string');
  var number = lexeme(regex(/-?(0|[1-9]\d*)([.]\d+)?(e[+-]?\d+)?/i))
    .map(parseFloat)
    .desc('number');
  var id = lexeme(regex(/[a-z!$%&*+-.\/:<=>?@^_~][a-z0-9!$%&*+-.\/:<=>?@^_~]*/i))
    .desc('identifier');

  var nullLiteral = lexeme(string('null')).result(null).desc('string');
  var trueLiteral = lexeme(string('true')).result(true).desc('boolean');
  var falseLiteral = lexeme(string('false')).result(false).desc('boolean');

  var atom = alt(
    nullLiteral,
    trueLiteral,
    falseLiteral,
    number,
    str,
    id
  );

  function delimited(left, right, name) {
    return lexeme(string(left))
      .then(expr.many())
      .skip(lexeme(string(right)))
      .map(function(l) { return {type: name, items: l}; })
      .desc(name);
  }
  var list = delimited('(', ')', 'list');
  var array = delimited('[', ']', 'array');
  var object = delimited('{', '}', 'object');
  var form = alt(list, array, object);

  return ignore.then(expr.many());
})();

var parse = function(source) {
  var result = parse.parse(source);
  if (!result.status) {
    return Parsimmon.formatError(source, result);
  } else {
    return result.value;
  }
}

var write = function(ast) {
  var cgReadyAst = {};

  return escodegen.generate(cgReadyAst);
};


var source = require('fs').readFileSync('test.sl', 'utf8');
console.log(JSON.stringify(
  go(source)
, null, 2));
