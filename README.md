# In progress, nothing to see here...

# Simple implementation

## Data structure

node types:

- literal
- fn
- special (lambda)

node properties
- op: \literal, \lambda, \iter, \reti, function-name
- id: random-hash
- args: 
- name: name-spaced name source-generation / documentation
- value: (literals)

data type
- name
- function table
- render-function


## UI

node:

4 lines, 2 result|name+result, 1 expr/name 40x80px
```
+-------------+
| $8   2     |
| $1 op      |
| $4 $3 $5 |
+-------------+
```

360/411/320/375/414/

each
hcea
```
+-------------------+
|                   |
|                   |
|                   |
|                   |
|                   |
|  list-of-nodes    |
|                   |
|                   |
|                   |
|                   |
|                   |
+---+-----------+---+
| < | a .foo ?  | > | 
+---+-----------+---+
```

```
+-------------------+
|                   |
|                   |
|     Node view     |
|                   |
|                   |
+-------------------+
|                   |
|                   |
|  list-of-methods  |
|                   |
|                   |
+---+-----------+---+
| < |  expr.    | > |
+---+-----------+---+
```

- click node / create -> node-view/method-list
- click method -> node-list


## Standard library

- JSON-operations
- http-requests
- React-UI-creation
- JS-interop

## code to/from data conversion

build upon code from BS-thesis

## evaluation

# Roadmap

NB: spineless tagless g-machine

- [ ] conversion between textual-code and data
- [ ] sample code / test data
  - [ ] sample programs
  - [ ] sample library functionality (ie. numbers, strings, turtle, lists, ...)
- [ ] code evaluator
- [ ] simple ui for interacting with the code, append/repl
- [ ] code for compilation

# Definition of language as data structure

Code is a data flow graph. Special "macros" for loops, function definition/export etc.

- node
  - fn
    - ~id~ identifier for the node
    - ~fn~ ns/name of method/function to be called
      - "id" is name of identifier which maps to function
      - Later: ".method" method dispatch
    - ~args~ - list of ids for nodes
    - ~meta~ (optional) JSON object with meta data
  - data
    - ~id~
    - ~data~
    - ~meta~
- representations
  - canonical representation of graph
    - constants sorted by value, and deduplicated
    - sort into independent components, and some kind of topological sort determining order of nodes
  - serialised as JSON
  - mapping to/from canonical text source code
- Special functions
  - ~fn~ (args result) -> callable function. 
  - ~loop~ (args result)
  - ~recur~ (args..) loop witin block
  - ~if~ (boolean val1 val2)
  - ~js~ (arg-count string-body)
- programs 
  - `ul2json.js` translate lisp-like syntax into json-graph
  - `eval.js` prints value of node with name as first parameter
 

- Stack machine
  - code
    - continuation passing style stack machine
      - function-call call/cc
      - `(fn [x] val)` written as `(fn [x k] val)` which actually translates to `(fn [x k] (cps k val)`. "Return" is done with `(cps k result)`.
        - where `(defn cps [k & args] ...)` drops stack frame, retaining only args, and jumps to k
      - how closures with compact code
    - all (mutually recursive) code in module, save in same generation in heap, to allow garbage collection despite cyclic references

```
  core.import(while end world then call push ++ <)
  blah/ooh.import(foo)

  arr = [];
  for(i = 0; i < 100; ++i) {arr = arr.push(i);}

  arr = []
  i = 0
  block = *.start(arr i)
  result = block.end(i.<(100).then(block.call(arr.push(i) i.++()) arr)).call(arr i)

  arr = []
  i = 0
  block = (start world arr i)
  result = (call (end block (then (< i 100) (call block (push arr i) (++ i)) arr)) arr i)

  block = world.start(elem)
  block.end(elem.*elem)
  arr.forEach(block)

  elem = arr.first()
  arr.foreach(world.start(elem).end(elem.*elem))

  block = world.start(f, arr)
  block.end(arr.isEmpty().thenelse([], block.call(f, arr.tail()).push(f.call(arr.head()))))
```


# Data structure
##  Language as data
Code as data instead of text.

```
{"name": "demo/hello"
"args": ["a", "b"],
"code": ["if" ["<" "a" "b"] ["str" "a is smallest"] ["str" "b is smallest"]]}
``` 

transforms into

```
function demo$hello(a, b) {
  return a["<"](b) ? "a is smallest" : "b is smallest";
}
```

# UI
## Elements:

- sexpr - The sexpression generating the currently selected object
- main - Object view: representation of current live object selected, or function + documentation
- fns - Method/function list
- props - special methods on objects, ie. direct access to properties on json-objects/arrays, special easy-access methods etc..
- objs - history of all objects interacted with
- actions - button bar for inteactions / changing modus
  - world - monad with access to state-object, constructors, functions, macros
  - toggle view
  - close - ends current method-call/function
  - delete - backspace one token
  - Make function - select result + parameters in objs to create a new function

## Screens
```
+---------------------+
| (expr ... )         |
+---------------------+
|                     |
|                     |
|     Object          |
|                     |
|                     |
+----+------+---------+
| fn | prop | obj obj |
| fn | prop | obj obj |
| fn | prop | obj obj |
| fn | prop | obj obj |
+----+------+---------+
| 123 abc world ...   |
+---------------------+
```

```
+---------------------+
| (expr ... )         |
+---------------------+
| prop |              |
|      |              |
|      |              |
+------+   Object     |
|  fn  |              |
|      |              |
|      |              |
+---------------------+
| obj obj obj obj obj |
| obj obj obj obj obj |
+---------------------+
| 123 abc world ...   |
+---------------------+
```

```
+---------------------------------+
| (expr ... )                     |
+------+-------------------+------+
| prop |                   | obj  |
|      |                   | obj  |
+------+     Object        | obj  |
|  fn  |                   |      |
|      |                   |      |
+------+-------------------+------+
| 123 abc world ...               |
+---------------------------------+
```
