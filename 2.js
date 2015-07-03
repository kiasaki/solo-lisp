var Parsimmon = require('parsimmon');
var escodegen = require('escodegen');

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
  var result = read.parse(source);
  if (!result.status) {
    throw new Error(Parsimmon.formatError(source, result));
  } else {
    return result.value;
  }
};

var builtins = {
  def: function(form) {
    // TODO Guard against items.length != 3
    return {
      type: 'VariableDeclaration',
      kind: 'let',
      declarations: [{
        type: 'VariableDeclarator',
        id: {
          type: 'Identifier',
          name: form.items[1]
        },
        init: writeForm(form.items[2])
      }]
    };
  }
};

var writeForm = function(form) {
  if (typeof form === 'object' && form.type) {
    if (form.type === 'list') {
      if (form.items[0] && form.items[0] in builtins) {
        return builtins[form.items[0]](form);
      } else {
        // treat as function call
        return {
          type: 'CallExpression',
          callee: {
            type: 'Identifier',
            name: form.items[0]
          },
          arguments: form.items.slice(1).map(function(item) {
            return writeForm(item);
          })
        }
      }
    } else {
      // TODO other (array, obj)
    }
  } else {
    return {
      type: 'Literal',
      value: form
    };
  }
};

var write = function(ast) {
  var cgReadyAst = {
    type: 'Program',
    body: []
  };

  for (var form in ast) {
    cgReadyAst.body.push(writeForm(ast[form]));
  }

  //console.log(JSON.stringify(cgReadyAst, null, 1));
  return escodegen.generate(cgReadyAst);
};

var source = require('fs').readFileSync('test.sl', 'utf8');
console.log(write(parse(source)));

/*

;(set asd {a 1 b 2})

;(function ()
;  (if true
;    false
;    9))

;(console.log "asd")
*/
