# The Smudge Programming Language
### What is Smudge?
**Smudge** is a **S**calable, **M**ulti platform, **U**ser-friendly, **D**ynamically typed, **G**arbage-collected and **E**legant programming language written in pure C++11.

Smudge is a **very lightweight** and **general-purpose** interpreted language that could be used for either **desktop applications** or **scripts** or incorpored as well into any kind of software that requires a scripting side (for 'modding') such as videogames, text editors, 3d/2d modeling software, etc.

With its C-like syntax, Smudge provides a **simple** but **effective** and **powerful** way to write **elegant** and **modular** code. For newbies Smudge could at first glance appear much more complicated than other programming languages like Python, Ruby, Swift or Go, but, after some practise, they will find coding Smudge **natural** and **fun**. C/C++, Java, JavaScript and C# developers, however, will love Smudge from the beginning.

### How to build
Smudge can be built with the following commands:
```
./configure
make
make install
```
Command `./configure` can take different options, to see them type `./configure --help`

### Language overview
Let's write a simple Hello World program!
We write the following code in a file named `hello.sm`:
```
import std.io = io;
func main(){
    io.println("Hello, world!");
}
```
And we call `smudge hello.sm`: *voilà!* `Hello, world!` appears on the screen.
It's also possible to make the program directly executable by adding the following line at the start of the file:
```
#! /usr/local/smudge
```
This, called *hashbang* or *shabang*, it's a special Unix instruction that tells the shell where is located the interpreter for the script.
Now it's possible to run it by simply typing `./hello.sm`.

This script is called **main script** (and its box the **main box**), because it contains the **main** function.
As you can see, you can declare and define a function with the `func` keyword. We'll depeen functions later.

When you run a program written in Smudge, you have to pass as argument to the interpreter **only** the main script:
all other scripts are automatically found thanks to the **box system**.

The first line of the program **imports** the **box** `std.io` with the label `io`.
**Boxes** are the Smudge way to make the code **modular**, **clear** and **self-explaining**.
Each source file has its own box which can be imported from other boxes.
The import syntax is the following:
```
import PATH.NAME = LABEL; 
```
`PATH` cannot contain spaces and instead of `/` it has `.`.
`NAME` is the name of the file whose box has to be imported. **Note**: each box must have extension `.sm` (the **main box is an exception**), which is not part of `NAME`.
`LABEL` is the name used by the current box as **link** to the imported box.

The same **box** shouldn't be imported in different ways (i.e. different `PATH`s).
The **search directories** (i.e. the directories where the boxes are searched) can be shown with the option `-s` or `--show-paths`.
Also, you can add new ones with the option `-D <directory>`. The first search directory is **always** that folder where is contained the main script.

So, the import statement above, imports box contained in file `std/io.sm` with the label `io`. Actually, there's no `std/io.sm` file because the **S**mudge **S**tandard **L**ibrary is **native** (that doesn't mean non-customizable) but you can write your own implementation of the **SSL** and install it in one of the seach directories. You can disable the SSL with the option `-n` or `--no-lib`.

The third line simply uses the box `std.io` (through its "link" `io`) by calling its function `println` with the string `Hello, world!`. This functions does nothing more than printing its **argument** and a **newline character** (`\n`) to the `stdout`.

Now, it's time to do something more interesting!
Take a look at the full [documentation](https://smudgelang.github.io/smudge/) to learn more.

### Licensing
Smudge is licensed under the Apache License 2.0. To read the conditions take a look to the file `LICENSE` in the main folder.
Copyright 2017 Riccardo Musso
