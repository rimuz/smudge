## The Smudge Programming Language - The Syntax (Statements)
Any Smudge program consists of a bunch of **statements** inside **functions** or **methods** (we'll see the difference later).
Each statement ends with a **semicolon** (i.e. `;`) to avoid any case
of ambiguity. It's important to remember that, in Smudge, **using
colons is mandatory**. Thus, statements can be **longer than a single
line**. Here are three statements (It's not important now to understand
the meaning of each one):

```as
var a = 100;
a += f(a) * a;
io.println(
    [1, 2, 3]
);
```

To explain hard-to-understand code, you can use **comments**: those
are arbitrary texts **ignored by the interpreter**.
Smudge supports three types of comments:
- The double-slash single-line comment (everything between `//` and new-line will be ignored)
- The hash single-line comment (everything between `#` and new-line will be ignored)
- The multi-line comment (everythin between `/*` and `*/` will be ignored)
```as
/*
 * This is the same code, but commented.
*/
var a = 100; // instantiates variable 'a'
a += f(a) * a; # stuff..

/*
 * Actually, you shouldn't comment self-explaining
 * statements like these.
 */
io.println(
    [1, 2, 3]
);
```

As said before, if you want to actually try the code you write you need to
insert it into a **function**. The `main` function is the one which is
invoke by the **interpreter** as soon as it's ready to interpret statements.

```as
/*
 * This is how are defined functions in Smudge.
 * Note: the two round brackets are optional
 * in this case
*/

func main () {
    // Code...
}
```

Next, will see how to print stuff on the `stdout`.
