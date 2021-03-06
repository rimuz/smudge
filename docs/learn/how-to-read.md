# The Smudge Programming Language -  How to read the `SmudgeDocs`
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
names of the boxes which are *`native`* (sometimes the interpreter
could show box names like `std.lang!`, etc..) [note: the `SSL` is all native].

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
**`EoW`** (state of Error Or Wrong paramater passed) is a term used by the `SmudgeDocs`
to denote the situation when one or more **prerequisites** of a function are
**violated**. Most of the functions of the SSL return `null`
when an EoW occurs (e.g. function `get()` in `String`, when
the index given is greater than the string's size).

### Congratulations
Now you're **ready** to surf the `SmudgeDocs`! Enjoy it!

|||
|--:|:---:|:--|
| | [Home](https://rimuz.github.io/smudge/) | |
