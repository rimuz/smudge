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
Also, it's possible to declare multiple variable at once:

```js
var a, b, c, d; // all nulls
var e = 100, f, g = 1, h = e+1; // only f is null
```

### Using variables
We can reassign a variable with another value or read its value as well:

```js
import std.io = io;

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
declare a variable with as default value an integer and put in it a string later.

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

So, because a variable can have a value of any of the above types (plus others used internally by the interpreter), we can type:

```js
import std.io = io;

func main {
    var p = io.println; // we're assigning a function to p
    p("Hello, world!"); // we're using p
}
```

Aliases can be easily created using variables.

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

TODO
