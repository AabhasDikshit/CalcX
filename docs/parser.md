# Parser Notes

The tokenizer emits adjacent tokens without inserting multiplication. This preserves source information and lets the parser decide when adjacency is unambiguous:

- `3x` becomes number plus identifier
- `2sin(x)` becomes number plus identifier plus parentheses
- `(x+1)(x-1)` becomes two parenthesized expressions

The next milestone will consume this stream with precedence-aware recursive descent. Unary operators and exponentiation need separate precedence handling so expressions such as `-x^2` can be interpreted consistently.
