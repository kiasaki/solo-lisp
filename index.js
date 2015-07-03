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
      reader.line++;
      reader.column = -1;
    }
    reader.column++;
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
    readerError(reader, 'Unmatched delimiter "' + ch + '"');
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
    while (ch = peekChar(reader)) {
      if (ch === '\n') {
        return {type: TYPE_COMMENT, value: buffer};
      } else if (ch === '\\') {
        buffer += escapeChar(reader, nextChar(reader));
      } else {
        buffer += ch;
      }
      nextChar(reader); // Only consume here so "returning" case is not affected
    }
    // We hit EOF (nextChar() == null)
    readerError(reader, 'Reached end-of-file while reading a comment');
  }

  function readUntil(reader, predicate) {
    var buffer = currentChar(reader);
    var ch;
    while ((ch = nextChar(reader)) && predicate(ch)) {
      buffer += ch;
    }
    if (!ch) return null; // We stopped because of EOF not predicate
    return buffer;
  }

  function readList(reader, ch) {
    nextChar(reader); // skip openning
    var contents = readUntil(reader, not(eq(')')));
    if (!contents) readerError(reader, 'End-of-file reached reading list, missing )');
    nextChar(reader); // consume closing )
    return {type: TYPE_LIST, items: contents};
  }

  function readArray(reader, ch) {
    nextChar(reader); // skip openning
    var contents = readUntil(reader, not(eq(']')));
    if (!contents) readerError(reader, 'End-of-file reached reading array, missing ]');
    nextChar(reader); // consume closing ]
    return {type: TYPE_ARRAY, items: contents};
  }

  function readObject(reader, ch) {
    nextChar(reader); // skip openning
    var contents = readUntil(reader, not(eq('}')));
    if (!contents) readerError(reader, 'End-of-file reached reading object, missing }');
    nextChar(reader); // consume closing }
    return {type: TYPE_OBJECT, items: contents};
  }

  function readIdentifier(reader, ch) {
    var ident = readUntil(reader, not(isWhitespace));
    return {type: TYPE_IDENTIFIER, value: ident};
  }

  function readNumber(reader, ch) {
    var num = readUntil(reader, not(isWhitespace));
    return {type: TYPE_NUMBER, value: ident};
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
    form.location = {start: start, end: end};
    return form;
  }

  function read(reader) {
    var ast = [];
    var ch;

    try {
    while (ch = peekChar(reader)) {
      if (!isWhitespace(ch)) {
        ast.push(readForm(reader));
      } else {
        nextChar(reader); // Consume char
      }
    }
    } catch (err) {
      console.log('Error: ' + err.message);
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

module.exports = solo;
