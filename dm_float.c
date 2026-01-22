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

#include <math.h>
#include <ctype.h>
#include <inttypes.h>

#include "dm_float.h"

static const int16_t SPECIAL_EXPONENT = -64;
static const int16_t MAX_EXPONENT = 63;
static const int16_t MIN_EXPONENT = -MAX_EXPONENT;
static const uint32_t DM_INFINITY = 0xFFFFFFU;
static const uint32_t SIGN_BIT = 0x1000000U;

static const uint32_t BIAS =           10000000U;
static const uint32_t MAX_SIGNIFICAND = 9999999U;
static const uint32_t MIN_SIGNIFICAND = 1000000U;
static const int16_t CUTOFF = 7; // Difference in exponents where we consider the operation degenerate.

#define DM_FLOAT_PACK(sign, exponent, significand) ((((uint32_t)(exponent)) << 25) | (significand) | ((sign) ? SIGN_BIT : 0))

#define DM_FLOAT_UNPACK_EXPONENT(value) (((int32_t)(value)) >> 25)
#define DM_FLOAT_UNPACK_SIGNIFICAND(value) ((value) & DM_INFINITY)

const dm_float dm_float_Zero = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0);
const dm_float dm_float_NaN  = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 255);
const dm_float dm_float_Inf  = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY);

int dm_float_fpclassify(dm_float arg)
 {
   int16_t exponent = DM_FLOAT_UNPACK_EXPONENT(arg);
   if (SPECIAL_EXPONENT != exponent)
      return FP_NORMAL;
   uint64_t significand = DM_FLOAT_UNPACK_SIGNIFICAND(arg);
   if (DM_INFINITY == significand)
      return FP_INFINITE;
   if (0 == significand)
      return FP_ZERO;
   return FP_NAN;
 }

int dm_float_isinf(dm_float arg)
 {
   return FP_INFINITE == dm_float_fpclassify(arg);
 }

int dm_float_isnan(dm_float arg)
 {
   return FP_NAN == dm_float_fpclassify(arg);
 }

int dm_float_signbit(dm_float arg)
 {
   return (arg & SIGN_BIT) ? 1 : 0;
 }

int dm_float_iszero(dm_float arg)
 {
   return FP_ZERO == dm_float_fpclassify(arg);
 }

dm_float dm_float_neg(dm_float arg)
 {
   return arg ^ SIGN_BIT;
 }

extern const uint64_t makeShift [];
extern int dm_global_round_mode;
uint64_t dm_internal_round(uint64_t quot, uint64_t rem);

dm_float dm_float_add(dm_float lhs, dm_float rhs)
 {
   return dm_float_add_r(lhs, rhs, dm_global_round_mode);
 }

dm_float dm_float_sub(dm_float lhs, dm_float rhs)
 {
      // Only handle NaNs, as we don't want to mutate the payload of an rhs NaN.
   if (dm_float_isnan(lhs)) return lhs; // Prefer lhs NaN's payload.
   if (dm_float_isnan(rhs)) return rhs;

   return dm_float_add_r(lhs, dm_float_neg(rhs), dm_global_round_mode);
 }

dm_float dm_float_mul(dm_float lhs, dm_float rhs)
 {
   return dm_float_mul_r(lhs, rhs, dm_global_round_mode);
 }

dm_float dm_float_div(dm_float lhs, dm_float rhs)
 {
   return dm_float_div_r(lhs, rhs, dm_global_round_mode);
 }

