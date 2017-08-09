## The Smudge Programming Language - Documentation of box `std.system`
Usually imported as `sys`, `std.system` contains miscellaneous environment
settings and utilities.

### Var `VERSION`
An integer containing the Smudge version using the following rule:
in **hexadecimal**:

|||
|:-|:-:|:-|
| `XX` (major) | `YY` (minor) | `ZZ` (patch) |

For example: version **`7.4.3`** becomes **`070403`**.

### Var `STR_VERSION`
A string containing a **human readable** Smudge version.

### Var `DATE_VERSION`
A string containing the **date** of the current Smudge version expressed in `MM.YYYY`.

### Function `check ()`
**Returns** `true` only if the environment is ready to execute commands (via `run (cmd)` or
`() (cmd)`), `false` otherwise.

### Function `run (cmd)` and `() (cmd)`
Executes the **command** contained in the string `cmd`.
**Returns** `null`.

### Function `exit (i)`
**Exits** the program with, as **return value**, the integer `i`.
**Does not return anything**.

### Function `abort ()`
**Aborts** the current program (terminates with signal `SIGABRT`).
**Does not return anything**.
