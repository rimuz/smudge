# The Smudge Programming Language - Types

### Smudge and types
As we've alredy seen, we can store many different kinds of objects
inside a variable, from integers to class instances.

I've alredy previously talked about how objects are treated in Smudge [here](vars.md),
but now we'll linger on them more deeply.

### Integers
Smudge's `integers` are effectively stored _at least_ in **64-bit signed integers**.
**Typically** the range of valid values goes from `-9223372036854775808` to
`+9223372036854775807`.

To check these values on your machine, try executing the following code:

```js
import std.io, std.math;

/*
 * We're using the integer constants MIN_INT and MAX_INT
 * in box std.math.
*/
func main
    io << "minimum value: " << math.MIN_INT << io.ln
       << "maximum value: " << math.MAX_INT << io.ln;
```

Note that you cannot insert the minimum value in the code: that's because the
Smudge parser evaluates the absolute value of the integer before applying its sign.

Integers can be written in different bases following the rule:
- **No prefix**: base-10
- **`0x` prefix**: base-16
- **`0` prefix**: base-8
- **`0b` prefix**: base-2
The sign **always** precede the prefix. As long as there are no ambiguities (such as `--` and `++`, used as pre/post decrement/increment operators) you can type any number of signes you want.

So, these strings are all valid integer **literals**:
```
    -0xFF    0b1010    - -0777    -+-125
```

### Floats
Floating-point values are stored in **double-precision floating point** generally
with 64-bits of size.

There are different formats for representing a float in Smudge:
- `A.B` -> represents value **A.B** (for example `1.2`, `2.5`, `0.33`)
- `.B` -> represents **0.B** (for example `.10`, `.001`)
- `A.` -> represents **A.0** (for example `1.`, `500.`)
- `VALe+X`, where `VAL` is any of the previous examples -> represents
    **VAL · 10<sup>X</sup>** (for example `1.e10`, `2.5e-100`, `.1e+3`) <br>
    Note: it won't work if VAL is an integer (it must contain a point to distunguish
    from them) and `e` must be lowercase.

At the moment there's no way to type an hexadecimal/octal/binary floating-point in Smudge.

### Strings
In Smudge strings are represented with the UTF-8 enconding, so it's a simple
sequence of bytes.
I've talked a lot about strings [here](../ssl/stdlang.md#class-string), so take a look
before continuing to read.

### Class Instances
Now we'll see different types of **class instances**. These are objects derived
from a **class**. The SSL provides several classes for many different uses.

Because the box `std.lang` is imported by default, classes like `String`, `List` or
`Tuple` (with the respective `StringIterator`, `ListIterator` and so on) alredy exist
in memory and are ready to be used. This is why you don't need to import anything to use a string, list or tuple.

### Lists
There are not arrays in Smudge: we use lists instead.
A list is a **structure** which can contain a **variable amount of objects**; it can
**grow and shrink** dynamically during the execution of a program and is automatically
freed by the _garbage collector_.

A list is initialized with the following syntax:

```js
[obj0, obj1, ..., objN] // list with N elements
[] // empty list
lg.List() // empty list, where 'lg' is box 'std.lang'
```

You can see all the methods and operators supported by lists [here](../ssl/stdlang.md#class-list).

### Tuples
Tuples are like lists but immutables. Tuples' syntax is similar to Lists':

```js
(obj0, obj1, ..., objN) // tuples with N elements
lg.Tuple() // empty tuple, where 'lg' is box 'std.lang'
```

**NOTE:** Syntax #1 **doesn't apply** to tuples with **one element**: because expression `(obj)`
could be ambigous, it's evaluated as `obj`.

To create a tuple with an element, you can use any of the following tricks:

```
[obj].tuple()
()+[obj]
```

### Other Classes
The SSL consists of many boxes, most of which define classes ready 'out of the box'.
However, very often we need to create and use our customized classes.
In the next page we'll find out how to define and implement them.

|||
|--:|:---:|:--|
| [Previous](operators.md) | [Home](https://smudgelang.github.io/smudge/) | Next |
