var code = {};
require('./code.json').map(o => code[o.id] = o);

console.log(code);
function eval(id) {
  var node = code[id];
  var fn = node.fn;
  switch(fn) {
    case undefined:
      return node.data
    case "loop":
      throw fn + ' not implemented';
    case "fn":
      throw fn + ' not implemented';
    case "recur":
      throw fn + ' not implemented';
    case "if":
      var p = eval(node.args[0]);
      if(p) {
        return eval(node.args[1]);
      } else {
        return eval(node.args[2]);
      }
    case "js":
      var args = [];
      var argcount = eval(node.args[0]);
      var body = eval(node.args[1]);
      for(var i = 0; i < argcount; ++i) {
        args.push("$" + i);
      }
      args.push(body);
      return Function.apply(Function,args);
    default:
      var args = node.args.map(eval);
      var f = eval(node.fn);
      console.log(f, args);
      return f.apply(null, args);
  }
}
console.log(eval(process.argv[2]));
