libdecmath
==========

A library for doing decimal math. This library is C code implementing a decimal floating point type inspired by the IEEE 754-2008 decimal64 floating point format. It also has 16 digits of precision: while it is not binary compatible with the decimal64 format, it ought to produce the same results for calculations that do not have overflows to infinity or underflow to subnormals. The only really odd thing about the representation is that setting it to all zero bits gives the number 1.0.

The library requires 64 bit math, and the type uint64_t to be defined. The underlying type of the data is uint64_t, so a dm_double can be passed around like normal doubles and integers in code. It will also try to use a 128 bit type, if it believes it to be available. The library includes a stripped-down sub-library for doing 128 bit calculations, should an 128 bit integral type not be available.

You can define DM_NO_DOUBLE_MATH to not get conversions to/from double, and DM_NO_LONG_DOUBLE_MATH to not get conversions to/from 'long double'. Note that no double conversions implies no 'long double' conversions. The double conversion tests fail on the Cygwin MingW because 1e100 is one ULP from what I have to assume is the "right" answer. Also 1e300 is three ULPs away.

Unit tests have 99.7% line and branch coverage (though I don't have a tool that computes condition coverage). I'm fairly certain that the two lines and two branches cannot be exercised by that data type, but oh well. The code that is present is more likely to be right, though it's not a guarantee. While unit tests can expose when code OUGHT to be present, but isn't, it can't test for it. Someone making up cases can find logic that isn't present, but unit tests can't cover lines that don't yet exist. And then you have code that is technically equivalent, but produces different numerical results, and one of those results feels superior to other results.

Differences from IEEE-754:
* It doesn't support floating point exceptions.
* It doesn't have gradual underflow. The range of the exponent is larger to make up for this deficiency.
* They cannot be sorted using integer compares. The sign is stored in the wrong place for that. (This may be an issue with decimal64, though.)

Note that it is a royal pain to debug code when the compiler doesn't understand the type of a variable.
