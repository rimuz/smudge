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
 * 4) stores the result in val
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
- Learning (a bit) operators priorities (see this page footage).

### Operator overloading
Unlike Java, Smudge **does support** operator overloading, so you can define
the behavoir of a **box** or **class instance** when an overloadable operator
is called on it. And it's so _easy_!

Note: **Classes** is the topic of the next page, so to _fully_ understand this
paragraph, you can treat them before and then return here.

#### `operator +`
A plus sign can be used in different kinds of expressions:
1. To perform an **addition**: `a + b`
2. To perform a **prefix increment**: `++a`
3. To perform a **suffix increment**: `a++`
4. To perform an **addition assignment**: `a += b`
5. As **unary** operator: `+a`

Anyway, cases (2), (3) and (4) are automatically translated to case (1) in the following way:

```js
++a       ->     (a = a + 1)
a++       ->     [a, a = a + 1].get(0) // Not really
a += b    ->     a = a + b
```

So, there are only two different behaviors of `operator +`:
1. To perform an **addition**: `a + b`
2. As **unary** operator: `+a`

Case (2) is sometimes used when you need to convert an object to an integer,
though [`std.cast::int(x)`](../ssl/stdcast.md#function-int-x\) is recommended.

Here's an example of overloading:

```js
import std.io, std.cast;

/*
 * WrInt is an integer wrapper.
*/
class WrInt {
    var val;
    func new(v = 0) val = v;

    func +(x, is_unary){
        // unary+ overload (#1)
        if(is_unary)
            return val;

        // addition overload (#2)
        return WrInt(val + x);
    }

    func to_string // WrInt can be converted to string
        return cast.string(val);
}

func main {
    var obj = WrInt(5),
        obj2 = obj + 100,       // calling #2
        obj3 = obj2 + +obj,    // calling both #1 and #2
        obj4 = obj3++,          // calling #2
        obj5 = +obj2,           // calling #1
        obj6 = +obj3;           // calling #1

    /*
     * Note: in the following line
     * operator+ is called on the string,
     * not on an instance of WrInt
    */
    io.println(
        "obj = "  + obj  + "\n"
        "obj2 = " + obj2 + "\n"
        "obj3 = " + obj3 + "\n"
        "obj4 = " + obj4 + "\n"
        "obj5 = " + obj5 + "\n"
        "obj6 = " + obj6
    );
}

```

OUTPUT, as expected:

```
obj = 0
obj2 = 100
obj3 = -100
obj4 = 100
obj5 = -100
```

Some notes about the println call:
- `\n` is an [escape sequence](../ssl/stdlang.md#escape-sequences).
- the `+` between strings `"\n"` and `"objN = "` is not necessary
    because Smudge tokenizer automatically concatenates near string literals.

As we've seen, we can distinguish an `operator+` call from an `operator unary+`
one thanks to a second parameter `is_unary`:
- When an **addition** is performed, the **first parameter** is the **second addend**,
while `is_unary` is set to **`null`**.
- When an **unary+ operation** is performed, the **first parameter** is set to **`null`**, while `is_unary` is set to **`true`** (integer `1`).

#### Smarter operation overloading
The above implementation is just **too simply** to be used in real applications,
because it doesn't support additions like **`WrInt + WrInt`** and `WrInt + not integer`
will print **meaningless error messages**.

To fix these issues we can use [`std.cast::kin()`](../ssl/stdcast.md#function-kin-c1-c2)
combined with [`std.system::sterr()`](../ssl/stdsystem.md#function-sterr-str) and
[`std.cast::desc()`](../ssl/stdcast.md#function-sterr-str) to check the operand type:

```js
import std.io, std.cast, std.system;
// ...
    func +(x, is_unary){
        if(is_unary)
            return val;

        if(cast.kin(this, x)) // if x is an instance of WrInt
            return WrInt(val + x.val);
        else if(cast.kin(123, x)) // if x is an integer
            return WrInt(x + val);

        // exits printing the stack trace and the message
        system.sterr("'operator+' between "
            + cast.desc(this) + " and "
            + cast.desc(x) + " is not supported.");
    }
// ...
```

### Overloading others operators
The operator `-` works like the operator `+`, while others operators support only
a _behavoir_, so they will have **zero or one** parameters.

They're all overloaded with the syntax (like any other normal function):

```js
func OPERATOR (PARAMETERS) CODE
```

Here is the list of all overloadable operators (where **rhs** means
**R**ight **H**and **S**ide and **CODE** is omitted):

```
// As we've seen
func + (rhs, is_unary);
func - (rhs, is_unary);

// Function call & array subscripting
func () (args...);
func [] (args...);

// Other operators
func ~ ();
func * (rhs);
func / (rhs);
func % (rhs);

func << (rhs);
func >> (rhs);

func > (rhs);
func >= (rhs);
func < (rhs);
func <= (rhs);
func == (rhs);
func != (rhs);

func & (rhs);
func | (rhs);
func ^ (rhs);
```

### Operators table

Here's the **priority table** (where **priority 0** is the **highest**):
<br> Feel free to consult this table while reading (or worse, writing) some difficult code.

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
| `!` | Pre | 1 | No | NOT operator |
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
| `&&` | Log | 10 | No | Logical AND operator |
| `||` | Log | 11 | No | logical OR operator |
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

|||
|--:|:---:|:--|
| [Previous](if-and-loops.md) | [Home](https://smudgelang.github.io/smudge/) | Next |
