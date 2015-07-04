// Solo Lisp
//
// int ([-+]?[0-9]+(\\.[0-9]*)?([eE][-+]?[0-9]+)?)(M)?
// float ^([-+]?)(?:(0)|([1-9][0-9]*)|0[xX]([0-9A-Fa-f]+)|0([0-7]+)|([1-9][0-9]?)[rR]([0-9A-Za-z]+)|0[0-9]+)(N)?$

var TYPE_COMMENT = 'comment';
var TYPE_STRING = 'string';
var TYPE_IDENTIFIER = 'identifier';
var TYPE_NUMBER = 'number';
var TYPE_LIST = 'list';
var TYPE_OBJECT = 'object';
var TYPE_ARRAY = 'array';

function UnmatchedDelimiterError(message) {
  this.constructor.prototype.__proto__ = Error.prototype;
  Error.captureStackTrace(this, this.constructor);
  this.name = this.constructor.name;
  this.message = message;
}

var solo = {};

solo.reader = (function() {
  // --- Core

  // Returns an object representing a Reader
  function newReader(source, uri) {
    return {
      line: 0,
      column: -1,
      uri: uri || '(lisp)',
      lines: source.split('\n')
    };
  }

  // Location URI used in sub readers instansiation
  function readerLocationUri(reader) {
    return reader.uri.split('#')[0] + '#' + (reader.line + 1) + ':' + (reader.column + 1);
  }

  // Return next char without reading it, or, nil if we reached the end of
  // the stream
  function peekChar(reader) {
    var line = reader.lines[reader.line];
    if (!line) return null;
    return line[reader.column + 1] || '\n';
  }

  // Returns the next char, moving the reader's cursor
  function nextChar(reader) {
    var ch = peekChar(reader);
    if (isNewline(ch)) {
      reader.line++; // Advance to next line
      while (reader.lines[reader.line] === '') {
        reader.line++; // Skip empty lines
      }

      reader.column = -1;
    } else {
      reader.column++;
    }
    return ch;
  }

  function currentChar(reader) {
    var line = reader.lines[reader.line];
    if (!line) return null;
    return line[reader.column] || '\n';
  }

  // Return the next n chars, moving the reader's cursor
  function nextNChars(reader, n) {
    var buffer = '';
    for (var i = 0; i < n; i++) {
      var ch = nextChar(reader);
      if (!ch) return ch;
      buffer += ch;
    }
    return buffer;
  }

  function readerError(reader, mess) {
    var message = mess + '\nline:' + reader.line + '\ncolumn:' + reader.column;
    var err = new SyntaxError(message, reader.uri);
    err.line = reader.line;
    err.column = reader.column;
    err.uri = reader.uri;
    throw err;
  }

  // --- Tests

  function isNewline(ch) {
    return ch === '\n';
  }

  function isWhitespace(ch) {
    return [' ', '\t', '\n', '\r'].indexOf(ch) !== -1;
  }

  function isNumberStart(reader, ch) {
    var numbers = '0,1,2,3,4,5,6,7,8,9'.split(',');
    if (numbers.indexOf(ch) !== -1) return true;
    if (['-', '+'].indexOf(ch) !== -1) {
      return numbers.indexOf(peekChar(reader)) !== -1;
    }
    return false;
  }

  function not(fn) {
    return function() {
      return !fn.apply(this, arguments);
    };
  }

  function eq(a) {
    return function(b) {
      return a === b;
    };
  }

  // --- Readers

  function readUnmatchedDelimiter(reader, ch) {
    var err = new UnmatchedDelimiterError('Unmatched delimiter "' + ch + '"');
    err.line = reader.line;
    err.column = reader.column;
    err.uri = reader.uri;
    throw err;
  }

  function makeUnicodeChar(codeStr) {
    var code = parseInt(codeStr, 16);
    return String.fromCharCode(code);
  }

  function validateUnicodeEscape(pattern, reader, escapeCh, codeStr) {
    if (pattern.test(codeStr)) {
      return codeStr;
    } else {
      readerError(reader, 'Unexpected unicode escape \\' + escapeCh + codeStr);
    }
  }

  function escapeCharSelect(ch) {
    switch(ch) {
      case 't': return '\t';
      case 'r': return '\r';
      case 'n': return '\n';
      case '\\': return '\\';
      case '"': return '"';
      case '\'': return '\'';
      case 'b': return '\b';
      case 'f': return '\f';
      default: return null;
    }
  }

  function escapeChar(reader, ch) {
    var escaped = escapeCharSelect(ch);
    if (escaped) {
      return escaped;
    } else if (ch === 'x') {
      return makeUnicodeChar(validateUnicodeEscape(
        /[0-9A-Fa-f]{2}/, reader, ch, nextNChars(reader, 2)));
    } else if (ch === 'u') {
      return makeUnicodeChar(validateUnicodeEscape(
        /[0-9A-Fa-f]{4}/, reader, ch, nextNChars(reader, 4)));
    } else {
      readerError('Unexpected unicode escape \\' + ch);
    }
  }

  function readString(reader, delimiter) {
    var buffer = '';
    var ch;
    nextChar(reader);
    while (ch = nextChar(reader)) {
      if (ch === '\\') {
        buffer += escapeChar(reader, nextChar(reader));
      } else if (ch === delimiter) {
        return {type: TYPE_STRING, value: buffer};
      } else {
        buffer += ch;
      }
    }
    // We hit EOF (nextChar() == null)
    readerError(reader, 'Reached end-of-file while reading a string');
  }

  function readComment(reader, ch) {
    var buffer = '';
    var ch;
    while (ch = nextChar(reader)) {
      if (ch === '\n') {
        return {type: TYPE_COMMENT, value: buffer};
      } else if (ch === '\\') {
        buffer += escapeChar(reader, nextChar(reader));
      } else {
        buffer += ch;
      }
    }
    // We hit EOF (nextChar() == null)
    readerError(reader, 'Reached end-of-file while reading a comment');
  }

  function readUntil(reader, predicate) {
    var buffer = '';
    var ch;
    while ((ch = peekChar(reader)) && predicate(ch)) {
      buffer += ch;
      nextChar(reader); // Don't consume non predicate matches
    }
    if (!ch) return null; // We stopped because of EOF not predicate
    return buffer;
  }

  function buildSetReader(type, closingCh) {
    return function(reader, ch) {
      var forms = [];
      nextChar(reader); // Consume opening

      // TODO Fix here
      while (ch = peekChar(reader)) {
        var r = readUntil(reader, isWhitespace);
        if (r === null) readerError(reader, 'End-of-file reached reading list, missing ' + closingCh);
        if (ch === closingCh) {
          nextChar(reader); // Consume closing
          return {type: type, items: forms};
        } else {
          try {
            forms.push(readForm(reader));
          } catch (err) {
            if (err instanceof UnmatchedDelimiterError) {
              // Cool, we read the whole set
            } else {
              throw err;
            }
          }
        }
      }
      nextChar(reader); // consume closing )
    }
  }

  var readList = buildSetReader(TYPE_LIST, ')');
  var readArray = buildSetReader(TYPE_ARRAY, ']');
  var readObject = buildSetReader(TYPE_OBJECT, '}');

  function readIdentifier(reader, ch) {
    var ident = ch + readUntil(reader, not(isWhitespace));
    return {type: TYPE_IDENTIFIER, value: ident};
  }

  function readNumber(reader, ch) {
    var num = ch + readUntil(reader, function(ch) {
      return !isWhitespace(ch) && [')', '}', ']'].indexOf(ch) === -1;
    });
    return {type: TYPE_NUMBER, value: num};
  }

  function readersSelect(ch) {
    switch (ch) {
      case "'": return readString;
      case '"': return readString;
      case ';': return readComment;
      case '(': return readList;
      case ')': return readUnmatchedDelimiter;
      case '[': return readArray;
      case ']': return readUnmatchedDelimiter;
      case '{': return readObject;
      case '}': return readUnmatchedDelimiter;
      default: return null;
    }
  }

  function _readForm(reader) {
    var ch = peekChar(reader);
    if (!ch) return null;
    var readHandler = readersSelect(ch);

    if (readHandler) {
      return readHandler(reader, ch);
    } else {
      // not else if because we need to advance reader to have 2 next chars
      var ch = nextChar(reader);
      if (isNumberStart(reader, ch)) {
        return readNumber(reader, ch);
      } else {
        return readIdentifier(reader, ch);
      }
    }
  }

  function readForm(reader) {
    var start = {line: reader.line, column: reader.column};
    var form = _readForm(reader);
    var end = {line: reader.line, column: reader.column};
    form.location = {start: start, end: end, uri: reader.uri};
    return form;
  }

  function read(reader) {
    var ast = [];
    var ch;

    var start;
    try {
      while (ch = peekChar(reader)) {
        if (!isWhitespace(ch)) {
          start = {line: reader.line, column: reader.column};
          ast.push(readForm(reader));
        } else {
          nextChar(reader); // Consume char & go on
        }
      }
    } catch (err) {
      // TODO: Debugging
      console.log('Error: ' + err.message + '\nuri:' + reader.uri, start);
      return ast;
    }

    return ast;
  }


  return {
    newReader: newReader,
    read: read
  };
})();

