var Parsimmon = require('parsimmon');
var escodegen = require('escodegen');

function Symbol(sym) {
  this.value = sym;
  this.type = 'symbol';
}
Symbol.prototype.toString = function() {
  return this.value;
};

var read = (function() {
  function interpretEscapes(str) {
    var escapes = {
      b: '\b',
      f: '\f',
      n: '\n',
      r: '\r',
      t: '\t'
    };

    return str.replace(/\\(u[0-9a-f]{4}|[^u])/i, function(_, escape) {
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
  var id = lexeme(regex(/[a-z!$%&*+-.\/:<=>?@^_~|][a-z0-9!$%&*+-.\/:<=>?@^_~|]*/i))
    .map(function(s) { return new Symbol(s); })
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

var buildExpressionStatement = function(expr) {
  // Don't wrap things that are already expressions
  if (expr.type === 'VariableDeclaration') return expr;
  return {
    type: 'ExpressionStatement',
    expression: expr
  };
};

var buildBinaryExpr = function(form) {
  // TODO Guard min length 3
  if (form.items.length < 3) {
    throw new Error('The ' + form.items[0] + ' operator need a minimum or 3 params');
  } else if (form.items.length === 3) {
    return {
      type: 'BinaryExpression',
      operator: form.items[0].toString(),
      left: writeForm(form.items[1]),
      right: writeForm(form.items[2])
    };
  } else {
    return {
      type: 'BinaryExpression',
      operator: form.items[0].toString(),
      left: writeForm(form.items[1]),
      right: buildBinaryExpr({
        items: [form.items[0]].concat(form.items.slice(2))
      })
    };
  }
};

var buildIdentifierExpr = function(form) {
  return {
    type: 'Identifier',
    name: form.toString()
  };
}

var buildIfExpr = function(form) {
  if (form.items.length !== 4) {
    throw new Error('The "if" expression takes exactly 3 parameters, ' + form.items.length-1 + ' given.');
  }
  return {
    type: 'ConditionalExpression',
    test: writeForm(form.items[1]),
    consequent: writeForm(form.items[2]),
    alternate: writeForm(form.items[3])
  };
};

var buildFunctionExpr = function(form) {
  if (form.items.length < 3) {
    throw new Error('The "function" expression takes minimum 2 parameters, ' + form.items.length-1 + ' given.');
  }
  if (form.items[1].type !== 'list') {
    throw new Error('The "function" expression takes a list of arguments as first parameter.');
  }
  for (var i in form.items[1].items) {
    if (!(form.items[1].items[i] instanceof Symbol)) {
      throw new Error('The "function" expression takes a list of arguments names, you passed a non-symbol as argument name');
    }
  }

  var body = [];
  var bodyExprsExcludingLast = form.items.slice(2, -1);
  for (var i in bodyExprsExcludingLast) {
    body.push(buildExpressionStatement(writeForm(bodyExprsExcludingLast[i])))
  }
  body.push({
    type: 'ReturnStatement',
    argument: writeForm(form.items[form.items.length - 1])
  });

  return {
    type: 'FunctionExpression',
    id: null,
    params: form.items[1].items.map(buildIdentifierExpr),
    defaults: [],
    body: {
      type: 'BlockStatement',
      body: body,
      generator: false,
      expression: true
    }
  };
};

var buildNewExpr = function(form) {
  return {
    type: 'NewExpression',
    callee: writeForm(form.items[1]),
    arguments: form.items.slice(2).map(function(item) {
      return writeForm(item);
    })
  };
};
var buildCallExpr = function(form) {
  return {
    type: 'CallExpression',
    callee: writeForm(form.items[0]),
    arguments: form.items.slice(1).map(function(item) {
      return writeForm(item);
    })
  };
};

var buildArrayExpr = function(form) {
  return {
    type: 'ArrayExpression',
    elements: form.items.map(writeForm)
  };
};

var buildObjectExpr = function(form) {
  if (form.items.length % 2 !== 0) {
    throw new Error('The "object" expression must be given a pair number of elements. ' + form.items.length + ' given.');
  }

  var properties = [];
  for (var i = 0; i < form.items.length; i += 2) {
    if (!(form.items[i] instanceof Symbol)) {
      throw new Error('All pair elements of the "object" expression must be "symbols", ' + form.items[i].toString() + ' given as element #' + (i+1) + '.');
    }

    properties.push({
      type: 'Property',
      key: buildIdentifierExpr(form.items[i]),
      value: writeForm(form.items[i+1]),
      kind: 'init',
      method: false,
      shorthand: false,
      computed: false
    });
  }

  return {
    type: 'ObjectExpression',
    properties: properties
  };
};

var builtins = {
  def: function(form) {
    // TODO Guard against items.length != 3
    return {
      type: 'VariableDeclaration',
      kind: 'let',
      declarations: [{
        type: 'VariableDeclarator',
        id: buildIdentifierExpr(form.items[1]),
        init: writeForm(form.items[2])
      }]
    };
  },
  'if': buildIfExpr,
  'function': buildFunctionExpr,
  'new': buildNewExpr,
  '+': buildBinaryExpr,
  '-': buildBinaryExpr,
  '*': buildBinaryExpr,
  '/': buildBinaryExpr,
  '<': buildBinaryExpr,
  '<=': buildBinaryExpr,
  '>': buildBinaryExpr,
  '>=': buildBinaryExpr,
  '||': buildBinaryExpr,
  '&&': buildBinaryExpr,
  '==': buildBinaryExpr,
  '===': buildBinaryExpr,
  '!=': buildBinaryExpr,
  '!==': buildBinaryExpr
};

var writeForm = function(form) {
  if (form.type === 'list') {
    if (form.items[0] && form.items[0].toString() in builtins) {
      return builtins[form.items[0].toString()](form);
    } else {
      // treat as function call
      return buildCallExpr(form);
    }
  } else if (form.type === 'array') {
    return buildArrayExpr(form);
  } else if (form.type === 'object') {
    return buildObjectExpr(form);
  } else if (form instanceof Symbol) {
    return buildIdentifierExpr(form);
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
    cgReadyAst.body.push(buildExpressionStatement(writeForm(ast[form])));
  }

  //console.log(JSON.stringify(cgReadyAst, null, 1));
  return escodegen.generate(cgReadyAst);
};

module.exports = {
  parse: parse,
  write: write
};
