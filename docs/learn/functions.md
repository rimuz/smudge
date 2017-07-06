# The Smudge Programming Language - Functions

### Function usefulness
To prove the awesomeness of **functions**, I'll give you an example.
Let's write an application that says hello to our friend Marco in a funny way.

```js
import std.io = io;

func main(){
    /*
     * We could've written simply (with the same
     * number of dashes in the string):
     * io.println("---\nHello, Marco.\n---");
    */
    io.println(" ---------- ");
    io.println("Hello, Marco.");
    io.println(" ---------- ");
}
```
Now, we want to greet also our friends David, Aaron and Bill, but we want to **reuse** as much as possible our code. How we can do? With a function
obviously:

```js
import std.io = io;

func greet(name){
    io.println(" ---------- ");
    io.println("Hello, " + name + ".");
    io.println(" ---------- ");
}

func main(){
    greet("Marco");
    greet("David");
    greet("Aaron");
    greet("Bill");
}
```

Note: on the fifth line we performed a **concatenation** between strings via the operator `+`, this is because the **operator overloading** is widely supported by Smudge.

Actually there are ways much more elegant than this one, but, for now, this is OK for the explanation.

As you see, sometimes it's actually **more convenient** to write some code inside a function and call that function.

### Arguments
Functions can take **arguments** (sometimes called also parameters) (in our case there was an argument called `name`) which are objects passed by the function's **caller** that will probably influence the working of the function.
We call **pure** a function which depends **only** on the given arguments.
And we call **inpure** the ones which depend also on the _environment_ settings (practically, the global variables).

When a function doesn't have arguments, you **can omit** the round brackets
after its name:

```js
func function_without_arguments () { /* code */ }
// it's the same as:
func function_without_arguments { /* code */ }
```

However, is **good practise** to **always** type them, to make a **clearer** code.

Smudge is not finicky about the **arguments**: function callers, in fact, can give **more** or **less** of them than the expected number, or even **not give** arguments at all. The following example is **valid** code:

```js
import std.io = io;

func my_function(first_arg, second_arg, third_arg){
    io.println(first_arg, " ", second_arg, " ", third_arg);
}

func main(){
    my_function("A", "B", "C");
    my_function("A", 2, 10.5);
    my_function("A", null);
    my_function();
}
```

OUTPUT:

```
A B C
A 2 10.5
A <null> <null>
<null> <null> <null>
```

Note: `<null>` is the result converting `null` to a string.

### Default argument values
As you can see, arguments not given by default are `null`, but you can **customize** the default value of a function argument by **adding `=`** and the desired value **after** the argument name.
When an argument has a specified default value, it's called **optional**.
Let's see an example:

```js
import std.io = io;

func sum(a, b = 1){
    return a + b;
}

func main(){
    io.println(sum(2, 5), " ", sum(1), " ", sum(sum(5), 10));
}
```

OUTPUT:

```
7 2 16
```

Note: As shown in this example, functions can **return** a value via the return
statement which is very simple: `return VALUE;`.

Plus, we can set as default values some more complex expressions, for example:

```js
import std.io = io;

func f2 (lhs, rhs){
    return lhs + lhs*rhs;
}

/*
 * random calculates :D
*/
func f(a, b, c = f2(a, b*2) -1){
    return a + b / c;
}

func main(){
    // using floating-point operations!
    io.println(f(1.0, 2.0));
}
```

OUTPUT:

```
1.5
```

### VARARGs
At the end, functions can accept a **variable number of arguments**: those are called **VARARGs** functions and can be implemented by adding as **last argument** a special argument ending with three points:
```js
func varargs_func (arg0, arg1, /* argN, */ specialArg...) {}
```

Now, `specialArg` won't never be `null`. When there are no arguments to pass
to `specialArg` it will be an **empty list**. Otherwise it will be a list containing all the arguments in excess.
Let's see a simple example:

```js
import std.io = io;

func foo(args...){
    io.println("args: ", args);
}

func bar(a = 1, b = a+1, c...){
    io.println("a: ", a, "\nb: ", b, "\nc: ", c);
}

func main(){
    foo(0, 1, 2);
    bar(0, 1, 2, 3, 4, 5);
    foo(0, "Hello", [1, 2]);
    foo();


    bar(0, 1, 2);
    bar(0, 1, 2, 3, 4, 5);
    bar(0, "Hello", [1, 2]);
    bar();
}
```

OUTPUT, as expected:

```
args: [0, 1, 2]
args: [0, 1, 2, 3, 4, 5]
args: [0, Hello, [1, 2]]
args: []
a: 0, b: 1, c: [2]
a: 0, b: 1, c: [2, 3, 4, 5]
a: 0, b: Hello, c: [[1, 2]]
a: 1, b: 2, c: []
```

NOTE: We've **alredy** used a lot VARARGS: in fact, the `std.io::println` function is implemented via VARARGS!

```js
/*
 * in box std.io, natively
*/

// [...]

func println(objects...){
    /* CODE to print each element in 'objects' */
    /* CODE to print new-line character ('\n') */
}

// [...]
```

This **allows** writing `io.println("Hello")`, `io.println("Hello ", "World")`, `io.println("a", "b", "c")` and so on.

|||
|--:|:---:|:--|
| [Previous](statements.md) | [Home](https://smudgelang.github.io/smudge/) | [Next](if-and-loops.md) |
