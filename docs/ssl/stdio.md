# The Smudge Programming Language - Documentation of box `std.io`
The box `std.io` allows you to handle console and file I/O.
**Note:** in this page I'll often say `character` **instead of `byte`**, so, unless specified don't think about Unicode (differently from the `std.lang` SmudgeDoc).

## Var `ln`
A string containing a newline character (`\n`).

## Function `print (obj...)`
Prints a text representation of the objects given to the `stdout`.
**Returns** the box object.

## Function `println (obj...)`
Prints a text representation of the objects given to the `stdout` **and** a newline character (`\n`).
**Returns** the box object.

## Function `e_print (obj..)`
Prints a text representation of the objects given to the `stderr`.
**Returns** the box object.

## Function `e_println (obj...)`
Prints a text representation of the object given to the `stderr` **and** a newline character (`\n`).
**Returns** the box object.

## Function `line ()`
Inputs a text line from `stdin` and stores it as a string.
**Returns** the string or `null` if typed `EOF` (e.g. Ctrl+D on Unix).

## Function `int ()`
Inputs an integer from `stdin`.
**Returns** the integer or `null` if an error occurs during the conversion.

## Function `float ()`
Inputs a floating point number from `stdin`.
**Returns** the number or `null` if an error occurs during the conversion.

## Function `get ()`
Reads a character from `stdin`.
**Returns** a string containing its value or `null` if `EOF`
is entered (e.g. Ctrl+D on Unix).

## Function `getc ()`
Reads a character from `stdin`.
**Returns** an integer containing its ASCII value or -1 if  
`EOF` is entered `EOF` (e.g. Ctrl+D on Unix).

## Function `next ()`
Inputs a string from `stdin` ending by **newline** character (`\n`), **tab** character (`\t`) or **spaces**.
**Returns** the string, and never returns `null`.

## Function `<< (obj)`
Alias for **`print (obj)`**.
Like **`print (obj)`**, **Returns** the box object.

## Function `>> (obj)`
Calls function **`next ()`** and sets **`obj`** to its return value. To make the magic work, **`obj`** should be a reference to a valid variable (got through `ref` keyword), otherwise this function won't do anything.
**Returns** the box object.

## Function `open (path[, mode = RW])`
Opens a file stream and creates a **`FileStream`** instance to handle it.
**`path`** is a string representation of the file path, while **`mode`**
is an integer which value corresponds to the stream opening mode.
You can obtain the right value of opening mode by combining the following
values (with `+` or `|` operators):

|  Var name  |  Value (bin) | Value (hex) |  Description                      |
|:----------:|:------------:|:-----------:|:---------------------------------:|
| **`BIN`**  |      1       |       1     | Opens in binary mode.             |
| **`APP`**  |     10       |       2     |  Writes at the end of the file    |
| **`WRITE`**|    100       |       4     |  Opens an **output** stream.      |
| **`READ`** |    1000      |       8     |  Opens an **input** stream.       |
| **`RW`**   | 1100 | C | both **`READ`** and **`WRITE`**, default value.     |
| **`TRUNC`**| 10000 | 10 | Deletes the content of the file before writing    |

Note that:
- if neither **`READ`** nor **`WRITE`** is set, they will be both selected by the interpreter.
- if both **`TRUNC`** and **`APP`** are set, wins **`TRUNC`**
- if neither **`TRUNC`** nor **`APP`** is set and **`WRITE`** is set,
the output will start from the beginning of the file.

**Returns** the instance of **`FileStream`** or `null` if the stream can't be open.

## Function `remove (fileName)`
**Deletes** the file named `fileName`.
**Returns** `false` if the operation failed, `true` otherwise.

## Function `rename (old, new)`
**Renames** file `old` with the new name `new`.
**Returns** `false` if the operation failed, `true` otherwise.

# Class `FileStream`
The implementation of the class is native and consist of an object containing
an `std::fstream` instanced and open with the given arguments, when an
instance of class `FileStream` is deleted, the stream is automatically closed.

## Methods `new ()` and `delete ()`
Implemented natively, will cause errors if called manually.

## Method `open (path[, mode = RW])`
Opens the file given with a certain mode.
See [`std.io::open()`] for more, which is better to use.
**Returns** `false` if the stream couldn't be open, `true` otherwise.

## Method `close ()`
Closes the stream.
**Returns** `null`.

## Method `get ()`
Reads a character from the stream.
**Returns** a string containing the character or `null` if `EOF` is reached.

## Method `getc ()`
Reads a character from the stream.
**Returns** an integer containing its ASCII value or -1 if `EOF` is reached.

## Method `line ()`
Reads a text line from the stream and stores it as a string.
**Returns** the string or `null` if the operation fails.

## Method `peek ()`
Peeks the next character.
**Returns** an integer containing its ASCII value or -1 if `EOF` is reached.

## Method `read (n)`
Reads next **`n`** characters from the stream and stores them as a string.
**Returns** the string or `null` is **`n`** is an invalid parameter.

## Method `read_all ()`
Reads all the text from the stream until it finds `EOF` and stores it in a string.
**Returns** the string. **`read_all()`** never returns `null`.

## Method `write (obj)`
Writes a text representation of the object given to the stream at the current
position.
**Returns** the **`FileStream`** instance.

## Method `count ()`
**Returns** the number of bytes read from the last **input** operation in
an integer.
**Note**: if called after **`peek()`**, **`count()`** will always return 0.

## Method `seek (off[, pos = BEG])`
Sets the position of the next character to be extracted to offset `off`
relatively to `pos` that can have one of the following values:

| Var name | Value (dec/hex) | Description |
|:--------:|:---------------:|:-----------:|
| **`BEG`** | 0 | Beginning of the stream |
| **`CURR`** | 1 | Current position |
| **`END`** | 2 | End of the stream |

Values greater than 2 will be considered as **`END`**.

**Returns** `null`.

## Method `tell ()`
**Returns** the number of the current character in an integer.

## Method `good ()`
Checks if the stream can be read.
**Returns** `true` if neither `EOF` nor errors occured, `false` otherwise.

||
|:---:|
| [Home](https://smudgelang.github.io/smudge/) |
