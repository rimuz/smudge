# The Smudge Programming Language - Variables

### Variables
Variables are some space in memory where we store **values**.
In Smudge we **declare** a variable with the syntax:

```js
var name;
// or
var name = default_value;
```

In the first statement we declare a variable named `name` with value `null`,
while in the second we specify a **default value**.
Also, it's possible to declare **multiple variables** at once:

```js
var a, b, c, d; // all nulls
var e = 100, f, g = 1, h = e+1; // only f is null
```

### Using variables
We can **reassign** a variable with **another value** or read its value as well:

```js
import std.io;

var a = 2, b = 5; // here we declare the variables
func main(){
    io.println("a is: ", a); // here we read a's value
    a = a * b; // here we set a's value (could be written as a *= b)
    io.println("a is: ", a); // again, to check if it changed
}
```

OUTPUT:

```
a is: 2
a is: 10
```

It's **not mandatory** to keep a **same type** for a variable: you can, for example,
declare a variable with as default value an **integer** and put in it a **string** later.

### Variable types
Here are the fateful **variable types** (or **object types**):
For now it's not important to understand all of them, there's plenty of time to do it.

| Type Name | How to obtain | Description |
|:---------:|:--------------|:------------|
| **Null**  | keyword `null` | Null represents absence of value. |
| **Integer** | Integer literal | An integer number |
| **Float** | Floating-point literal | A floating point number |
| **String** | String literal (double or single quotes) | A vector of characters |
| **Class instance** | Calling `operator()` to a class. | An instance of a class. |  
| **Class** | `class <name> ... { }` | - |
| **Function** | `func <name> ... { }` | - |
| **Box** | keyword `box` (to get the current one) | - |
| **Reference** | through `ref(...)` | A reference (address of) another variable |

So, because a variable can have a value of **any of** the above types (plus others used internally by the interpreter), we can type:

```js
import std.io;

func main {
    var p = io.println; // we're assigning a function to p
    p("Hello, world!"); // we're using p
}
```

**Aliases** can be easily created using variables.

### Scopes and variables' life
Smudge, unlike many other interpreted programming languages, inherited the powerful
**scope rules** from **C-like** programming languages.

A variable can be declared in the **box scope** or inside a **local scope**.
The box scope is **outside** any function, while local scope **not**.
```js
var x = 1; // x is declared in a global scope

func main (){
    var y = 2; // y is declared in a local scope
    { // with braces we're creating a new scope
        var z = 3;
    } // now z is deleted (because become unused)
} // now y is deleted

// x will be deleted by the interpreter only when the program has ended.
```

Usually, we don't create an **inner-scope** directly with the braces like the above example,
but they will be created when using if statements and loops (that we'll see later).

### Variable declaration inside expressions
In Smudge we **can put** variable declations bigger expressions as well:
In this code both `x` and `y` will have value `100`.

```js
var x;
func main ()
    x = (var y = 100); // same as x = var y = 100;
```

In fact, Smudge treats variable **definitions just like assignments**: they will "return" a reference of the variable that can be, in turn, **reassigned**.

```js
func main()
    ((var x = 3) = 5) *= 2; // setting x to 10 in an original way.
```

This, of course, gives you a lot of **power**.
Here is an weird use of this feature:

```js
var x = 1, y = 2;

/*
 * Will call function given with
 * global var x *or* y, selected
 * by boolean value exclude.
 * The other parameter value
 * can be given or not.
 * Examples:
 *
 * callF(f, false, null)
 *    will call f(x, null)
 *
 * callF(f, true)
 *    will call f(0, y)
 *
*/
func callF(f, exclude, value = 0) {
    (exclude ? var x : var y) = value;
    return f(x, y);
}
```

Note: the one with `?` and `:` is called conditional expression,
and allows to inline an `if` condition in an expression (Again, too early to understand it at all).

### Functions, function arguments and variables
When we declare a **function**, it will be stored in a **global** variable of the current box. So, we can treat them as **normal variables**.

```js
import std.io;

func a io.println("a()");
func b io.println("b()");

func main {
    // swapping a and b
    var tmp = a;
    a = b;
    b = tmp;

    // calling functions
    a();
    b();
}
```

OUTPUT:

```
b()
a()
```

While **function arguments** are stored in its **local** scope:

```js
import std.io;

func main(){
    var x = "old value";
    f(x);
    io.println(x);
}

func f(a){ // a is a *copy* of x, so
    a = "new value"; // we set local a value
} // a destroyed
```

OUPUT:
```
old value
```

|||
|--:|:---:|:--|
| [Previous](functions.md) | [Home](https://smudgelang.github.io/smudge/) | [Next](if-and-loops.md) |