solo.parse = function (source, uri) {
  var reader = solo.reader.newReader(source, uri);
  return solo.reader.read(reader);
}

solo.write = (function() {
  function writeString(string) {
    string = string.replace(/\\\\/g, '\\\\');
    string = string.replace(/\n/g, '\\n');
    string = string.replace(/\r/g, '\\r');
    string = string.replace(/\t/g, '\\t');
    string = string.replace(/\"/g, '\\"');
    return '"' + string + '"';
  }

  function writeBlock() {

  }

  function writeFunction(astNode) {
    // TODO Guard against less than 1 items
    // TODO Guard against param 1 not being a list
    return 'function(' + astNode.items[1].items.map(writeNode).join(', ') + ')'
      + ' {\n  ' + write(astNode.items.slice(2)).replace(/\n/g, '\n  ') + '\n}';
  }

  function writeIf(astNode) {
    // TODO Guard against less than 1 items
    return 'if (' + writeNode(astNode.items[1]) + ')'
      + ' {\n  ' + writeNode(astNode.items[2]).replace('\n', '\n  ') + '\n' +
        '} else {\n  ' + writeNode(astNode.items[3]).replace('\n', '\n  ') + '\n}';
  }

  function writeList(astNode) {
    // TODO add callee type check
    // TODO add callee primitives handling
    if (astNode.items.length === 0) return '';
    var callee = astNode.items[0].value;
    var params = astNode.items.slice(1);

    if (callee === 'function') return writeFunction(astNode);
    if (callee === 'if') return writeIf(astNode);

    return callee + '(' + params.map(writeNode).join(', ') + ');';
  }

  function writeNode(astNode) {
    switch (astNode.type) {
      case TYPE_COMMENT: return '//' + astNode.value.replace(/^;+/, '') + '\n';
      case TYPE_STRING: return writeString(astNode.value);
      case TYPE_IDENTIFIER: return astNode.value;
      case TYPE_NUMBER: return astNode.value;
      case TYPE_LIST: return writeList(astNode).trim(' ');
      case TYPE_ARRAY: return '[' + write(astNode.items).trim(' ') + ']';
      case TYPE_OBJECT: return '{' + write(astNode.items).trim(' ') + '}';
    }
  }

  function write(ast) {
    var buffer = '';
    for (var i in ast) {
      var node = ast[i];

      // Add line returns
      if (ast[i - 1] && node.location.start.line != ast[i - 1].location.start.line) {
        var returns = new Array(node.location.start.line - ast[i - 1].location.start.line + 1);
        buffer += returns.join('\n');
      }

      buffer += writeNode(node);

      if (buffer[buffer.length - 1] !== '\n') {
        buffer += ' '; // Space elements (not after new lines)
      }
    }

    return buffer;
  }

  return write;
})();

module.exports = solo;
