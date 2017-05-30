## The Smudge Programming Language - Documentation of box `std.lang`
The box `std.lang` is imported by default, because it contains all the methods
of strings, lists and tuples.

## Class `String`
Class `String` provides all the methods invokable from a string object.
This class is very special, because it's the **only one** of the SSL which
has **not** a matching **native class** (with native I mean a C++ class).

## `String` and **UTF-8**
Smudge's strings fully support **UTF-8** with some
specialized functions (when we treat them, I'll tell you,
but typically they start with `u_`). However, to use it
effectively, you should understand how does UTF-8
work: below is a simple explanation.

#### ASCII and UTF-8
As you know, ASCII uses only **7 bits** to represent all its
characters.
All its implementations, although, use a whole **byte** for
each character.
**UTF-8** was born to extend the character set of **ASCII**
being, at the
same way, fully compatible with it.

In fact, all ASCII strings **are**
automatically UTF-8 strings because all of the ASCII
characters **exist** in
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

#### Why UTF-8? And why not UTF-16, UTF-32, UCS-2 or other encondings?
I've chosen UTF-8 because of its total compatibility with ASCII and because
it's the cheapest in terms of memory: as I said, strings that use **only** or
**mostly** ASCII will be up to twice longer in UTF-16 (and up to four times
longer in UTF-32). Also:
- Java and .NET use UTF-16
- Golang and Python use UTF-8s

### Method `idx (i)`
Calculates the index in **bytes** from the given index in **characters** `i`.
If `i` is negative, the characters will be counted from the end.
**Returns** the **value** of the index resulting or **null** if it cannot be
calculated (e.g. in case of **out of index**).

### Method `len ()`
**Returns** the **size** of the sring expressed in **bytes**.

### Method `count ([start = 0[, end = len()]])`
**Returns** the **length** (expressed in **characters**) of the range of the
string from index `start` (included) to index `end` (excluded).
`start` and `end` are expressed in **bytes** and can be negative: in that case
they will be counted from the end.

### Method `empty ()`
**Returns** true if the string is empty, false otherwise.

### Method `compare (str[, ignore_case = false])`
Compares this string to the given string `str`.
**Returns** an integer which will be:
- **< 0** if **this < str**
- **0** if **this == str**
- **1** if **`str`** is not a string.
- **> 0** if **this > str**

Also, the comparision between strings can be done ignoring
cases: just pass **`ignore_case`** as **`true`**!

### Method `u_compare (str[, ignore_case = false])`
Compares this string to the given string `str`, like
function **`compare()`** does, but supporting all the
**UTF-8** characters (when involving **cases**).

### Method `== (str)`
Compares this string to the given string `str`.
**Returns** `true` if they are equal, `false` otherwise.

### Method `!= (str)`
Compares this string to the given string `str`.
**Returns** `true` if they are different, `false` otherwise.

### Methods `< (str)`, `<= (str)`, `> (str)`, `>= (str)`
Compare this string to the given string `str`.
**Return** `true` if the string is respectively
less than, less or equal to, greater than or greater or equal
to `str`.

### Method `+ (str)`
**Returns** a new string created by concatenating the string with the given
string `str`. If `str` is not a string, will be converted by calling its method
`to_string()`.

### Method `- (n_chs)`
**Returns** a new string **without** the **last** *n* **bytes** specified by
the given integer `n_chs` or `null` if `c_chs` is not an integer or is greater
than `len()`.

### Method `* (times)`
**Returns** a new string whose content will be `times` reps of the string or
`null` if `times` is not an integer. If `times` is negative, the string will
be reversed. Thus, to simply reverse a string you need only to multiply for  
`-1`.

### Method `get (idx)`
**Returns** a **string** containing the ASCII **character**
located at the given index `idx` (expressed in **bytes**) or
**`null`** in case of **out of index** (or id `idx` is not an integer).
If `idx` is negative, it will be counted from the end of the
string.

