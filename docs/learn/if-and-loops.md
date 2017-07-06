# The Smudge Programming Language - If statement and loops

### `if` statement
In this page you'll learn how to control the **behavior** of your programs.
The first way is using an `if statement`, which allows to **execute or not**
a statement (or several statements) according to the given **condition**.
The syntax is:

```
if(<condition>)
    <statement>;
```

`<condition>` is an expression of type integer where `0` represents `false` and `1` represents `true`, while `<statement>` can be either a **statement** or a **group of statements** delimited
by a couple of braces (`{` and `}`), that will be executed only if `<condition>` is `true`.

When `<condition>` is not an integer (or a float), it will be converted to `false`
if `<condition>` is `null`, `true` otherwise. If you want to check only if an expression
is `null`, you can use the form `null(<expression>)`.

### `if` and `else`
`else` statements can follow `if`s to execute a statement (or more) only when
the `if` is not satisfied.
Here is the usage:

```
if(<condition>)
    <statement>;
else
    <statement2>;
```

#### Example
```js
import std.io = io;
func main {
    var i = io.int();
    if(!null(i))
        io.println("You typed an int");
    else
        io.println("You typed something else");
}
```
`std.io::int()` is a function that returns an int or `null` (see more [here](../ssl/stdio.md#function-int-)).

This code will check if the input string could be converted to integer or not.

### Loops
Loops are another way to control your program's flow

### `while`
This is the simplest loop: it repeats  