dm_float dm_float_add_r(dm_float lhs, dm_float rhs, int round_mode)
 {
      // First, handle NaNs, as they have the highest precedence.
   if (dm_float_isnan(lhs)) return lhs; // Prefer lhs NaN's payload.
   if (dm_float_isnan(rhs)) return rhs;

      // Next, if both are infinities.
   if (dm_float_isinf(lhs) && dm_float_isinf(rhs))
    {
      if (dm_float_signbit(lhs) != dm_float_signbit(rhs)) return dm_float_NaN; // Inf - Inf = NaN
      return lhs; // Both have the same sign, so just return lhs.
    }
      // Anything else with infinity is infinity
   if (dm_float_isinf(lhs)) return lhs;
   if (dm_float_isinf(rhs)) return rhs;

      // Finally, zeros.
   if (dm_float_iszero(lhs) && dm_float_iszero(rhs))
    {
      if ((dm_float_signbit(lhs) && dm_float_signbit(rhs)) || ((dm_float_signbit(lhs) != dm_float_signbit(rhs)) && (DM_FE_DOWNWARD == round_mode)))
         return dm_float_neg(dm_float_Zero);
      return dm_float_Zero; // All other cases return positive zero.
    }
   if (dm_float_iszero(lhs)) return rhs;
   if (dm_float_iszero(rhs)) return lhs;

      // Now, we can start the add proper!
   uint32_t lhd = DM_FLOAT_UNPACK_SIGNIFICAND(lhs);
   uint32_t rhd = DM_FLOAT_UNPACK_SIGNIFICAND(rhs);
   uint32_t resultSignificand;

      // Normalize exponents
   int16_t lhse = DM_FLOAT_UNPACK_EXPONENT(lhs);
   int16_t rhse = DM_FLOAT_UNPACK_EXPONENT(rhs);
   int32_t resultExponent = lhse;
   int16_t expDiff = 0;
   if (lhse > rhse)
    {
      if ((lhse - rhse) <= (CUTOFF + 1))
       {
         expDiff = lhse - rhse;
         if (expDiff > 2) // 2 : guard digit, rounding digit / sticky digit
          {
            uint32_t removed;
            removed = rhd % (uint32_t)makeShift[(expDiff - 2) + 1];
            rhd /= (uint32_t)makeShift[(expDiff - 2) + 1];
            rhd += dm_internal_round(rhd, removed);
            expDiff = 2;
          }
         lhd *= (uint32_t)makeShift[expDiff + 1];
       }
      else
       {
         expDiff = 2;
         lhd *= 100U;
         rhd = 1U;
       }
    }
   else if (lhse < rhse)
    {
      resultExponent = rhse;
      if ((rhse - lhse) <= (CUTOFF + 1))
       {
         expDiff = rhse - lhse;
         if (expDiff > 2)
          {
            uint32_t removed;
            removed = lhd % (uint32_t)makeShift[(expDiff - 2) + 1];
            lhd /= (uint32_t)makeShift[(expDiff - 2) + 1];
            lhd += dm_internal_round(lhd, removed);
            expDiff = 2;
          }
         rhd *= (uint32_t)makeShift[expDiff + 1];
       }
      else
       {
         expDiff = 2;
         rhd *= 100U;
         lhd = 1U;
       }
    }

      // Compute the digits and sign.
   int resultSign = dm_float_signbit(lhs);
   if (dm_float_signbit(lhs) == dm_float_signbit(rhs)) // Both same sign
    {
      lhd += rhd;
    }
   else // Different Signs
    {
      if (lhd >= rhd) // lhs larger : result sign of lhs
         lhd -= rhd;
      else // rhs larger : result sign of rhs (opposite sign of lhs)
       {
         lhd = rhd - lhd;
         resultSign = !resultSign;
       }
    }
   resultSignificand = lhd;

      // Normalize the result
   if (0U == lhd)
    {
      if (DM_FE_DOWNWARD != round_mode) resultSign = 0;
      else resultSign = 1;
      resultExponent = SPECIAL_EXPONENT;
      resultSignificand = 0;
    }
   else if (lhd > MAX_SIGNIFICAND)
    {
      if (0 == expDiff) // Did overflow occur? ie 9 + 9
       {
         ++resultExponent;
       }
      uint32_t temp = (uint32_t)makeShift[expDiff]; // This is why the table has one extra entry: we want the previous entry here.
      uint32_t test = BIAS * (uint32_t)makeShift[expDiff];
      if (lhd < test) // Did destructive cancellation occur? ie 100 - 1.
       {
         --resultExponent;
       }
      else
       {
         temp *= 10U;
         test *= 10U;
         if (lhd >= test) // Did overflow occur? ie 99 + 1
          {
            ++resultExponent;
            temp *= 10U;
          }
       }
      uint32_t rem;
      resultSignificand = lhd / temp;
      rem = lhd % temp;
      resultSignificand += dm_decideRound(resultSign, resultSignificand & 1U, (int64_t)temp - (int64_t)(rem << 1U), rem == 0U, round_mode);
      if (resultSignificand == BIAS)
       {
         resultSignificand = MIN_SIGNIFICAND;
         ++resultExponent;
       }
      if (resultExponent > MAX_EXPONENT) // Flush to infinity?
       {
         resultSignificand = DM_INFINITY;
         resultExponent = SPECIAL_EXPONENT;
       }
    }
   else
    {
      if (expDiff == 1) // If we have gotten here, and an operation like 10 - 1 occurred, then fix the exponent.
       {
         --resultExponent;
       }
      while (resultSignificand < MIN_SIGNIFICAND)
       {
         resultSignificand *= 10U;
         --resultExponent;
       }
      if (resultExponent < MIN_EXPONENT) // Flush to zero?
       {
         resultSignificand = 0;
         resultExponent = SPECIAL_EXPONENT;
       }
    }

   return DM_FLOAT_PACK(resultSign, resultExponent, resultSignificand);
 }

