# The Smudge Programming Language - Documentation of box `std.lang`
The box `std.lang` is imported by default, because it contains all the methods
of strings, lists and tuples.

# Class `String`
Class `String` provides all the methods invokable from a string object.
This class is very special, because it's the **only one** of the SSL which
is **not completely** a Smudge class, because it is an independent type.

A string can be instanced by enclosing its content with a couple of quotes
`'` or double quotes `"`.

```js
/*
 * These are all strings:
*/

"string"
"Hello, world!"
'hel" lo" world'
"writin'"
```


## `String` and **UTF-8**
Smudge's strings fully support **UTF-8** with some
specialized functions (when we treat them, I'll tell you,
but typically they start with `u_`). However, to use them
effectively, you should understand how does UTF-8
work: below is a simple explanation.

### ASCII and UTF-8
As you know, ASCII uses only **7 bits** to represent all its characters.
All its implementations, although, use a whole **byte** for each character.
**UTF-8** was born to extend the character set of **ASCII**, being, at the
same way, fully compatible with it.

In fact, all ASCII strings **are**
automatically UTF-8 strings because all of the ASCII
characters **exist** in
UTF-8. But how are [128,172](http://www.unicode.org/versions/Unicode9.0.0/)
characters represented in a single byte? The answer is: they are **not** stored
in a single byte! Each UTF-8 character is contained in 1, 2, 3 or even 4
**bytes**
following the rule:

| N. of bytes per character | Value in binary |
|:-----------------------------:|:----------------|
|  1 | 0xxxxxxx |
|  2 | 110xxxxx 10xxxxxx |
|  3 | 1110xxxx 10xxxxxx 10xxxxxx |
|  4 | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx |

Where `x`s are replaced with the Unicode's codepoint of the characters.
This is why Smudge has two different methods to get the length of a string:
 - `len()` which will return the **size** of the string expressed in **bytes**
    (faster, **O(1)**).
 - `count()` which will return the **length** of the string expressed
    in **characters** (slower, **O(N)**). This is an example of one of those
    *charming* functions I've previously talk about.

You should use `len()` only when working with ASCII strings or you need to
obtain its real memory size (in bytes).

### Why UTF-8? And why not UTF-16, UTF-32, UCS-2 or other encondings?
I've chosen UTF-8 because of its total compatibility with ASCII and because
it's the cheapest in terms of memory: as I said, strings that use **only** or
**mostly** ASCII will be up to twice longer in UTF-16 (and up to four times
longer in UTF-32). Also:
- Java and .NET use UTF-16
- Golang and Python use UTF-8

## Escape sequences
Some characters **can't be just typed** between the quotes (such as
`new line`, `carriage return`, `tab`, `any control character`, etc.),
so you can enter any character via special sequences called **escape
sequences**.

| Escape sequences | Value | Description |
|:----------------:|:-----:|:-----------:|
| `\'` | 0x27 | Single quote |
| `\"` | 0x22 | Double quote |
| `\\` | 0x5C | Backslash |
| `\a` | 0x07 | Bell |
| `\b` | 0x88 | Backspace |
| `\f` | 0x0C | Form feed - New page |
| `\n` | 0x0A | Form feed - New line |
| `\r` | 0x0D | Carriage return |
| `\t` | 0x09 | Horizontal tab |
| `\v` | 0x0B | Vertical tab |
| `\NNN` | value NNN | Arbitrary octal value (only **ASCII**) |
| `\xNN` | value NN | Arbitrary hexadecimal value (only **ASCII**) |
| `\uNNNN` | codepoint U+NNNN | Arbitrary hexadecimal 32-bit Unicode codepoint (**UTF-8**) |
| `\UNNNNNNNN` | codepoint U+NNNNNNNN | Arbitrary hexadecimal 64-bit Unicode codepoint  (**UTF-8**) |

## Methods `new ()` and `delete ()`
Do nothing.
**Return** `null`.

