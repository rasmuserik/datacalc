var s = require('fs').readFileSync("code.ul", "utf-8").split('');

// NB: written in a style easily translateable into touchlang, not nice JS-style

function arrpush(arr, x) {
  return arr.concat([x]);
}
function tokens(s, pos, tok) {
  var c = s[pos];
  if(pos >= s.length) {
    return tok;
  } else if(c === ' ' || c === '\n') {
    return tokens(s, pos + 1, tok);
  } else if(c == ')' || c == '(') {
    return tokens(s, pos + 1, arrpush(tok, c));
  } else if(c == '"') {
    pos = pos + 1;
    c = s[pos];
    var res = "\"";
    while(pos <= s.length && c !== '"') {
      if(c === "\\") {
        pos = pos + 1;
        c = s[pos];
        res += c;
      } else {
        res += c;
      }
      pos = pos + 1;
      c = s[pos];
    }
    res += c;
    return tokens(s, pos + 1, arrpush(tok, res))
  } else if('0' <= c && c <= '9') {
    var res = c;
    while(pos <= s.length && '0' <= c && c <= '9') {
      res += c;
      pos = pos + 1;
      c = s[pos];
    }
    return tokens(s, pos, arrpush(tok, parseInt(res, 10)))
  } else {
    var res = c;
    while(pos <= s.length && c !== ')' && c !== '(' && c !== " " && c !== "\n") {
      res += c;
      pos = pos + 1;
      c = s[pos];
    }
    return tokens(s, pos, arrpush(tok, res));
  }
}
console.log(tokens(s, 0, []));
