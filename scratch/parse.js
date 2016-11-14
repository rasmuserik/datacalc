var s = require('fs').readFileSync("code.ul", "utf-8").split('');

// NB: written in a style easily translateable into touchlang, 
// not nice JS-style, and will overflow stack on large data

function List(car, cdr) {
  this.car = car;
  this.cdr = cdr;
};
List.prototype.first = function() {
  return this.car;
};
List.prototype.rest = function() {
  return this.cdr;
};
List.prototype.reverse = function() {
  var result = null;
  var cell = this;
  while(cell !== null) {
    result = cons(cell.car, result);
    cell = cell.cdr;
  }
  return result;
};
List.prototype.toArray = function() {
  var result = [];
  var cell = this;
  while(cell) {
    result.push(cell.car);
    cell = cell.cdr;
  }
  return result
};
List.prototype.toJSON = function() {
  return this.toArray().map(function(o) {
      return (o && o.toJSON) ? o.toJSON() : o;
  });
};
function cons(car, cdr) {
  return new List(car, cdr);
}
List.fromArray = function(arr) {
  var i = arr.length;
  var result = null;
  while(i) {
    --i;
    result = cons(arr[i], result);
  }
  return result;
};

function tokenize(chars, tokens) {
  var c = chars && chars.first();
  if(c === null) {
    return tokens.reverse();
  } else if(c === ' ' || c === '\n') {
    return tokenize(chars.rest(), tokens);
  } else if(c == ')' || c == '(') {
    return tokenize(chars.rest(), cons(c,  tokens));
  } else if(c == '"') {
    var result = c;
    c = "";
    while(chars !== null && c !== '"') {
      chars = chars.rest();
      c = chars && chars.first();
      if(c === "\\") {
        chars = chars.rest();
        c = chars && chars.first();
        result += c;
      } else {
        result += c;
      }
    }
    return tokenize(chars.rest(), cons(result, tokens))
  } else if('0' <= c && c <= '9') {
    var result = '';
    while(chars !== null && '0' <= c && c <= '9') {
      result += c;
      chars = chars.rest();
      c = chars && chars.first();
    }
    return tokenize(chars, cons(parseInt(result, 10), tokens))
  } else {
    var result = '';
    while(chars !== null && "() \n".indexOf(c) === -1) {
      result += c;
      chars = chars.rest();
      c = chars && chars.first();
    }
    return tokenize(chars, cons(result, tokens));
  }
}

function reverse(list) {
  return list && list.reverse();
}
function makeTree(tokens, stack, acc) {
  if(tokens === null) {
    return acc.reverse();
  } else {
    var token = tokens.first();
    var tokens = tokens.rest();
    if(token === "(") {
      return makeTree(tokens, cons(acc, stack), null);
    } else if(token === ")") {
      return makeTree(tokens, stack.rest(), cons(acc.reverse(), stack.first()));
    } else {
      return makeTree(tokens, stack, cons(token, acc));
    }
  }
}

function parse(name, expr, i, acc) {
  if(name && name !== "_") { 
    name = name;
  } else {
    name = "$" + i;
    i = i + 1
  }
  if(expr.constructor === List) {
    var fn = expr.first();
    var list = expr.rest();
    var args = null;
    while(list !== null) {
      expr = list.first();
      list = list.rest();
      i = i + 1;
      var id = "$" + i;
      if(typeof expr === "string" && expr[0] !== '"') {
        args = cons(expr, args);
      } else {
        var pair = parse(id, expr, i, acc);
        i = pair.car;
        acc = pair.cdr;
        args = cons(id, args);
      }
    }
    return cons(i, cons({id: name, fn: fn, args: reverse(args)}, acc));
  } else {
    return cons(i+1, cons({id: name, data: expr}, acc));
  }
}
function parsePairs(list, i, acc) {
  if(list === null) {
    return acc;
  } else {
    var name = list.first();
    list = list.rest();
    var pair = parse(name, list.first(), i, acc);
    i = pair.car;
    acc = pair.cdr;
    list = list.rest();
    return parsePairs(list, i, acc);
  }
}
s =  List.fromArray(s);
tokens = tokenize(s, null)
console.log(JSON.stringify(tokens));
tree = makeTree(tokens, null, null);
console.log(JSON.stringify(tree));
console.log(JSON.stringify(parsePairs(tree, 0, null)));
    