### Method `u_get (n_ch)`
**Returns** a **string** containing the UTF-8 **character**
at the given index `n_ch` (expressed in **characters**) or
**`null`** in case of **out of index** (or id `n_ch` is not an integer).
If `n_ch` is negative, it will be counted from the end of the string.

### Method `getc (idx)`
**Returns** an **integer** containing the **code** of the ASCII character
located at the given index `idx` (expressed in **bytes**) or
**`null`** in case of **out of index** (or id `idx` is not an integer).
If `idx` is negative, it will be counted from the end of the string.

### Method `u_getc (n_ch)`
**Returns** an **integer** containing the  **UNICODE codepoint** of the UTF-8
**character** located at the given index `n_ch` (expressed in **bytes**) or
**`null`** in case of **out of index** (or id `n_ch` is not an integer).
If `n_ch` is negative, it will be counted from the end of the string.

### Method `contains (str)`
**Returns** `true` if `str`'s content is contained into
the string, `false` otherwise.

### Method `bytes ()`
**Returns** a `List` containing **all** the **numeric** values
of each byte of the string.

### Method `chars ()`
**Returns** a `List` of strings cotaining each an **ASCII/UTF-8 character**
of the origin string.

### Method `join (list)`
Concatenates all the string equivalent of `list`'s elements (got via a call to
elements' `to_string()` method) with the string as separator. `list` can be
either a `List` or a `Tuple`.
**Returns** the string, or **`null`** if `list` is neither a `List` nor a
`Tuple`.

### Method `ends_with (str)`
**Returns** `true` if the string **ends with** the given string `str`, `false`
otherwise (or `null` if `str` is not a string).

### Method `starts_with (str)`
**Returns** `true` if the string **starts with** the given string `str`,
`false` otherwise (or `null` if `str` is not a string).

### Method `hash ()`
**Returns** the hash value of the string.

### Method `find (str)`
**Returns** the index location of the given string `str` in the string,
returning `len()` if `str` is not found or `null` if `str` is not a string.

### Method `find_last (str)`
**Same as `find (str)`**, but only the last occurrence of `str` is taken into
account.

### Method `substr (start[, end = len()])`
**Returns** a copy of the string from index `start` (included) to index `end`
(excluded) or `null` if `start` > `end` or if they are not integers.
Just as previous methods, indexes `start` and `end` can be negative: in that
case they will be counted from the end of the string.

### Method `u_substr (u_start[, u_end = count()])`
Same as `substr()`, but uses indexes expressed in **characters** instead of
**bytes**.

### Method `replace ([to_replace = ''[, replacement = '']])`
**Returns** a new string where all the occurrences of string `to_replace` are
replaced with string `replacement` or `null` if `replacement` is not a string.
If `replacement` is an empty string, every occurrences will be simply removed
from the string, while if `to_replace` is not a string or is an empty string,
the returned string won't be different to the original.

### Method `replace_first ([to_replace = ''[, replacement = '']])`
Same as `replace()`, but only the first occurrence is replaced.

### Method `split (separators[, skipEmpty = true])`
Splits the string into tokens separated by **any** of the string
`separators`'s **bytes**. If `skipEmpty` is set to `false`, are valid tokens
empty ones, too. **Returns** a `List` containing the created tokens, or
`null` if `separators` is not a string.

### Method `trim ()`
**Returns** a copy of the string without **spacing characters** (all
ASCII characters <= ' ', such as **control characters**, **spaces**, **tabs**,
**newlines**, etc.) at the begin end at the end of the string.

### Method `upper ()`
**Returns** a copy of the string with all the **ASCII characters** in their
**uppercase** version.

### Method `lower ()`
**Returns** a copy of the string with all the **ASCII characters** in their
**lowercase** version.

### Method `u_upper ()`
**Returns** a copy of the string with all the **UTF-8 characters** in their
**uppercase** version.

### Method `u_lower ()`
**Returns** a copy of the string with all the **UTF-8 characters** in their
**lowercase** version.

### Method `clone ()`
**Returns** an exact copy of the string, stored in a distinct address of the
memory.

### Method `to_string ()`
**Returns** the string.
