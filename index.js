// Solo Lisp
//
// int ([-+]?[0-9]+(\\.[0-9]*)?([eE][-+]?[0-9]+)?)(M)?
// float ^([-+]?)(?:(0)|([1-9][0-9]*)|0[xX]([0-9A-Fa-f]+)|0([0-7]+)|([1-9][0-9]?)[rR]([0-9A-Za-z]+)|0[0-9]+)(N)?$

var reader = (function() {
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


  // --- Readers

  function readUnmatchedDelimiter(reader, ch) {
    readerError(reader, 'Unmatched delimiter "' + ch '"');
  }

  function makeUnicodeChar(codeStr) {
    var code = parseInt(codeStr, 16);
    return String.fromCharCode(code);
  }

  function validateUnicodeEscape(pattern, reader, escapeCh, codeStr) {
    if (pattern.match(codeStr)) {
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
    nextChar(reader);
    while (var ch = nextChar(reader)) {
      if (ch === '\\') {
        buffer += escapeChar(reader, nextChar(reader));
      } else if (ch === delimiter) {
        return new String(buffer);
      } else {
        buffer += ch;
      }
    }
    // We hit EOF (nextChar() == null)
    readerError(reader, 'Reached end-of-file while reading a string');
  }

  function readComment(reader, ch) {
  
  }

  function readList(reader, ch) {
  
  }

  function readArray(reader, ch) {
  
  }

  function readObject(reader, ch) {
    
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

  function read(reader) {
    var ch = peekChar(reader);
    if (!ch) return null;
    var readHandler = readersSelect(ch);

    return handler(reader, ch);
  }

  return {
    newReader: newReader,
    read: read
  };
})();

var parse = function (source, uri) {
  var r = reader.newReader(source, uri);
  var ast = [];

  while (var form = reader.read(r)) {
    ast.push(form);
  }

  return ast;
}

module.exports = {
  reader: reader,
  parse: parse
};
