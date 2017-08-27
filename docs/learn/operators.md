# The Smudge Programming Language - Operators
Smudge supports all of the common operators for **C-like** programming languages (see [more](http://www.cplusplus.com/doc/tutorial/operators/).
They can be grouped in:
- **Syntactical Operators**: used to follow the language syntax (such as `(` and `)`, `.` (dot), `,` (comma), etc).
- **Math Operators**: used to compute mathematical (and logical bit-by-bit or bit-shift) operations (such as `+`, `/`, `|`, `%`, etc.)
- **Logical Operators**: used to compute the result of logical (boolean) expressions (`||` and `&&`)
- **Compare Operators**: used to compare the values of two objects (such as `<`, `>`, `==`, etc.)
- **Assign Operators**: used to set the value of an object (such as `=`, `+=`, `*=`, `<<=`, etc.)
- **Prefix Operators**: used to change value of a part of an expression (such as `unary-`, `~`, `!`, `pre++`, `pre--`, etc.).
- **Postfix Operators**: used to change value of an object inside an expression (`post++` and `post--`)

### Uncommon operators
There are **three** more operators that you can rarely find in other programming languages:

- **Null-checker operator**
    <br> Usage: `null(xxx)`
    <br> Checks wheter expression `xxx` is `null`
- **Elvis operator**
    <br> Usage: `xxx ?: yyy`
    <br> If `xxx` is `null`, its value will be rejected and `yyy` will be used instead.
- **Reference operator**
    <br> Usage: `ref(xxx)`
    <br> Gets a reference from an lvalue expression `xxx`.

Note: don't mix up the elvis and the **conditional operator**!
This one has another syntax:

```js
    /*
     * Will result 'expr1' if the `codition` is
     * verified, `expr2` otherwise.
    */
    condition ? expr1 : expr2
```

### Operators Priority
Another important fact about **operators** is their **priority** (or precedence):

While in some cases it's very easy to understand an expression, like in this example:
```js
/*
 * 1) computes 2 * y
 * 2) divides by 5
 * 3) adds x
*/

val = x + 2 * y / 5;
```

Sometimes it could be more complicated (and could cause some bugs):
```js
/*
 * Wat?
*/
val = x && 5 == a || b ? x : y || z % 2 != 0;
```

So we can fix this by:
- **Using parenthesis** (but don't exaggerate!).
- Learning (a bit) operators priorities.

### Operators table

Here's the **priority table** (where **priority 0** is the **highest**):

| Operator | Type | Priority | Overload? | Described |
|:--:|:--:|:--:|:--:|:--:|:--|
| `.` | Syn | 0 | No | Find inside object |
| `{}` | Syn | 0 | No | Group some lines of code together |
| `[]` | Syn | 0 | Yes: `[]` | Array subscripting |
| `()` | Syn | 0 | Yes: `()` | Function call |
| `[]` | Syn | 0 | No | Create list |
| `()` | Syn | 0 | No | Create tuple |
| `,` | Syn | 0 | No | Split parts of an expression |
| `++` | Post | 0 | Yes: `+` | Postfix increment |
| `--` | Post | 0 | Yes: `-` | Postfix decrement |
| `++` | Pre | 1 | Yes: `+` | Prefix increment |
| `--` | Pre | 1 | Yes: `-` | Prefix decrement |
| `+` | Pre | 1 | Yes: `+` | Unary plus |
| `-` | Pre | 1 | Yes: `-` | Unary minus |
| `~` | Pre | 1 | Yes: `~` | Complement operator |
| `!` | Pre | 1 | Yes: `!` | NOT operator |
| `*` | Math | 2 | Yes: `*` | Multiply |
| `/` | Math | 2 | Yes: `/` | Divide |
| `%` | Math | 2 | Yes: `%` | Modulo |
| `+` | Math | 3 | Yes: `+` | Addition |
| `-` | Math | 3 | Yes: `-` | Subtraction |
| `<<` | Math | 4 | Yes: `<<` | Left bit-shift |
| `>>` | Math | 4 | Yes: `>>` | Right bit-shift |
| `>` | Cmp | 5 | Yes: `>` | Greater |
| `>=` | Cmp | 5 | Yes: `>=` | Greater or equal |
| `<` | Cmp | 5 | Yes: `<` | Less |
| `<=` | Cmp | 5 | Yes: `<=` | Less or equal |
| `==` | Cmp | 6 | Yes: `==` | Equal |
| `!=` | Cmp | 6 | Yes: `!=` | Not equal |
| `&` | Math | 7 | Yes: `&` | AND operator |
| `^` | Math | 8 | Yes: `^` | XOR operator |
| `|` | Math | 9 | Yes: `|` | OR operator |
| `&&` | Log | 10 | Yes: `&&` | Logical AND operator |
| `||` | Log | 11 | Yes: `||` | logical OR operator |
| `?:` | Syn | 12 | No | Elvis operator |
| `?:` | Syn | 12 | No | Ternal conditional operator |
| `=` | Assign | 13 | No | Assign |
| `+=` | Assign | 13 | No | Addition and assign |
| `-=` | Assign | 13 | No | Subtraction and assign |
| `*=` | Assign | 13 | No | Assign |
| `/=` | Assign | 13 | No | Division and assign |
| `%=` | Assign | 13 | No | Modulo assign |
| `&=` | Assign | 13 | No | AND and assign |
| `^=` | Assign | 13 | No | XOR and assign |
| `|=` | Assign | 13 | No | OR and assign |
| `<<=` | Assign | 13 | No | Left bit-shift and assign |
| `>>=` | Assign | 13 | No | Right bit-shift and assign |
| `,` | Syn | 14 | No | Comma |
| `;` | Syn | 15 | No | Semicolon |
