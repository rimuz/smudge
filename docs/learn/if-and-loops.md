# The Smudge Programming Language - If statement and loops

### `if` statement
In this page you'll learn how to control the **behavior** of your programs.
The first way is using an `if statement`, which allows to **execute or not**
a statement (or several statements) according to the given **condition**.
The syntax is:

```js
if(condition)
    statement;
```

`condition` is an expression of type integer where `0` represents `false` and `1` represents `true`, while `statement` can be either a **statement** or a **group of statements** delimited
by a couple of braces (`{` and `}`), that will be executed only if `condition` is `true`.

When `condition` is not an integer (or a float), it will be converted to `false`
if `condition` is `null`, `true` otherwise. If you want to check only if an expression
is `null`, you can use the form `null(expression)`.

### `if` and `else`
`else` statements can follow `if`s to execute a statement (or more) only when
the `if` is **not** satisfied.
Here is the usage:

```js
if(condition)
    statement;
else
    statement2;
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
Loops are another way to control your **program's flow**.

### `while`
This is the simplest loop: it repeats some code as long as an expression is `true`.

```js
while(condition)
    statement;
```

Here's an example:

```js
import std.io = io;

func main {
    var max = 10;
    var i = 0;

    while(i != max)
        io.println("i: " + i++);
}
```

This code will print all numbers from 0 to 10 (10 excluded).
The 8th line we use a so called **postfix increment operator**, that will increment
the variable `i` returning the old value to the expression.

### `do`-`while`
There is a **variant of while** often useful:

```js
do
    expression;
while(condition);
```

This loop executes *at least* one time, then **repeats** the code if the `condition`
evaluates to `true`.

### `for`
This is of course the most **famous** loop, with the following syntax:

```js
for(init_expr; condition; iter_expr)
    expression;
```

And does the same as:

```js
init_expr;
while(condition){
    expression;
    iter_expr;
}
```

So, in the `for` loop the `init_expr` is **executed once** at the **beginning** of the cycle.
Then, it executes some code untill the `condition` is `false`, executing at the end of each
iteration the `init_expr`.

This example is equivalent to the previous one.

```js
import std.io = io;

func main {
    for(var i = 0; i != 10; ++i)
        io.println("i: " + i);
}
```

### `for-each`
This loop is more like a **syntax sugar**, because it simplifies iterating each object in another.
We'll call the object container an **iterable object**, which contains a method named `iterate()`
that returns an **iterator**.

In Smudge, iterators are objects with the method `next()` that returns another
object container (typically a **tuple**) holding:
- the **object extracted**
- an **integer**,

Then, this:

```js
for(label : iterable_object) // simple for-each, each item is stored in label
    expression; // and it's used here
```

Could be rewritten as:
```js
for(var iterator = iterable_object.iterate(), tuple = iterator.next();
        tuple.get(1); tuple = iterator.next()){
    // we explicitly store the item into the variable
    var label = tuple.get(0);
    expression;
}
```
A bit **tricky**, isn't it?

Note: the example uses the method [`std.lang::Tuple::get(idx)`](ssl/stdlang.md#method-get-idx-1),
while the interpreter could get the values in other ways.

Here is an example of usage (combining `for-each` with a **VARARG**):

```js
import std.io;

func main {
    io << sum(1, 2, 3, 4) << io.ln;
    io << sum([1], [2], [3], [4]) << io.ln;
}

func sum(args...){
    var sum = args[0];
    for(arg : args.slice(1)){
        sum = sum + arg;
    }
    return sum;
}
```

OUPUT:
```
10
[1, 2, 3, 4]
```

See also [`std.lang::List::slice()`](ssl/stdio.md#method-slice-start--0-end--size).