## Method `idx (i)`
Calculates the index in **bytes** from the given index in **characters** `i`.
If `i` is negative, the characters will be counted from the end.
Returns the **value** of the index resulting or **null** if it cannot be
calculated (e.g. in case of **out of index**).

## Method `len ()`
Returns the **size** of the sring expressed in **bytes**.

## Method `count ([start = 0[, end = len()]])`
Returns the **length** (expressed in **characters**) of the range of the
string from index `start` (included) to index `end` (excluded).
`start` and `end` are expressed in **bytes** and can be negative: in that case
they will be counted from the end.

## Method `empty ()`
Returns true if the string is empty, false otherwise.

## Method `compare (str[, ignore_case = false])`
Compares this string to the given string `str`.
Returns an integer which will be:
- **< 0** if **this < str**
- **0** if **this == str**
- **1** if **`str`** is not a string.
- **> 0** if **this > str**

Also, the comparision between strings can be done ignoring
cases: just pass **`ignore_case`** as **`true`**!

## Method `u_compare (str[, ignore_case = false])`
Compares this string to the given string `str`, like
function **`compare()`** does, but supporting all the
**UTF-8** characters (when involving **cases**).

## Method `== (str)`
Compares this string to the given string `str`.
Returns `true` if they are equal, `false` otherwise.

## Method `!= (str)`
Compares this string to the given string `str`.
Returns `true` if they are different, `false` otherwise.

## Methods `< (str)`, `<= (str)`, `> (str)`, `>= (str)`
Compare this string to the given string `str`.
**Return** `true` if the string is respectively
less than, less or equal to, greater than or greater or equal
to `str`.

## Method `+ (str)`
Returns a new string created by concatenating the string with the given
string `str`. If `str` is not a string, will be converted by calling its method
`to_string()`.

## Method `- (n_chs)`
Returns a new string **without** the **last** *n* **bytes** specified by
the given integer `n_chs` or `null` if `c_chs` is not an integer or is greater
than `len()`.

## Method `* (times)`
Returns a new string whose content will be `times` reps of the string or
`null` if `times` is not an integer. If `times` is negative, the string will
be reversed. Thus, to simply reverse a string you need only to multiply for  `-1`.

## Method `get (idx)`
Returns a **string** containing the ASCII **character**
located at the given index `idx` (expressed in **bytes**) or
**`null`** in case of **out of index** (or id `idx` is not an integer).
If `idx` is negative, it will be counted from the end of the
string.

## Method `u_get (n_ch)`
Returns a **string** containing the UTF-8 **character**
at the given index `n_ch` (expressed in **characters**) or
**`null`** in case of **out of index** (or id `n_ch` is not an integer).
If `n_ch` is negative, it will be counted from the end of the string.

## Method `getc (idx)`
Returns an **integer** containing the **code** of the ASCII character
located at the given index `idx` (expressed in **bytes**) or
**`null`** in case of **out of index** (or id `idx` is not an integer).
If `idx` is negative, it will be counted from the end of the string.

## Method `u_getc (n_ch)`
Returns an **integer** containing the  **UNICODE codepoint** of the UTF-8
**character** located at the given index `n_ch` (expressed in **bytes**) or
**`null`** in case of **out of index** (or id `n_ch` is not an integer).
If `n_ch` is negative, it will be counted from the end of the string.

## Method `contains (str)`
Returns `true` if `str`'s content is contained into
the string, `false` otherwise.

## Method `bytes ()`
Returns a `List` containing **all** the **numeric** values
of each byte of the string.

## Method `chars ()`
Returns a `List` of strings cotaining each an **ASCII/UTF-8 character**
of the origin string.

## Method `join (list)`
Concatenates all the string equivalent of `list`'s elements (got via a call to
elements' `to_string()` method) with the string as separator. `list` can be
either a `List` or a `Tuple`.
Returns the string or **`null`** if `list` is neither a `List` nor a
`Tuple`.

## Method `ends_with (str)`
Returns `true` if the string **ends with** the given string `str`, `false`
otherwise (or `null` if `str` is not a string).

