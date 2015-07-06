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
  var whitespace = Parsimmon.whitespace;
  var optWhitespace = Parsimmon.optWhitespace;
  var lazy = Parsimmon.lazy;
  var alt = Parsimmon.alt;

  var comment = regex(/;[^\n]*/).skip(string('\n')).desc('comment');
  var ignore = comment.or(regex(/\s*/));
  function lexeme(p) { return p.skip(ignore); }

  var expr = lazy('s-expression', function() { return form.or(atom); });

  var str = lexeme(regex(/"((?:\\.|.|\s)*?)"/, 1))
    .map(interpretEscapes)
    .desc('string');
  var number = lexeme(regex(/-?(0|[1-9]\d*)([.]\d+)?(e[+-]?\d+)?/i))
    .map(parseFloat)
    .desc('number');
  var id = lexeme(regex(/[a-z!$%&*+-.\/:<=>?@^_~|][a-z0-9!$%&*+-.\/:<=>?@^_~|]*/i))
    .map(function(s) { return new Symbol(s); })
    .desc('identifier');

  var nullLiteral = lexeme(string('null').then(whitespace)).result(null).desc('string');
  var trueLiteral = lexeme(string('true').then(whitespace)).result(true).desc('boolean');
  var falseLiteral = lexeme(string('false').then(whitespace)).result(false).desc('boolean');

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
        type: 'list',
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

  var body = [];
  var params = [];
  var functionIdentifier = null;
  var rawParamsList = form.items[1];
  var bodyExprsExcludingLast = form.items.slice(2, -1);

  if (form.items[1] instanceof Symbol) {
    rawParamsList = form.items[2]; // use 2nd arg for params list
    bodyExprsExcludingLast = form.items.slice(3, -1); // offset body
    functionIdentifier = buildIdentifierExpr(form.items[1]); // define identifier
  }

  if (rawParamsList.type !== 'list') {
    throw new Error('The "function" expression takes a list of arguments as first parameter.');
  }
  for (var i in rawParamsList.items) {
    if (!(rawParamsList.items[i] instanceof Symbol)) {
      throw new Error('The "function" expression takes a list of arguments names, you passed a non-symbol as argument name');
    }
  }

  // Params
  var rawParams = rawParamsList.items;
  for (var i in rawParams) {
    var param = rawParams[i];
    if (param.toString() === '.' && parseInt(i) === rawParams.length - 2) {
      body.push(buildDefinitionExpr({
        type: 'list',
        items: [new Symbol('def'), rawParams[parseInt(i)+1], {
          type: 'list',
          items: [new Symbol('Array.prototype.slice.call'), new Symbol('arguments'), rawParams.length-2]
        }]
      }));
      break;
    } else {
      params.push(buildIdentifierExpr(param));
    }
  }


  // Body
  for (var i in bodyExprsExcludingLast) {
    body.push(buildExpressionStatement(writeForm(bodyExprsExcludingLast[i])))
  }
  body.push({
    type: 'ReturnStatement',
    argument: writeForm(form.items[form.items.length - 1])
  });

  return {
    type: 'FunctionExpression',
    id: functionIdentifier,
    params: params,
    defaults: [],
    body: {
      type: 'BlockStatement',
      body: body,
      generator: false,
      expression: true
    }
  };
};

