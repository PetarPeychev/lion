# Lion
Lion is an interpreted concatenative programming language without all the stack shuffling pain. The focus is on lisp-like homoiconicity, reflection and metaprogramming.

- [Lion](#lion)
  - [Execution Model](#execution-model)
  - [Types](#types)
    - [Numbers](#numbers)
    - [Strings](#strings)
    - [Symbols](#symbols)
    - [Lists](#lists)

## Execution Model
Lion is designed to be maximally self-referential and as such, lion programs themselves and the way they are executed is defined using lion semantics and terminology. Therefore, the following summary will only make sense once you understand all of the language semantics, however here it is anyway:

A lion program is a **string** of code which is then **parsed** into a **list** of **values** and **called** with a root **dictionary** of built-in **definitions**.

## Types
Lion programs support 4 distinct types of values.

### Numbers

```
0
42
58.0
34.98
```

### Strings

```
""
"abc 123"
"漢字"
```

### Symbols

```
x
abc1
+
```

### Lists

```
[]
["abc"]
[1 "two" 3 ["four" five]]
```
