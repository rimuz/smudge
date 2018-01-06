# The Smudge Programming Language - Hello World

### Hello world
Let's write a simple Hello World program!
We type the following code into a file named `hello.sm`:

```js
import std.io;
func main(){
    io.println("Hello, world!");
}
```

And we call `smudge hello.sm`: *voilà!* `Hello, world!` appears on the screen.
It's also possible to make the program directly executable by adding the following line at the start of the file:

```js
#! /usr/bin/smudge
```

This, called *hashbang* or *shabang*, it's a special Unix instruction that tells the shell where is located the interpreter for the script.
Now it's possible to run it by simply typing `./hello.sm`.

This script is called **main script** (and its box the **main box**), because
it contains the **main** function.
As you can see, you can declare and define a function with the `func` keyword.
We'll deepen functions later.

When you run a program written in Smudge, you have to pass as argument to the interpreter **only** the main script:
all other scripts are automatically found thanks to the **box system**.

### Boxes
The first line of the program **imports** the **box** `std.io` (which is part of the **SSL**, the Smudge Standard Library) with the label `io`.
**Boxes** are the Smudge way to make the code **modular**, **clear** and **self-explaining**.
Each source file has its own box which can be imported from other boxes.
The import syntaxes are the following:

```js
import <path>.<name>;
```

```js
import <path>.<name> = <label>;
```

`<path>` cannot contain spaces and instead of *slashes* (i.e. `/`) it has *dots* (i.e. `.`).
`<name>` is the name of the file whose box has to be imported. **Note**: each box must have extension `.sm` (the **main box is an exception**), which is not part of `<name>`.
`<label>` is the name used by the current box as **link** to the imported box, in the first
syntax `<label>` is equal to `<name>`.

The same box **shouldn't** be imported in different ways (i.e. **different `PATH`s**), but can be imported different times (with **different** link **names**, obviously).
The **search directories** (i.e. the directories where the boxes are searched) can be shown with the option `-s` or `--show-paths`.
Also, you can add new ones with the option `-D <directory>`. The first search directory is **always** the folder where is contained the main script.

### Smudge Standard Library (SSL)
So, the import statement above, imports box contained in file `std/io.sm` with
the label `io`. Actually, there's no `std/io.sm` file because the **SSL is native** (that doesn't mean non-customizable) but
you can write your own implementation of the **SSL** and install it in one of the search directories. You can disable the SSL with the option `-n` or `--no-stdlib`.

The third line simply uses the box `std.io` (through its "link" `io`) by calling its function `println` with the string `Hello, world!` as parameter. This function does nothing more than printing its **argument** and a **newline character** (`\n`) to the `stdout`.

Next, we'll learn more about statements.

||||
|---:|:---:|:---|
| Previous | [Home](https://rimuz.github.io/smudge/) | [Next](statements.md) |