var buildUnaryExpr = function(form) {
  return {
    type: 'UnaryExpression',
    operator: form.items[0].toString(),
    argument: writeForm(form.items[1]),
    prefix: true
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

var methodTypeEq = function(type) {
  return function(form) {
    var typeOfExpr = {
      type: 'list',
      items: [new Symbol('typeof'), form.items[1]]
    };
    return buildBinaryExpr({
      type: 'list',
      items: [new Symbol('==='), typeOfExpr, type]
    });
  };
};

var methodEqLiteral = function(literal) {
  return function(form) {
    if (!form.items || form.items.length !== 2) {
      throw new Error('Method "' + literal + ' takes exactly 1 parameter. Got ' + form.items.length-1 + '.');
    }
    return buildBinaryExpr({
      items: [new Symbol('==='), form.items[1], literal]
    });
  };
};

var buildDefinitionExpr = function(form) {
  if ((form.items.length-1) % 2 !== 0) {
    throw new Error('The "def" expression takes a pair number of arguments.');
  }

  var declarations = [];
  var rawDeclarations = form.items.slice(1);
  for (var i = 0; i < rawDeclarations.length; i += 2) {
    declarations.push({
      type: 'VariableDeclarator',
      id: buildIdentifierExpr(rawDeclarations[i]),
      init: writeForm(rawDeclarations[i + 1])
    })
  }

  return {
    type: 'VariableDeclaration',
    kind: 'var',
    declarations: declarations
  };
};

var buildAssignmentExpr = function(form) {
  if (form.items.length !== 3) {
    throw new Error('The "set!" method takes exactly 2 parameters. Got ' + form.items.length-1 + '.');
  }
  if (!(form.items[0] instanceof Symbol)) {
    throw new Error('The "set!" method takes only symbols as 1st parameter.');
  }
  return {
    type: 'AssignmentExpression',
    operator: '=',
    left: buildIdentifierExpr(form.items[1]),
    right: writeForm(form.items[2])
  };
};

var buildTryExpr = function(form) {

};

var buildImportExpr = function(form) {
  var declarations = [];

  for (var i in form.items.slice(1)) {
    var declaration = form.items.slice(1)[i];
    var n = parseInt(i)+1;

    if (!declaration || !declaration.type || declaration.type !== 'list') {
      throw new Error('The parameter #' + n + ' passed to the "import" function can only be a list.');
    }
    if (typeof declaration.items[0] !== 'string') {
      throw new Error('The 1st element of parameter #' + n + ' passed to the "import" function can only be a string.');
    }

    function pushRequire(name, mod, part) {
      var reqExpr = {
        type: 'list',
        items: [new Symbol('require'), mod]
      };

      if (part) {
        var justRequireExpr = reqExpr;
        reqExpr = {
          type: 'list',
          items: [new Symbol('get'), new Symbol(part), reqExpr]
        };
      }

      declarations.push(buildDefinitionExpr({
        type: 'list',
        items: [new Symbol('def'), new Symbol(name), require]
      }));
    }

    // Simple require
    var requireName = declaration.items[0].toString();
    if (declaration.items.length === 1) {

      pushRequire(requireName, declaration.items[0], null);

    } else if (declaration.items.length === 3 && declaration.items[1] instanceof Symbol
      && declaration.items[1].toString() === 'as') {

      pushRequire(declaration.items[2].toString(), requireName, null);

    } else if (declaration.items.length === 3 && declaration.items[1] instanceof Symbol
      && declaration.items[1].toString() === 'refer') {

      if (!declaration.items[2] || !declaration.items[2].type || declaration.items[2].type !== 'list') {
        throw new Error('The parameter #' + n + ' passed to the "import" specified "refer" but not followed by a list.');
      }
      for (var j in declaration.items[2].items) {
        var referedName = declaration.items[2].items[j].toString();
        pushRequire(referedName, requireName, referedName);
      }

    }

  }

  return declarations;
};

var buildGetExpr = function(form) {
  var computed = !(form.items[1] instanceof Symbol);
  return {
    type: 'MemberExpression',
    computed: computed,
    object: writeForm(form.items[1]),
    property: writeForm(form.items[1])
  }
};

var builtins = {
  'def': buildDefinitionExpr,
  'set!': buildAssignmentExpr,
  'import': buildImportExpr,
  'get': buildGetExpr,

  'if': buildIfExpr,
  'function': buildFunctionExpr,
  'new': buildNewExpr,
  'try': buildTryExpr,
  'throw': buildUnaryExpr,

  'instanceof': buildBinaryExpr,

  'typeof': buildUnaryExpr,
  'void': buildUnaryExpr,

  '+': buildBinaryExpr,
  '-': buildBinaryExpr,
  '*': buildBinaryExpr,
  '/': buildBinaryExpr,
  '%': buildBinaryExpr,

  '<': buildBinaryExpr,
  '<=': buildBinaryExpr,
  '>': buildBinaryExpr,
  '>=': buildBinaryExpr,
  '||': buildBinaryExpr,
  '&&': buildBinaryExpr,
  '==': buildBinaryExpr,
  '===': buildBinaryExpr,
  '!=': buildBinaryExpr,
  '!==': buildBinaryExpr,

  'null?': methodEqLiteral(null),
  'true?': methodEqLiteral(true),
  'false?': methodEqLiteral(false),
  'undefined?': methodTypeEq('undefined'),
  'boolean?': methodTypeEq('boolean'),
  'number?': methodTypeEq('number'),
  'string?': methodTypeEq('string'),
  'object?': methodTypeEq('object'),
  'array?': methodTypeEq('array'),
  'function?': methodTypeEq('function'),
};

var writeForm = function(form) {
  if (form === null) { return {type: 'Literal', value: null};
  } else if (form === true) { return {type: 'Literal', value: true};
  } else if (form === false) { return {type: 'Literal', value: false};
  } else if (form.type === 'list') {
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
    var form = writeForm(ast[form]);
    if ('length' in form) { // support arrays returned by writeForm
      cgReadyAst.body = cgReadyAst.body.concat(form.map(buildExpressionStatement));
    } else {
      cgReadyAst.body.push(buildExpressionStatement(form));
    }
  }

  // DEBUG
  //console.log(JSON.stringify(cgReadyAst, null, 1));
  return escodegen.generate(cgReadyAst);
};

module.exports = {
  parse: parse,
  write: write
};

/*
 * For TCO

var count = function count(f, t) {
  return function loop() {
    var recur = loop;
    var fø2 = f;
    var tø2 = t;
    do {
      // other exprs
      recur = fø2 >= tø2 ? fø2 : (loop[0] = 1 + fø2, loop[1] = tø2, loop);
    } while (fø2 = loop[0], tø2 = loop[1], recur === loop);
    return recur;
  }.call(this);
};

*/
