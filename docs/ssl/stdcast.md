## The Smudge Programming Language - box `std.cast`
The box `std.cast` is an utility to convert into and
check object types.

### Function `int (x)`
Casts the object `x` to an `integer`.
It does work only if `x` is an **`integer`**, **`float`** or a **`string`**.
**Returns** the `integer` or `null` if `x` is of an incompatible type.

### Function `float (x)`
Casts the object `x` to a `float`.
It does work only if `x` is an **`integer`**, **`float`** or a **`string`**.
**Returns** the `float` or `null` if `x` is of an incompatible type.

### Function `string (obj)`
Converts object `obj` to a `string`.
If `obj` is an instance, it **has to** have a method named **`to_string()`**.
**Returns** the `string`.

### Function `is_int (x)`
**Returns** `true` if `x` is an `integer`, `false` otherwise.

### Function `is_float (x)`
**Returns** `true` if `x` is a `float`, `false` otherwise.

### Function `is_string (x)`
**Returns** `true` if `x` is a `string`,
`false` otherwise.

### Function `uchar (cp)`
**Returns** a string containing the character represented by the **Unicode codepoint** `cp` or `null` if `cp` is not a valid codepoint.

### Function `ucode (str)`
**Returns** the **Unicode codepoint** of the first character in the string `str` or `null` if `str` is not a valid string.

### Function `value (x)`
**Returns** `x`. If `x` is a **reference**, **returns** the object pointed by `x`. 

### Function `typeof (obj)`
**Returns** a string representing the type of the object `obj` following the rules:

| Object Type | Output String |
|:--:|:--:|
| **Null** | `n` |
| **Integer** | `i` |
| **Float** | `f` |
| **String** | `s` |
| **Class instance** | `o` |
| **Enum** | `e` |
| **Class** | `c` |
| **Function** | `F` |
| **Method** | `m` |
| **Box** | `b` |
| **Reference** | `r` + type |
| **Instance Creator** | `I` |
| **Native Data** | `d` |
| Other | `u` |

### Function `classof (obj)`
**Returns** the `class` of the object `obj` or `null` if `obj` is neither a class instance nor a string.

### Function `baseof (c[, n = 0])`
**Returns** the `n`th super class of the class `c` or `null` if `c` is not a class or has not that super class.

### Function `same (a, b)`
**Returns** `true` if `a` and `b` are the same object, `false` otherwise.

### Function `kin (c1, c2)`
**Returns** `true` if class `c2` is **derived** from or the **same** as class `c1`. `c1` and `c2` can be also class instances, in that case their base classes will be used instead (see `classof()`).