dm_float dm_float_sub_r(dm_float lhs, dm_float rhs, int round_mode)
 {
      // Only handle NaNs, as we don't want to mutate the payload of an rhs NaN.
   if (dm_float_isnan(lhs)) return lhs; // Prefer lhs NaN's payload.
   if (dm_float_isnan(rhs)) return rhs;

   return dm_float_add_r(lhs, dm_float_neg(rhs), round_mode);
 }

dm_float dm_float_mul_r(dm_float lhs, dm_float rhs, int round_mode)
 {
   int resultSign = dm_float_signbit(lhs) != dm_float_signbit(rhs);

      // First, handle NaNs, as they have the highest precedence.
   if (dm_float_isnan(lhs)) return lhs; // Prefer lhs NaN's payload.
   if (dm_float_isnan(rhs)) return rhs;

      // Next, infinity times zero is NaN.
   if ((dm_float_isinf(lhs) && dm_float_iszero(rhs)) || (dm_float_iszero(lhs) && dm_float_isinf(rhs)))
    {
      if (resultSign) return dm_float_neg(dm_float_NaN); // Preserve the sign of NaN ...
      return dm_float_NaN;
    }

      // Next, if either are infinities.
   if (dm_float_isinf(lhs) || dm_float_isinf(rhs))
    {
      if (resultSign) return dm_float_neg(dm_float_Inf); // Preserve the sign of infinity.
      return dm_float_Inf;
    }

      // Finally, zeros.
   if (dm_float_iszero(lhs) || dm_float_iszero(rhs))
    {
      if (resultSign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

      // Compute the exponent of the result, and use it to decide if we are even multiplying.
   int16_t resultExponent = DM_FLOAT_UNPACK_EXPONENT(lhs) + DM_FLOAT_UNPACK_EXPONENT(rhs);
   if (resultExponent > MAX_EXPONENT)
    {
      if (resultSign) return dm_float_neg(dm_float_Inf);
      return dm_float_Inf;
    }
   else if (resultExponent < (MIN_EXPONENT - 1)) // Minus one in case we can save the result from flushing to zero.
    {
      if (resultSign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

      // Now, we can start the multiply proper!
   uint64_t lhd = DM_FLOAT_UNPACK_SIGNIFICAND(lhs);
   lhd *= DM_FLOAT_UNPACK_SIGNIFICAND(rhs);

   uint64_t temp = MIN_SIGNIFICAND;
   uint64_t test = BIAS; // Two steps to prevent overflow.
   test *= MIN_SIGNIFICAND;
   if (lhd >= test)
    {
      temp *= 10U;
      ++resultExponent;
    }

   uint64_t resultSignificand, rem;
   resultSignificand = lhd / temp;
   rem = lhd % temp;
      // This can't cause an overflow.
   resultSignificand += dm_decideRound(resultSign, resultSignificand & 1U, (int64_t)temp - (int64_t)(rem << 1U), rem == 0U, round_mode);

   if (resultExponent > MAX_EXPONENT) // Flush to infinity?
    {
      if (resultSign) return dm_float_neg(dm_float_Inf);
      return dm_float_Inf;
    }
   if (resultExponent < MIN_EXPONENT) // Were we unsuccessful in saving a result from flushing to zero?
    {
      if (resultSign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

   return DM_FLOAT_PACK(resultSign, resultExponent, resultSignificand);
 }

dm_float dm_float_div_r(dm_float lhs, dm_float rhs, int round_mode)
 {
   int resultSign = dm_float_signbit(lhs) != dm_float_signbit(rhs);

      // First, handle NaNs, as they have the highest precedence.
   if (dm_float_isnan(lhs)) return lhs; // Prefer lhs NaN's payload.
   if (dm_float_isnan(rhs)) return rhs;

      // Next, zero divided by zero or infinity divided by infinity is NaN.
   if ((dm_float_iszero(lhs) && dm_float_iszero(rhs)) || (dm_float_isinf(lhs) && dm_float_isinf(rhs)))
    {
      if (resultSign) return dm_float_neg(dm_float_NaN); // Preserve the sign of NaN ...
      return dm_float_NaN;
    }

      // Next, if the result is infinity.
   if (dm_float_isinf(lhs) || dm_float_iszero(rhs))
    {
      if (resultSign) return dm_float_neg(dm_float_Inf);
      return dm_float_Inf;
    }

      // Finally, if the result is zero.
   if (dm_float_iszero(lhs) || dm_float_isinf(rhs))
    {
      if (resultSign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

      // Compute the exponent of the result, and use it to decide if we are even dividing.
   int16_t resultExponent = DM_FLOAT_UNPACK_EXPONENT(lhs) - DM_FLOAT_UNPACK_EXPONENT(rhs);
   if (resultExponent > (MAX_EXPONENT + 1)) // Plus one in case we can save the result from flushing to infinity.
    {
      if (resultSign) return dm_float_neg(dm_float_Inf);
      return dm_float_Inf;
    }
   else if (resultExponent < MIN_EXPONENT)
    {
      if (resultSign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

      // Now, we can start the divide proper!
   uint64_t lhd = DM_FLOAT_UNPACK_SIGNIFICAND(lhs);
   uint64_t rhd = DM_FLOAT_UNPACK_SIGNIFICAND(rhs);

   if (lhd < rhd)
    {
      --resultExponent;
    }
   else
    {
      rhd *= 10U;
    }

   lhd *= BIAS;
   uint64_t resultSignificand, rem;
   resultSignificand = lhd / rhd;
   rem = lhd % rhd;
      // This can't cause an overflow.
   resultSignificand += dm_decideRound(resultSign, resultSignificand & 1U, (int64_t)rhd - (int64_t)(rem << 1U), rem == 0U, round_mode);

   if (resultExponent > MAX_EXPONENT) // Were we unsuccessful in saving a result from flushing to infinity?
    {
      if (resultSign) return dm_float_neg(dm_float_Inf);
      return dm_float_Inf;
    }
   if (resultExponent < MIN_EXPONENT) // Flush to zero?
    {
      if (resultSign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

   return DM_FLOAT_PACK(resultSign, resultExponent, resultSignificand);
 }

   // This function is almost entirely unchanged and only exists so that
   // numbers input as strings are correctly rounded as floats.
dm_float dm_float_fromstring(const char * arg)
 {
   int resultSign = 0;
   int32_t resultExponent = -1;
   uint32_t resultSignificand = 0U;
   int digits = 0;
   int hasResidue = 0;
   int allZero = 1;
   int realDigit = 0;
   int residue = 0;

   if ('-' == *arg)
    {
      resultSign = 1;
      ++arg;
    }

   while (isdigit((unsigned char)*arg))
    {
      if (digits < CUTOFF)
       {
         if (realDigit || ('0' != *arg))
          {
            resultSignificand = resultSignificand * 10U + *arg - '0';
            ++digits;
            realDigit = 1;
          }
       }
      else
       {
         if (!hasResidue)
          {
            hasResidue = 1;
            if ('0' == *arg)
             {
               // Do Nothing
             }
            else
             {
               allZero = 0;
               if (*arg > '5')
                {
                  residue = -1;
                }
               else if (*arg < '5')
                {
                  residue = 1;
                }
             }
          }
         else
          {
            if (0 == residue)
             {
               if ('0' == *arg)
                {
                  // Do Nothing
                }
               else
                {
                  if (allZero)
                     residue = 1;
                  else
                     residue = -1;
                  allZero = 0;
                }
             }
          }
       }
      if (realDigit)
         ++resultExponent;
      ++arg;
    }
   if (('.' == *arg) || (',' == *arg))
    {
      ++arg;
    }
   while (isdigit((unsigned char)*arg))
    {
      if (digits < CUTOFF)
       {
         if (realDigit || ('0' != *arg))
          {
            resultSignificand = resultSignificand * 10U + *arg - '0';
            ++digits;
            realDigit = 1;
          }
       }
      else
       {
         if (!hasResidue)
          {
            hasResidue = 1;
            if ('0' == *arg)
             {
               // Do Nothing
             }
            else
             {
               allZero = 0;
               if (*arg > '5')
                {
                  residue = -1;
                }
               else if (*arg < '5')
                {
                  residue = 1;
                }
             }
          }
         else
          {
            if (0 == residue)
             {
               if ('0' == *arg)
                {
                  // Do Nothing
                }
               else
                {
                  if (allZero)
                     residue = 1;
                  else
                     residue = -1;
                  allZero = 0;
                }
             }
          }
       }
      if (!realDigit)
         --resultExponent;
      ++arg;
    }
   while (digits < CUTOFF)
    {
      resultSignificand *= 10U;
      ++digits;
    }
   if (allZero) // By the definition of the comp argument.
      residue = 1;
   resultSignificand += dm_decideRound(resultSign, 1U & resultSignificand, residue, allZero, dm_global_round_mode);
   if (resultSignificand == BIAS)
    {
      resultSignificand = MIN_SIGNIFICAND;
      ++resultExponent;
    }
   if ('e' == (*arg | ' '))
    {
      ++arg;
      int32_t exponentSign = 1;
      int32_t exponentValue = 0;
      if ('-' == *arg)
       {
         exponentSign = -1;
         ++arg;
       }
      if ('+' == *arg)
       {
         ++arg;
       }
      while (isdigit((unsigned char)*arg))
       {
         exponentValue = exponentValue * 10 + *arg - '0';
         ++arg;
         if (((exponentValue * exponentSign) > (2 * MAX_EXPONENT)) || ((exponentValue * exponentSign) < (2 * MIN_EXPONENT)))
            break;
       }
      resultExponent += exponentValue * exponentSign;
    }

   if (0U == resultSignificand)
    {
      resultExponent = SPECIAL_EXPONENT;
    }

   if (resultExponent > MAX_EXPONENT) // Flush to infinity?
    {
      if (resultSign) return dm_float_neg(dm_float_Inf);
      return dm_float_Inf;
    }
   if (resultExponent < MIN_EXPONENT) // Flush to zero?
    {
      if (resultSign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

   return DM_FLOAT_PACK(resultSign, resultExponent, resultSignificand);
 }

   // DANGER! DANGER! DANGER!
   //    While this beauty looks tame, it relies on the internals of only one of the two implementations of dm_double.

#define DM_DOUBLE_PACK(sign, exponent, significand) ((((uint64_t)(exponent)) << 54) | ((significand) - 1000000000000000ULL) | ((sign) ? 0x20000000000000ULL : 0))
#define DM_DOUBLE_PACK_ALT(sign, exponent, significand) ((((uint64_t)(exponent)) << 54) | (significand) | ((sign) ? 0x20000000000000ULL : 0))

#define DM_DOUBLE_UNPACK_EXPONENT(value) (((int64_t)(value)) >> 54)
#define DM_DOUBLE_UNPACK_SIGNIFICAND(value) (((value) & 0x1FFFFFFFFFFFFFULL) + 1000000000000000ULL)
#define DM_DOUBLE_UNPACK_SIGNIFICAND_ALT(value) ((value) & 0x1FFFFFFFFFFFFFULL)

dm_float dm_float_from_dm_double(dm_double input)
 {
   int16_t exponent = DM_DOUBLE_UNPACK_EXPONENT(input);
   uint64_t significand = DM_DOUBLE_UNPACK_SIGNIFICAND(input);
   int sign = input & 0x20000000000000ULL ? 1 : 0;

   if (-512 == exponent) // It's special.
    {
      significand = DM_DOUBLE_UNPACK_SIGNIFICAND_ALT(input);
      if (0x1FFFFFFFFFFFFFULL == significand) // If it's infinity, return infinity.
       {
         if (sign) return dm_float_neg(dm_float_Inf);
         return dm_float_Inf;
       }
      // I'm not really sure what to do here but truncate any extra NaN information....
      significand = significand & DM_INFINITY;
      if (DM_INFINITY == significand) // But, don't promote a NaN to infinity.
       {
         if (sign) return dm_float_neg(dm_float_NaN);
         return dm_float_NaN;
       }
      // This is now either zero or a valid NaN.
      return DM_FLOAT_PACK(sign, SPECIAL_EXPONENT, (uint32_t)(significand));
    }

   // It ain't special.
   uint32_t resultSignificand = significand / makeShift[10];
   uint32_t rem = significand % makeShift[10];
   resultSignificand += dm_decideRound(sign, resultSignificand & 1U, (int64_t)makeShift[10] - (int64_t)(rem << 1U), rem == 0U, dm_global_round_mode);
   if (BIAS == resultSignificand)
    {
      resultSignificand = MIN_SIGNIFICAND;
      ++exponent;
    }

   if (exponent > MAX_EXPONENT) // Flush to infinity?
    {
      if (sign) return dm_float_neg(dm_float_Inf);
      return dm_float_Inf;
    }
   if (exponent < MIN_EXPONENT) // Flush to zero?
    {
      if (sign) return dm_float_neg(dm_float_Zero);
      return dm_float_Zero;
    }

   return DM_FLOAT_PACK(sign, exponent, resultSignificand);
 }

dm_double dm_double_from_dm_float(dm_float input)
 {
   int16_t exponent = DM_FLOAT_UNPACK_EXPONENT(input);
   uint64_t significand = DM_FLOAT_UNPACK_SIGNIFICAND(input);
   int sign = input & SIGN_BIT ? 1 : 0;

   // Special cases...
   if (exponent == SPECIAL_EXPONENT)
    {
      // Fix the significand of infinity.
      if (significand == DM_INFINITY)
       {
         return DM_DOUBLE_PACK_ALT(sign, -512, 0x1FFFFFFFFFFFFFULL);
       }
      // Otherwise: zero remains zero and NaN remains NaN.
      return DM_DOUBLE_PACK_ALT(sign, -512, significand);
    }

   // All other cases.
   return DM_DOUBLE_PACK(sign, exponent, significand * makeShift[10]);
 }
