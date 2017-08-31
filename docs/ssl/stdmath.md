# The Smudge Programming Language - box `std.math`
`std.math` is just an interface to the `C++` math library [`<cmath>`](http://www.cplusplus.com/reference/cmath/) plus a
couple of function to generate random numbers.
So, see also the original C++ docs on [cplusplus.com](http://www.cplusplus.com/reference/cmath/)).

## Var `NAN`
Contains the floating-point **N**ot **A** **N**umber value.
Note that `NAN == NAN` and `std.cast::same(NAN, NAN)` return **`false`**: use [`is_nan(x)`](stdmath.md#function-is_nan-).

## Var `INF`
Contains the floating-point **Infinity** value.

## Var `E`
The [Euler's number](https://en.wikipedia.org/wiki/E_(mathematical_constant)) stored in a `float`.

## Var `PI`
The [PI constant](https://en.wikipedia.org/wiki/Pi) stored in a `float`.

## Function `rand ()`
Generates and **returns** a random `float` between `0` and `1` (`1` excluded).

## Function `rand_int (min, max)`
Generates and **returns** a random `integer` between `min` and `max` (`max` excluded),
or `null` if either `min` or `max` is not an integer.

## Function `deg (r)`
Converts the angle `r` (`float` or `integer`) expressed in **radians** to **degrees**.
Returns the new `float`, or`null, if `r` is an invalid parameter.

## Function `rad (d)`
Converts the angle `r` (`float` or `integer`) expressed in **degrees** to **radians**.
Returns the new `float`, or`null, if `r` is an invalid parameter.

## Function `frexp (x)`
Returns a tuple containing the values of `significand` and `exp` that make true the following equation:<br>
**x = significand * 2<sup>exp</sup>**

## Function `hypot (a, b)`
Returns the length of the hypotenuse given the catheti (i.e. legs) `a` and `b` (`integer`s or `float`s) or null if the parameters are invalid.

# Other functions
Each of the next functions accept as parameter one or more `float`s (or `integer`s) and return a `float`, or`null if the parameters are invalid. **Angles are expressed in radians** (both input and output values).
Use `deg()` and `rad()` for angle conversions.

- Trigonometric functions:
    - `cos (x)` - compute cosine
    - `sin (x)` - compute sine
    - `tan (x)` - compute tangent
    - `acos (x)` - compute arc cosine
    - `asin (x)` - compute arc sine
    - `atan (x)` - compute arc tangent
    - `atan2 (x, y)` - compute arc tangent of y/x

- Hyperbolic functions:
    - `cosh (x)` - compute hyperbolic cosine
    - `sinh (x)` - compute hyperbolic sine
    - `tanh (x)` - compute hyperbolic tangent
    - `acosh (x)` - compute area hyperbolic cosine
    - `asinh (x)` - compute area hyperbolic sine
    - `atanh (x)` - compute area hyperbolic tangent

- Power functions:
    - `pow(x, y)` - compute x<sup>y</sup>
    - `sqrt (x)` - compute square root of `x`
    - `cbrt (x)` - compute cubic root of `x`

- Rounding functions:
    - `ceil (x)` - round up `x`
    - `floor (x)` - round down `x`
    - `trunc (x)` - truncate `x`
    - `round (x)` - round `x` to nearest value
    - `round_int (x)` - round `x` to nearest **integer** value (`round_int()` returns an integer).

- Exponential and logarithmic functions:
    - `exp (x)` - compute _e_<sup>x</sup>
    - `exp2 (x)` - compute 2<sup>x</sup>
    - `log (x)` - compute natural logarithm of `x`
    - `log2 (x)` - compute base-2 logarithm of `x`
    - `log10 (x)` - compute base-10 logarithm of `x`

- Check functions:
    - `is_nan (x)` - check if z is [**NAN**](stdmath.md#var-nan)
    - `is_inf (x)` - check if z is [**INF**](stdmath.md#var-inf)
