## The Smudge Programming Language - box `std.math`
`std.math` is just an interface to the `C++` math library [`<cmath>`](http://www.cplusplus.com/reference/cmath/) plus a
couple of function to generate random numbers.
See also:

### Function `rand ()`
Generates and **returns** a random `float` between `0` and `1` (`1` excluded).

### Function `rand_int (min, max)`
Generates and **returns** a random `integer` between `min` and `max` (`max` excluded),
or `null` if either `min` or `max` is not an integer.

### Function `deg (r)`
Converts the angle `r` (`float` or `integer`) expressed in **radians** to **degrees**.
**Returns** the new `float` or `null`, if `r` is an invalid parameter.

### Function `rad (d)`
Converts the angle `r` (`float` or `integer`) expressed in **degrees** to **radians**.
**Returns** the new `float` or `null`, if `r` is an invalid parameter.

## Other functions
Each of the next functions accept as parameter a `float` and return a `float` or `null`
if the parameter is invalid (see the C++ docs on [cplusplus.com](http://www.cplusplus.com/reference/cmath/)).

Warning: **angles are expressed in radians** (both input and output values).

- Trigonometric functions:
    - `cos (x)` - compute cosine
    - TODO