## Method `starts_with (str)`
Returns `true` if the string **starts with** the given string `str`,
`false` otherwise (or `null` if `str` is not a string).

## Method `hash ()`
Returns the hash value of the string.

## Method `find (str)`
Returns the index location of the given string `str` in the string,
returning `len()` if `str` is not found or `null` if `str` is not a string.

## Method `find_last (str)`
**Same as `find (str)`**, but only the last occurrence of `str` is taken into
account.

## Method `substr (start[, end = len()])`
Returns a copy of the string from index `start` (included) to index `end`
(excluded) or `null` if `start` > `end` or if they are not integers.
Just as previous methods, indexes `start` and `end` can be negative: in that
case they will be counted from the end of the string.

## Method `u_substr (u_start[, u_end = count()])`
Same as `substr()`, but uses indexes expressed in **characters** instead of
**bytes**.

## Method `replace ([to_replace = ''[, replacement = '']])`
Returns a new string where all the occurrences of string `to_replace` are
replaced with string `replacement` or `null` if `replacement` is not a string.
If `replacement` is an empty string, every occurrences will be simply removed
from the string, while if `to_replace` is not a string or is an empty string,
the returned string won't be different to the original.

## Method `replace_first ([to_replace = ''[, replacement = '']])`
Same as `replace()`, but only the first occurrence is replaced.

## Method `split (separators[, skipEmpty = true])`
Splits the string into tokens separated by **any** of the string
`separators`'s **bytes**. If `skipEmpty` is set to `false`, are valid tokens
empty ones, too. Returns a `List` containing the created tokens or
`null` if `separators` is not a string.

## Method `trim ()`
Returns a copy of the string without **spacing characters** (all
ASCII characters <= ' ', such as **control characters**, **spaces**, **tabs**,
**newlines**, etc.) at the begin end at the end of the string.

## Method `upper ()`
Returns a copy of the string with all the **ASCII characters** in their
**uppercase** version.

## Method `lower ()`
Returns a copy of the string with all the **ASCII characters** in their
**lowercase** version.

## Method `u_upper ()`
Returns a copy of the string with all the **UTF-8 characters** in their
**uppercase** version.

## Method `u_lower ()`
Returns a copy of the string with all the **UTF-8 characters** in their
**lowercase** version.

## Method `clone ()`
Returns an exact copy of the string, stored in a distinct address of the
memory.

## Method `to_string ()`
Returns the string.

## Method `iterate ()`
Returns an instance of `StringIterator` pointing to the start of the string.

# Class `List`
This is the class which provides all the Smudge lists' functionalities.
A `List` can be instanced simply with **a couple of brackets** containing
the elements separed by `,`. Lists' elements could be of **any type**.
```js
/* A list could contain integers, strings and lists as well */
var my_list = [0, 1, "Hello!", [0, "World!"], xyz];
```

## Methods `new ()` and `delete ()`
Create and destroy the list, shouldn't be called manually. **Return** `null`.

## Method `[] (idx)`
Returns a **reference** to the element located ad the given index `idx`,
or `null` if `idx` is not an integer or is **out of index**.
if `idx` is negative, it will be counted from the end.

## Method `+ (lst)`
Concatenates the list with the given list or tuple `lst`.
Returns the new list or `null` if `lst` is neither a list nor a tuple.

## Method `- (n)`
Returns the list without the last `n` elements or `null` if `n` is
not an integer or negative.

## Method `* (times)`
Returns a new list whose content will be `times` reps of the list or
`null` if `times` is not an integer. If `times` is negative, the list will
be reversed. Thus, to simply reverse a list you need only to multiply for `-1`.

## Method `| (lst)`
Returns a new list containing all the elements from this and `lst` lists,
repeating the objects in common only **once** (works well with lists containing
**unique** elements) or `null` if `lst` is neither a list nor a tuple.

