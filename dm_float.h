/*
BSD 3-Clause License

Copyright (c) 2026, Thomas DiModica
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef DM_FLOAT_H
#define DM_FLOAT_H

/*
   dm_float is a decimal floating point class with seven digits of
   significand and an exponent range of -63 to 63, inclusive.

   It is four bytes (32 bits) per number. The exponent range is intended to
   cover up the lack of subnormals. Yes, decimal32 has a much larger exponent
   range AND subnormals. Relative to binary, this has a larger exponent range.

   The significand is a seven-digit, normalized number: 1000000 - 9999999.

   The sign is stored as a bit between the exponent and significand.

   The exponent field is just a two's complement number, with -64 being
   reserved for special cases, indicated in the significand:
      All zeros : zero
      All ones : infinity
      Anything else : NaN

   The IEEE-754 exceptions perform their default behavior, as most users of
      the C++ types float or double would expect.
*/

#include "dm_double.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t dm_float;


dm_float dm_float_neg   (dm_float);           // -x
dm_float dm_float_add   (dm_float, dm_float); // x + y
dm_float dm_float_sub   (dm_float, dm_float); // x - y
dm_float dm_float_mul   (dm_float, dm_float); // x * y
dm_float dm_float_div   (dm_float, dm_float); // x / y
dm_float dm_float_add_r (dm_float, dm_float, int);
dm_float dm_float_sub_r (dm_float, dm_float, int);
dm_float dm_float_mul_r (dm_float, dm_float, int);
dm_float dm_float_div_r (dm_float, dm_float, int);

dm_float   dm_float_fromstring     (const char *); // To ensure correct rounding
dm_float   dm_float_from_dm_double (dm_double);
dm_double  dm_double_from_dm_float (dm_float);

#ifdef __cplusplus
}
#endif

#endif /* DM_FLOAT_H */
