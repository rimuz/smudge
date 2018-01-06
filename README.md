# The Smudge Programming Language
### What is Smudge?
**Smudge** is a **S**imple, **M**ulti platform, **U**ser-friendly, **D**ynamically typed, **G**arbage-collected and **E**legant programming language written in C++11.

Smudge is a **very lightweight** and **general-purpose** interpreted language that could be used for either **desktop applications** or **scripts** or incorporated as well into any kind of software that requires a scripting side (for 'modding') such as videogames, text editors, 3d/2d modeling software, etc.

With its C-like syntax, Smudge provides a **simple** but **effective** and **powerful** way to write **elegant** and **modular** code. For newbies Smudge could at first glance appear much more complicated than other programming languages like Python, Ruby, Swift or Go, but, after some practice, they will find coding Smudge **natural** and **fun**. C/C++, Java, JavaScript and C# developers, however, will love Smudge from the beginning.

## How to build
### Supported OSes
Smudge can be easily built on the following platforms:
- Any **POSIX**-compliant environment (e.g. **Linux**, **macOs** and **Cygwin**)
    - **make** is required.
    - a compiler supporting **C++11** (e.g. **GCC** and **Clang**) is required, too.
- Windows with **MinGW** or **MinGW-w64** and **MSYS** (see [more](http://www.mingw.org/wiki/msys))
- A \*nix with **MinGW-w64** (cross-compiling for Windows).

### Pre-installation
On any OS you'll need to have a **copy** of the project somewhere on your PC,
typically done with the command:

```
git clone https://github.com/rimuz/smudge
```

Now enter the newly created directory `smudge`.

### On POSIX
Just run the following:

```
./configure
make
make install
```

Note:
1. Command `./configure` can take several options, to see them type `./configure --help`.
1. Command `make` will produce the executable `smudge` (on Cygwin the library `libsmudge.a`, too).
1. Command `make install` will install on your machine the `smudge` executable
(typically in **`/usr/bin`**, but can be changed with `./configure` option `--prefix` (see point 1)) and Smudge's man pages on `/usr/share/man`.
1. Note also that command `make install` could require **`root` privileges** (**\*nix** and **macOs**).

Smudge is installed by default in directory **`/usr/bin`**, while additional libraries should be added inside `/usr/lib/smudge` or `/usr/lib/smudge/VERSION`.
Smudge **man pages** are copied in the directory `/usr/share/man/man1`.

### On MinGW or MinGW-w64 (Windows)
As said [here](http://www.mingw.org/wiki/msys), you should before _mount_ the MinGW directory (in this example is `C:\mingw`):

```
mount c:/mingw /mingw
```

Then build and install with:
```
./configure --prefix=/mingw
make
make install
```

It will build two files:
- `smudge.exe`, the interpreter, which will be installed with its man pages.
- `libsmudge.a`, the static library, which can be used to build Smudge native libraries.

### On MinGW-w64 (\*nix)
To cross-compile for Windows on Linux, Unix or any other environments supported by MinGW-w64
follow these steps:

```
./configure --host=YOUR_HOST
make
make install
```

Where `YOUR_HOST` is replaced by:
- `i686-w64-mingw32` to compile **32-bits** programs.
- `x86_64-w64-mingw32` to compile **64-bits** programs.

Such as MinGW or Cygwin on Windows, this will generate a static library named `libsmudge.a`
useful to build Smudge native libraries.


### [Getting Started](https://rimuz.github.io/smudge/)
Go to this [link](https://rimuz.github.io/smudge/) to learn how to program in Smudge.

Also, run `man smudge` to see the meaning of all the options you can give to `smudge`.

### Licensing
Smudge is licensed under the Apache License 2.0. To read the conditions take a look to the file `LICENSE` in the main folder.
Copyright 2016-2017 Riccardo Musso
