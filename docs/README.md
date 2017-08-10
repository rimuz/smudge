# The Smudge Programming Language - Documentation
The Smudge Documentation (aka **SmudgeDocs**) has two different Portals.
One for the users of the Smudge Programming Language (who programs with Smudge),
and another one for the contributors to the project (or who wants to understand
_how_ does Smudge work). Here are the links:
- [For the users](#for-the-users)
- [For the contributors](#for-the-contributors)

## How to read the `SmudgeDocs`
The `SmudgeDocs` describe how do `classes`, `functions`, `vars` and
`methods`work. The following paragraphs tell you how to read them.

### The Smudge Notation
If you see something like `std.io!::FileStream::read()` and you can't understand
the meaning, _don't worry_! It will be soon clear, so keep reading.

First of all, keep in mind that the **Smudge Notation** (aka the **S::N**)
uses double colons (i.e the `::`) to divide different scopes where you couldn't
use `.`, for example: `BOXNAME::CLASSNAME`, `CLASSNAME::METHOD`
or both `BOXNAME::CLASSNAME::METHOD`.

We add `()` after each **function name** to distinguish it from vars and
classes. Though it's not mandatory, you should also specify its arguments
(especially for documentation purposes).

Last, an _exclamation mark_ (i.e. the `!`) should be added at the end of the
names of the boxes which are part of the `SSL` (sometimes the interpreter
could show box names like `std.lang!`, etc..).

So, a class named `Foo` contained in box named `bar.baz` is called
`bar.baz::Foo`, while the function named `open` in box `std.io` is called
`std.io!::open()`.

### The SmudgeDocs Notation
The **SmudgeDocs** uses the **S::N** notation anywhere but in the **titles**,
where it's omitted the **box name** (and eventually the **class name**, too).
Also, all functions and methods will have **all** of their arguments listed.

**Optional** arguments can be easily recognized because a `=` and their
**default value** will follow the names. To make them more visible, we'll
sorround them by brackets (i.e. `[]`).

For example, `f1(a[, b = 100[, c = f2(a)]])` means `a function f1 which has three arguments`:
 - `a`: which is "mandatory"
 - `b`: which is optional, with as default value `100`
 - `c`: which is optional, with as default value the result of the expression
`f2(a)`

### `EoW`
**`EoW`** (state of Error Or Wrong paramater passed) is a very
common term used by the `SmudgeDocs` to denote the situation
when one or more **prerequisites** of a function are
**violated**. Most of the functions of the SSL return `null`
when an EoW occurs (e.g. function `get()` in `String`, when
the index given is greater than the string's size).

### Congratulations
Now you're **ready** to surf the `SmudgeDocs`! Enjoy it!

## For the users
- **Learn**
    - [Writing an Hello World](learn/hello-world.md)
    - [Statements](learn/statements.md)
    - [Functions](learn/functions.md)
    - [Variables](learn/vars.md)
    - [If, switch and loops](learn/if-and-loops.md)
    - Working in progress..
- The **Smudge Standard Library** (the **SSL**)
    - [std.io](ssl/stdio.md)
        - [Class FileStream](ssl/stdio.md#class-filestream)
    - [std.lang](ssl/stdlang.md)
        - [Class String](ssl/stdlang.md#class-string)
        - [Class List](ssl/stdlang.md#class-list)
        - [Class Tuple](ssl/stdlang.md#class-tuple)
        - [Class StringIterator](ssl/stdlang#class-stringiterator)
        - [Class ListIterator](ssl/stdlang#class-listiterator)
    - [std.system](ssl/stdsystem.md)
        - [Class Chunk](ssl/stdsystem.md#class-chunk)
        - [Class ChunkIterator](ssl/stdsystem.md#class-chunkiterator)
    - Working in progress..

## For the contributors
- Working in progress..
