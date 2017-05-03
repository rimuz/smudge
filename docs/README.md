# The Smudge Programming Language - Documentation
The Smudge Documentation (aka **SmudgeDocs**) has two different sections based
on who's directed to:
 - For the users (who develops programs in Smudge)
 - For the contributors
 The SmudgeDocs will be competed in

## How to read the `SmudgeDocs`
The `SmudgeDocs` describe how do `classes`, `functions`, `vars` and `methods`work.

### The Smudge Notation
If you see something like `std.io!::FileStream::read()` and you can't understand
the meaning, _don't worry_! It will be soon clear, so keep reading.

First of all, keep in mind that the **Smudge Notation** (aka the **S::N**)
uses double colons (i.e the `::`) to divide different scopes where you couldn't
use `.`, for example: `BOXNAME::CLASSNAME`, `CLASSNAME::METHOD` or both
`BOXNAME::CLASSNAME::METHOD`.

We add `()` after each **function name** to distinguish it from vars and
classes. Though it's not mandatory, you should also specify its arguments
(especially for documentation purposes).

Last, a _bang_ (i.e. `!`) should be added at the end of the names of the
boxes which are part of the `SSL` (sometimes the interpreter could show
box names like `std.lang!`, etc..).

So, a class named `Foo` contained in box named `bar.baz` is called
`bar.baz::Foo`, while the function named `open` in box `std.io` is called
`std.io!::open()`.

### The SmudgeDocs Notation



## For the users
- **Smudge for dummies**
    - [an Hello World](Overview.md)
- **The Syntax**
    - Basic Syntax
    - Operators
    - Strings
    - Lists and Tuples
    - Functions
    - Boxes
    - Classes
- The **Smudge Standard Library** (the **SSL**)
    - [std.io](stdio.md)

## For the contributors
- How Smudge works
    - Smudge explained
    - The Sources
    - The Tokenizer
    - The Compiler
    - The bytecode
    - The Interpreter
