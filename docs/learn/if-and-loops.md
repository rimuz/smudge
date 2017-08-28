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
import std.io;

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
Then, it executes some code **until** the `condition` is `false`, executing at the end of each
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

### The `switch` statement (note: not a loop!)
While in most programming languages `switch` works only with **specific types** (typically **PODs**,
Plain Old Data types), in Smudge you can use any type, in fact, as long as it does **support the
`operator == ()`**, you **can** insert it in a `switch`.
Plus, objects in the `case`s of the `switch` are **not necessary constant** expressions.

The `switch` is practically a **jump table**, below is the syntax:

```js
switch(object){
    case object1:
        code1;
    case object2:
        code2;
    ...
    case objectN:
        codeN;
    default:
        code3;
}
```

When this code is executed, the interpreter will call the `operator == ()` against
`object` to **check if** it's **equal to** `object1`:
- if **it is**, it will run `code1` then `code2` then `codeN` and then `code3`.
- if **it is not**, it will check if `object` is equal to `object2` and:
    - if it is, it will run `code2` then `codeN` and then `code3`.
    - if it is not, it will check if `object` is equal to any other case,
        and, if **it's different**, it will run only `code3` (the `default` case).

**Some tips about `switch`es**:
- `default` is **not necessary**.
- prefer `switch` instead of nested `else-if`s
- use `break` to exit the switch after `case`s

### `break` statement
Sometimes we need to **stop** a loop while it's iterating: in these cases we can use `break`.

```js
import std.io;

func main {
    io.println("Start of the program");
    var line;
    while(line = io.line()){
        if(line == "hello")
            io.println("Hello, world!");
        else if(line == "exit")
            break;
    }
    io.println("End of the program");
}
```

See [`std.io::line()`](ssl/stdio.md#function-line-).

This code will print `Hello, world!` if the input is `hello`, while will
end when `exit` is prompted (if the input is different, it will be ignored).

#### `break` and `switch`
`break` can be used in `switch`es, too.
Try the following code inputting **different values** and see what happens:

```js
import std.io;

func main {
    var magic = 200, x = 10;
    var val = io.int();

    switch(val){
        case magic:
            io << "MAGIC!" << io.ln;
            break;
        case x:
            io << "X!" << io.ln;
            break;
        case 200: // never used
            io << "200" << io.ln; // dead code
            break;
        case 10: // never used
            io << "10" << io.ln; // dead code
            break;
        case 3:
            io << "3" << io.ln;
            // no break;
        default:
            io << "unexpected" << io.ln;
    }
}
```

Some examples:

```
INPUT -> OUTPUT

200 -> MAGIC!
10 -> X!
3 -> 3 & unexpected
33 -> unexpected
```

### `continue` statement
Differently from the `break` statement, `continue` **doesn't stop** the loop,
but **jumps** to its next **iteration**; when applied to a `for` loop, it will also
run the `iter_expr` (described above).

**Note**: `continue` works in loops only (so not in `switch`es).

|||
|--:|:---:|:--|
| [Previous](vars.md) | [Home](https://smudgelang.github.io/smudge/) | [Next](operators.md) |
