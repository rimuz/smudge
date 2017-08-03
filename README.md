# The Smudge Programming Language
### What is Smudge?
**Smudge** is a **S**imple, **M**ulti platform, **U**ser-friendly, **D**ynamically typed, **G**arbage-collected and **E**legant programming language written in pure C++11.

Smudge is a **very lightweight** and **general-purpose** interpreted language that could be used for either **desktop applications** or **scripts** or incorpored as well into any kind of software that requires a scripting side (for 'modding') such as videogames, text editors, 3d/2d modeling software, etc.

With its C-like syntax, Smudge provides a **simple** but **effective** and **powerful** way to write **elegant** and **modular** code. For newbies Smudge could at first glance appear much more complicated than other programming languages like Python, Ruby, Swift or Go, but, after some practise, they will find coding Smudge **natural** and **fun**. C/C++, Java, JavaScript and C# developers, however, will love Smudge from the beginning.

### How to build
If you are on **Linux**, **Mac** or **Cygwin** (or any other POSIX compliant platform),
follow these steps:

First, clone the sources in your current working directory (see it with `pwd`):
```
git clone https://github.com/smudgelang/smudge
```
Then, go to the new directory `smudge`
```
cd smudge
```
And finally build and install it with the following commands:
```
./configure
make
make install
```
Note: Command `./configure` can take different options, to see them type `./configure --help`.

Smudge is installed by default in directory `/usr/bin`, while additional libraries should be added inside `/usr/lib/smudge` or `/usr/lib/smudge/VERSION`

### [Getting Started](https://smudgelang.github.io/smudge/)
Go to this [link](https://smudgelang.github.io/smudge/) to learn how to program in Smudge.

### On Holiday :D
I won't commit a lot for the next two weeks: from 7/18 to 8/3s

### Licensing
Smudge is licensed under the Apache License 2.0. To read the conditions take a look to the file `LICENSE` in the main folder.
Copyright 2016-2017 Riccardo Musso
