## The Smudge Programming Language - Documentation of box `std.lang`
The box `std.lang` is imported by default, because it contains all the methods of strings, lists and tuples.

## Warning
At the moment I'm still developing this box, so it's incomplete and unstable.
However, all of the classes/functions documented in this page are **alredy**
stable and tested.

## Class `String`
Class `String` provides all the methods invokable from a string object.
This class is very special, becuase it's the only one of the **SSL** which
has **not** a matching **native class** (with native I mean a C++ class).  

### Function `idx([i = 0])`