## Method `& (lst)`
Returns a list containing **only** the objects **in common** between this
list and `lst` (works well if each list has only unique elements) or `null`
if `lst` is neither a list nor a tuple.

## Method `== (lst)`
Returns `true` if `lst` is a list and its content is equivalent to this
list's, `false` otherwise.

## Method `!= (lst)`
Returns `false` if `lst` is a list and its content is equivalent to this
list's, `true` otherwise.

## Method `get (idx)`
Returns the element located at the given index `idx` or `null` if `idx` is
not an integer or if out of range. `idx` can be negative, in that case it will be
counted from the end.

## Method `reserve (size)`
Reserves enough space in memory for the list to contain `size` elements.
If `size` is not an integer or is negative, it does nothing.
Returns `null`.

## Method `resize (size)`
Resizes the list to size `size`, objects eventually added will be `null`s.
If `size` is not an integer or is negative, it does nothing.
Returns `null`.

## Method `pop ()`
Removes the last element to the list or does nothing if the list is empty.
Returns `null`.

## Method `push (obj)`
Adds object `obj` to the end of the list.
Returns `null`.

## Method `pop_front ()`
Removes the first element to the list or does nothing if the list is empty.
Returns `null`.

## Method `push_front (obj)`
Adds object `obj` at the beginning of the list.
Returns `null`.

## Method `clone ([start = 0[, end = size()]])`
Returns a list containing a copy of the objects from index `start`
(included) to index `end` (excluded) or null if either `start` or `end`
is not an integer or an out of range one. As always, indexes can be negative,
in that case they will be counted from the end of the list.

## Method `tuple ([start = 0[, end = size()]])`
Same as `clone()`, but **returns** a `Tuple` instance instead of a list.

## Method `append (lst)`
Added the content of the list `lst` to the end of this list or does nothing
if `lst` is not a list.
Returns `null`.

## Method `insert (idx, obj)`
Inserts the object `obj` at the given index `idx`.
`idx` can be negative, in that case it will be counted from the end.
Does nothing if `idx` is not an integer or if it's out of range.
Returns `null`.

## Method `insert_list (idx, lst)`
Same as `insert()`, but inserts the whole content of the list or tuple given
`lst`. Does nothing if `lst` is neither a tuple nor a list.

## Method `erase ([start = 0[, end = start+1]])`
Removes elements from index `start` (included) to index `end` (excluded)
from the list. Since the `end`'s default value is `start`'s plus one,
when **only `start`** index is specified, **only** the object at that index
will be removed. `start` and `end` can be negative, in that case they will be
counted from the end. Does nothing when either `start` or `end` is not an
integer or the range given is broken (e.g. start < end). 'Returns `null`.

## Method `copy_list (idx, lst)`
Copies the content of list or tuple `lst` in the list starting from index idx
following the table:

