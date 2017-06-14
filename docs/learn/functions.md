# The Smudge Programming Language - Functions

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

As you see, sometimes it's actually **more convenient** to write some code inside a function and call this function.
Functions can take **arguments** (sometimes called also parameters) (in our case there was an argument called `name`) which are objects passed by the function's **caller** that will probably influence the working of the function.
We call **pure** a function which depends **only** on the given arguments.
And we call **inpure** the ones which depend also on the _environment_ settings (practically, the global variables).

When a function doesn't have arguments, you **can omit** the round brackets
after its name:

```js
func function_without_arguments () {}
// it's the same as:
func function_without_arguments {}
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

Plus, we can set as default values some morte complex expressions, for example:

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

|||
|--:|:---:|:--|
| [Previous](statements.md) | [Home](https://smudgelang.github.io/smudge/) | Next |
