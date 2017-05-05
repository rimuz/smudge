## The Smudge Programming Language - Documentation of box `std.lang`
The box `std.lang` is imported by default, because it contains all the methods
of strings, lists and tuples.

## Warning
At the moment I'm still developing this box, so it's incomplete and unstable.
However, all of the classes/functions documented in this page are **alredy**
stable and tested.

## Class `String`
Class `String` provides all the methods invokable from a string object.
This class is very special, because it's the **only one** of the SSL which
has **not** a matching **native class** (with native I mean a C++ class).

## `String` and **UTF-8**
Smudge's strings fully support **UTF-8** with some specialized functions
(when we treat them, I'll tell you, typically starting with `u_`).
However, to use it effectively, you should understand how does UTF-8
work: below is a simple explanation.

#### ASCII and UTF-8
As you know, ASCII uses only **7 bits** to represent all its characters.
All its implementations, although, use a whole **byte** for each character.
**UTF-8** was born to extend the character set of **ASCII** being, at the
same way, fully compatible with it.

In fact, all ASCII strings **are**
automatically UTF-8 strings because all of the ASCII characters **exist** in
UTF-8. But how are [128,172](http://www.unicode.org/versions/Unicode9.0.0/)
characters represented in a single byte? The answer is: they are **not** in
a single byte! Each UTF-8 character is contained in 1, 2, 3 or even 4 **bytes**
following the rule:

| N. of bytes per character | Value in binary |
|:-----------------------------:|:----------------|
|  1 | 0xxxxxxx |
|  2 | 110xxxxx 10xxxxxx |
|  3 | 1110xxxx 10xxxxxx 10xxxxxx |
|  4 | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx |
Where `x` is replaced with the Unicode's codepoint of the characters.
This is why Smudge has two different methods to get the length of a string:
 - `len()` which will return the **size** of the string expressed in **bytes**
    (faster, **O(1)**).
 - `count()` which will return the **length** of the string expressed
    in **characters** (slower, **O(N)**). This is an example of one of those
    *magic* functions I've previously talk about.

You should use `len()` only when working with ASCII strings or you need to
obtain its memory size (in bytes).

#### Why UTF-8? And why now UTF-16, UTF-32, UCS-2 or other encondings?
I've chosen UTF-8 because of its total compatibility with ASCII and because
it's the cheapest in terms of memory: as I said, strings that use **only** or
**mostly** ASCII will be up to twice longer in UTF-16 (and up to four times
longer in UTF-32). Also:
- Java and .NET use UTF-16
- Golang and python use UTF-8

### Function `idx(i)`
Calculates the index in **bytes** from the given index in **characters** `i`.
If `i` is negative, the characters will be counted from the end.
**Returns** the **value** of the index resulting or **null** if it cannot be
calculated (e.g. in case of **out of index**).

### Function `len()`
**Returns** the **size** of the sring expressed in **bytes**.

### Function `count()`
**Returns** the **length** of the string expressed in **characters**.

### Function `empty()`
**Returns** true if the string is empty, false otherwise.

### Function `compare(str)`