| Condition | What happens |
|:---------:|:-------------|
| `idx` is not an integer or `lst` is not a list or a tuple | Nothing, returns `false`. |
| `idx` >= size() | Nothing, returns `false`. |
| `idx` < size() and `idx + lst.size()` <= `size()` | Copies `lst`'s content in this list, starting at given index. Returns `true` |
| `idx` < size() but `idx + lst.size()` > `size()` | Same as above, but resizes this list in order to contain the whole `lst' content. Returns `true`. |

## Method `find (obj)`
Searches an equivalent object to `obj` (checked via `operator== ()`).
Returns the index of the first occurence or `null` if not found.

## Method `count (obj)`
Counts how many equivalent objects to `obj` (checked via `operator== ()`).
Returns integer which value is the number counted.

## Method `slice ([start = 0[, end = size()]])`
Alias for `clone()`.

## Method `reverse ()`
Reverses the list in place.
Returns `null`.

## Method `sort ([reversed = false])`
Sorts the list in place in ascending order if `reversed` is `false` or
descending order if `reversed` is `true`. The elements of the list are
compared via the operator `<`, which can be overloaded. Returns `null`.

## Method `unique ()`
Eliminates all but the first element from every consecutive group of
equivalent elements from the list. Returns `null`.

## Method `to_string ()`
Returns a string representation of the list (typically elements'
`to_string()` values separated by a comma inside a couple of brackets).

## Method `empty ()`
Returns `true` if the list is empty (has no elements), `false` otherwise.

## Method `size ()`
Returns the number of elements contained by the list.

## Method `iterate ()`
Returns an instance of `ListIterator` pointing to the start of the list.

# Class `Tuple`
Tuples are immutable lists. You can instance a `Tuple` by enclosing its
elements with a couple of round brackets (i.e. `()`).
```js
/* Tuples are just like lists, but are immutable. */
var my_tuple = (1, 2, "Hello", ("World", [0, 1, "!"], ()), []);
```

## Methods `new ()` and `delete ()`
Create and destroy the tuple, shouldn't be called manually. **Return** `null`.

## Method `+ (lst)`
Returns a new tuple created by concatenating the tuple to list or tuple
`lst` or `null` if `lst` is neither a list nor a tuple.

## Method `- (n)`
Returns a new tuple without the last `n` elements of the tuple or `null`
if `n` is not an integer or negative.

## Method `* (times)`
Returns a new tuple whose content will be `times` reps of the tuple or
`null` if `times` is not an integer. If `times` is negative, the tuple will
be reversed. Thus, to simply reverse a tuple you need only to multiply for `-1`.

## Method `| (tup)`
Returns a new tuple containing all the elements from this and `tup` tuples,
repeating the objects in common only **once** (works well with tuples containing
**unique** elements) or `null` if `tup` is neither a list nor a tuple.

## Method `& (tup)`
Returns a tuple containing **only** the objects **in common** between this
tuple and `tup` (works well if each tuple has only unique elements) or `null`
if `tup` is neither a list nor a tuple.

## Method `== (lst)`
Returns `true` if `lst` is a tuple and its content is equivalent to this
tuple's, `false` otherwise.

## Method `!= (lst)`
Returns `false` if `lst` is a tuple and its content is equivalent to this
tuple's, `true` otherwise.

## Method `slice ([start = 0[, end = size()]])`
Returns a tuple containing a copy of the objects from index `start`
(included) to index `end` (excluded) or `null` if either `start` or `end`
is not an integer or an out of range one. As always, indexes can be negative,
in that case they will be counted from the end of the list.

## Method `get (idx)`
Returns the element located at the given index `idx` or `null` if `idx` is
not an integer or if out of range. `idx` can be negative, in that case it will be
counted from the end.

## Method `list ([start = 0[, end = size()]])`
Same as `slice()`, but returns a list.

## Method `hash ()`
Returns the hash value of the tuple.

## Method `empty ()`
Returns `true` if the tuple is empty (has no elements), `false` otherwise.

## Method `clone ([start = 0[, end = size()]])`
Returns a tuple containing a copy of the objects from index `start`
(included) to index `end` (excluded) or null if either `start` or `end`
is not an integer or an out of range one. As always, indexes can be negative,
in that case they will be counted from the end of the list.

## Method `to_string ()`
Returns a string representation of the tuple, very similar to tuple's with
round brackets instead of brackets.

## Method `iterate ()`
Returns an instance of `ListIterator` pointing to the start of the tuple.

# Class `StringIterator`
An Iterator class for `String`.

## Method `new (str)`
Initializes a new `StringIterator` starting from the begin of String `str`.
Returns `null`.

## Method `delete ()`
Destroy the `StringIterator` instance.
Returns `null`.

## Method `next ()`
Advance the iterator.
Returns a tuple containing the current item and an integer (1 if it's a valid item, 0 otherwise).

# Class `ListIterator`
Same as `StringIterator`, but for `List`s or `Tuple`s.

## Methods `new (lst)`, `delete ()`, `next ()`
See [StringIterator](stdlang.md#class-stringiterator).

||
|:---:|
| [Home](https://smudgelang.github.io/smudge/) |
