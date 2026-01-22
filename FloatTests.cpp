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
#include "gtest/gtest.h"

#include <cmath>

#include "dm_float.h"
#define DM_FLOAT_PACK(sign, exponent, significand) ((((uint32_t)(exponent)) << 25) | (significand) | ((sign) ? SIGN_BIT : 0))

const int16_t SPECIAL_EXPONENT = -64;
const uint32_t DM_INFINITY = 0xFFFFFFU;
const uint32_t SIGN_BIT = 0x1000000U;
const uint32_t MIN_SIGNIFICAND = 1000000U;

#ifdef __cplusplus
extern "C" {
#endif
int dm_float_fpclassify(dm_float arg);
int dm_float_isinf(dm_float arg);
int dm_float_isnan(dm_float arg);
int dm_float_signbit(dm_float arg);
int dm_float_iszero(dm_float arg);
#ifdef __cplusplus
}
#endif

TEST(DMFloatTest, testClassifiers)
 {
   dm_float positiveZero = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U); // Special exponent, zero significand : zero
   dm_float negativeZero = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U);
   dm_float positiveOne  = DM_FLOAT_PACK(0, 0, 1000000U);
   dm_float negativeOne  = DM_FLOAT_PACK(1, 0, 1000000U);
   dm_float positiveInf  = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY); // Special exponent, ones significand : infinity
   dm_float negativeInf  = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float nan          = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 1U); // Special exponent, any other significand : NaN
   dm_float negativenan  = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 1U);

   EXPECT_EQ(0, dm_float_isinf(positiveZero));
   EXPECT_EQ(0, dm_float_isinf(negativeZero));
   EXPECT_EQ(0, dm_float_isinf(positiveOne));
   EXPECT_EQ(0, dm_float_isinf(negativeOne));
   EXPECT_EQ(1, dm_float_isinf(positiveInf));
   EXPECT_EQ(1, dm_float_isinf(negativeInf));
   EXPECT_EQ(0, dm_float_isinf(nan));

   EXPECT_EQ(0, dm_float_isnan(positiveZero));
   EXPECT_EQ(0, dm_float_isnan(negativeZero));
   EXPECT_EQ(0, dm_float_isnan(positiveOne));
   EXPECT_EQ(0, dm_float_isnan(negativeOne));
   EXPECT_EQ(0, dm_float_isnan(positiveInf));
   EXPECT_EQ(0, dm_float_isnan(negativeInf));
   EXPECT_EQ(1, dm_float_isnan(nan));

   EXPECT_EQ(0, dm_float_signbit(positiveZero));
   EXPECT_EQ(1, dm_float_signbit(negativeZero));
   EXPECT_EQ(0, dm_float_signbit(positiveOne));
   EXPECT_EQ(1, dm_float_signbit(negativeOne));
   EXPECT_EQ(0, dm_float_signbit(positiveInf));
   EXPECT_EQ(1, dm_float_signbit(negativeInf));
   EXPECT_EQ(0, dm_float_signbit(nan));
   EXPECT_EQ(1, dm_float_signbit(negativenan)); // This is the only time we test this. All other functions will ignore this case.

   EXPECT_EQ(1, dm_float_iszero(positiveZero));
   EXPECT_EQ(1, dm_float_iszero(negativeZero));
   EXPECT_EQ(0, dm_float_iszero(positiveOne));
   EXPECT_EQ(0, dm_float_iszero(negativeOne));
   EXPECT_EQ(0, dm_float_iszero(positiveInf));
   EXPECT_EQ(0, dm_float_iszero(negativeInf));
   EXPECT_EQ(0, dm_float_iszero(nan));

   EXPECT_EQ(FP_ZERO, dm_float_fpclassify(positiveZero));
   EXPECT_EQ(FP_ZERO, dm_float_fpclassify(negativeZero));
   EXPECT_EQ(FP_NORMAL, dm_float_fpclassify(positiveOne));
   EXPECT_EQ(FP_NORMAL, dm_float_fpclassify(negativeOne));
   EXPECT_EQ(FP_INFINITE, dm_float_fpclassify(positiveInf));
   EXPECT_EQ(FP_INFINITE, dm_float_fpclassify(negativeInf));
   EXPECT_EQ(FP_NAN, dm_float_fpclassify(nan));
   EXPECT_EQ(FP_NAN, dm_float_fpclassify(negativenan));
 }

TEST(DMFloatTest, testNegate)
 {
   dm_float positiveOne  = DM_FLOAT_PACK(0, 0, 1000000U);
   dm_float negativeOne  = DM_FLOAT_PACK(1, 0, 1000000U);
   dm_float positiveInf  = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float negativeInf  = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float positiveZero = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U);
   dm_float negativeZero = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U);

   EXPECT_EQ(negativeOne, dm_float_neg(positiveOne));
   EXPECT_EQ(positiveInf, dm_float_neg(negativeInf));
   EXPECT_EQ(negativeZero, dm_float_neg(positiveZero));
 }

TEST(DMFloatTest, testAdds)
 {
   dm_float positiveZero = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U);
   dm_float negativeZero = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U);
   dm_float positiveInf  = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float negativeInf  = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float nan1         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 1U);
   dm_float nan2         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 2U);
   dm_float positiveOne  = DM_FLOAT_PACK(0, 0, 1000000U);
   dm_float negativeOne  = DM_FLOAT_PACK(1, 0, 1000000U);
   dm_float positiveTwo  = DM_FLOAT_PACK(0, 0, 2000000U);
   dm_float negativeTwo  = DM_FLOAT_PACK(1, 0, 2000000U);
   dm_float positiveTen  = DM_FLOAT_PACK(0, 1, 1000000U);
   dm_float negativeTen  = DM_FLOAT_PACK(1, 1, 1000000U);

   EXPECT_EQ(nan1, dm_float_add(nan1, positiveOne));
   EXPECT_EQ(nan1, dm_float_add(positiveOne, nan1));
   EXPECT_EQ(nan2, dm_float_add(nan2, nan1));

   EXPECT_EQ(positiveInf, dm_float_add(positiveInf, positiveInf));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 255), dm_float_add(positiveInf, negativeInf));
   EXPECT_EQ(positiveInf, dm_float_add(positiveInf, positiveOne));
   EXPECT_EQ(positiveInf, dm_float_add(positiveOne, positiveInf));

   EXPECT_EQ(positiveZero, dm_float_add(positiveZero, positiveZero));
   EXPECT_EQ(positiveZero, dm_float_add(positiveZero, negativeZero));
   EXPECT_EQ(positiveZero, dm_float_add(negativeZero, positiveZero));
   EXPECT_EQ(negativeZero, dm_float_add_r(positiveZero, negativeZero, DM_FE_DOWNWARD));
   EXPECT_EQ(positiveOne, dm_float_add(positiveZero, positiveOne));
   EXPECT_EQ(positiveOne, dm_float_add(positiveOne, positiveZero));
   EXPECT_EQ(negativeZero, dm_float_add(negativeZero, negativeZero));

   dm_fesetround(DM_FE_TONEAREST);

   EXPECT_EQ(positiveTwo, dm_float_add(positiveOne, positiveOne)); // Basic cases
   EXPECT_EQ(negativeTwo, dm_float_add(negativeOne, negativeOne));
   EXPECT_EQ(negativeOne, dm_float_add(positiveOne, negativeTwo));
   EXPECT_EQ(positiveOne, dm_float_add(negativeOne, positiveTwo));
   EXPECT_EQ(positiveZero, dm_float_add(positiveOne, negativeOne)); // Cancel to zero
   EXPECT_EQ(negativeZero, dm_float_add_r(positiveOne, negativeOne, DM_FE_DOWNWARD)); // Cancel to zero w/ rounding
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 1100000U), dm_float_add(positiveOne, positiveTen)); // Handles offset of exponents correctly
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 1100000U), dm_float_add(positiveTen, positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(1, 1, 1100000U), dm_float_add(negativeOne, negativeTen));
   EXPECT_EQ(DM_FLOAT_PACK(1, 1, 1100000U), dm_float_add(negativeTen, negativeOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 9000000U), dm_float_add(negativeOne, positiveTen)); // Result needs exponent - 1 : result in bounds path
      // Result needs exponent - 1 : result > MAX_SIGNIFICAND path
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 9900000U), dm_float_add(negativeOne, DM_FLOAT_PACK(0, 2, 1000000U))); // 100 - 1
      // Result needs exponent + 1
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 1800000U), dm_float_add(DM_FLOAT_PACK(0, 0, 9000000U), DM_FLOAT_PACK(0, 0, 9000000U))); // 9 + 9
   EXPECT_EQ(DM_FLOAT_PACK(0, 2, 1000000U), dm_float_add(DM_FLOAT_PACK(0, 1, 9900000U), positiveOne)); // 99 + 1
   EXPECT_EQ(DM_FLOAT_PACK(0, 2, 1000000U), dm_float_add(DM_FLOAT_PACK(0, 1, 9990000U), DM_FLOAT_PACK(0, -1, 1000000U))); // 99.9 + .1
      // Catastrophic cancellation
   EXPECT_EQ(positiveOne, dm_float_add(DM_FLOAT_PACK(0, 1, 2000000U), DM_FLOAT_PACK(1, 1, 1900000U))); // 20 - 19
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 1000000U), dm_float_add(DM_FLOAT_PACK(0, 1, 2000000U), DM_FLOAT_PACK(1, 1, 1990000U))); // 20.0 - 19.9
      // Result flushes to zero
   EXPECT_EQ(positiveZero, dm_float_add(DM_FLOAT_PACK(0, -63, 2000000U), DM_FLOAT_PACK(1, -63, 1900000U)));
   EXPECT_EQ(negativeZero, dm_float_add(DM_FLOAT_PACK(1, -63, 2000000U), DM_FLOAT_PACK(0, -63, 1900000U)));
      // Result flushes to infinity
   EXPECT_EQ(positiveInf, dm_float_add(DM_FLOAT_PACK(0, 63, 9000000U), DM_FLOAT_PACK(0, 63, 9000000U)));
   EXPECT_EQ(negativeInf, dm_float_add(DM_FLOAT_PACK(1, 63, 9000000U), DM_FLOAT_PACK(1, 63, 9000000U)));
      // Rebound with rounding : pre-check
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999990U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -6, 1000000U)));
   EXPECT_EQ(positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(0, -7, 1000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -7, 1000000U)));
      // Rebound with rounding
   EXPECT_EQ(positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -8, 1000000U)));
   EXPECT_EQ(positiveOne, dm_float_add(DM_FLOAT_PACK(1, -8, 1000000U), positiveOne));
      // No rebound
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add_r(positiveOne, DM_FLOAT_PACK(1, -8, 1000000U), DM_FE_TOWARDZERO));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add_r(DM_FLOAT_PACK(1, -8, 1000000U), positiveOne, DM_FE_TOWARDZERO));
      // Final Case : Add w/ round w/ no overflow
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1000001U), dm_float_add(positiveOne, DM_FLOAT_PACK(0, -7, 9000000U)));
      // Pathological case
   EXPECT_EQ(DM_FLOAT_PACK(0, 8, 1000000U), dm_float_add(DM_FLOAT_PACK(0, 7, 9999999U), DM_FLOAT_PACK(0, 0, 9000000U)));
      // Run up the flagpole.
   EXPECT_EQ(DM_FLOAT_PACK(0,  2, 1010000U), dm_float_add(DM_FLOAT_PACK(0,  2, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0,  3, 1001000U), dm_float_add(DM_FLOAT_PACK(0,  3, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0,  4, 1000100U), dm_float_add(DM_FLOAT_PACK(0,  4, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0,  5, 1000010U), dm_float_add(DM_FLOAT_PACK(0,  5, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0,  6, 1000001U), dm_float_add(DM_FLOAT_PACK(0,  6, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0,  7, 1000000U), dm_float_add(DM_FLOAT_PACK(0,  7, 1000000U), positiveOne));


      // Pathological Tests for correct rounding
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9000000U), dm_float_add(DM_FLOAT_PACK(1, -1, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9900000U), dm_float_add(DM_FLOAT_PACK(1, -2, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9990000U), dm_float_add(DM_FLOAT_PACK(1, -3, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999000U), dm_float_add(DM_FLOAT_PACK(1, -4, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999900U), dm_float_add(DM_FLOAT_PACK(1, -5, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999990U), dm_float_add(DM_FLOAT_PACK(1, -6, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(DM_FLOAT_PACK(1, -7, 1000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -8, 1000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 1000000U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 5000001U), dm_float_add(DM_FLOAT_PACK(1, -1, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(DM_FLOAT_PACK(1, -2, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(DM_FLOAT_PACK(1, -3, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(DM_FLOAT_PACK(1, -4, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(DM_FLOAT_PACK(1, -5, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(DM_FLOAT_PACK(1, -6, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(DM_FLOAT_PACK(1, -7, 4999999U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -8, 4999999U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 4999999U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 5000000U), dm_float_add(DM_FLOAT_PACK(1, -1, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(DM_FLOAT_PACK(1, -2, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(DM_FLOAT_PACK(1, -3, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(DM_FLOAT_PACK(1, -4, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(DM_FLOAT_PACK(1, -5, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(DM_FLOAT_PACK(1, -6, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(DM_FLOAT_PACK(1, -7, 5000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -8, 5000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 5000000U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 4999999U), dm_float_add(DM_FLOAT_PACK(1, -1, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(DM_FLOAT_PACK(1, -2, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(DM_FLOAT_PACK(1, -3, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(DM_FLOAT_PACK(1, -4, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(DM_FLOAT_PACK(1, -5, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(DM_FLOAT_PACK(1, -6, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(DM_FLOAT_PACK(1, -7, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(DM_FLOAT_PACK(1, -8, 5000001U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 5000001U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 1000000U), dm_float_add(DM_FLOAT_PACK(1, -1, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9100000U), dm_float_add(DM_FLOAT_PACK(1, -2, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9910000U), dm_float_add(DM_FLOAT_PACK(1, -3, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9991000U), dm_float_add(DM_FLOAT_PACK(1, -4, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999100U), dm_float_add(DM_FLOAT_PACK(1, -5, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999910U), dm_float_add(DM_FLOAT_PACK(1, -6, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999991U), dm_float_add(DM_FLOAT_PACK(1, -7, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(DM_FLOAT_PACK(1, -8, 9000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 9000000U), positiveOne));

   dm_fesetround(DM_FE_TONEARESTODD);

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9000000U), dm_float_add(DM_FLOAT_PACK(1, -1, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9900000U), dm_float_add(DM_FLOAT_PACK(1, -2, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9990000U), dm_float_add(DM_FLOAT_PACK(1, -3, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999000U), dm_float_add(DM_FLOAT_PACK(1, -4, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999900U), dm_float_add(DM_FLOAT_PACK(1, -5, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999990U), dm_float_add(DM_FLOAT_PACK(1, -6, 1000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(DM_FLOAT_PACK(1, -7, 1000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -8, 1000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 1000000U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 5000001U), dm_float_add(DM_FLOAT_PACK(1, -1, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(DM_FLOAT_PACK(1, -2, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(DM_FLOAT_PACK(1, -3, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(DM_FLOAT_PACK(1, -4, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(DM_FLOAT_PACK(1, -5, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(DM_FLOAT_PACK(1, -6, 4999999U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(DM_FLOAT_PACK(1, -7, 4999999U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -8, 4999999U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 4999999U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 5000000U), dm_float_add(DM_FLOAT_PACK(1, -1, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(DM_FLOAT_PACK(1, -2, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(DM_FLOAT_PACK(1, -3, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(DM_FLOAT_PACK(1, -4, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(DM_FLOAT_PACK(1, -5, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(DM_FLOAT_PACK(1, -6, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(DM_FLOAT_PACK(1, -7, 5000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(DM_FLOAT_PACK(1, -8, 5000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 5000000U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 4999999U), dm_float_add(DM_FLOAT_PACK(1, -1, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(DM_FLOAT_PACK(1, -2, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(DM_FLOAT_PACK(1, -3, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(DM_FLOAT_PACK(1, -4, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(DM_FLOAT_PACK(1, -5, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(DM_FLOAT_PACK(1, -6, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(DM_FLOAT_PACK(1, -7, 5000001U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(DM_FLOAT_PACK(1, -8, 5000001U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 5000001U), positiveOne));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 1000000U), dm_float_add(DM_FLOAT_PACK(1, -1, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9100000U), dm_float_add(DM_FLOAT_PACK(1, -2, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9910000U), dm_float_add(DM_FLOAT_PACK(1, -3, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9991000U), dm_float_add(DM_FLOAT_PACK(1, -4, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999100U), dm_float_add(DM_FLOAT_PACK(1, -5, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999910U), dm_float_add(DM_FLOAT_PACK(1, -6, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999991U), dm_float_add(DM_FLOAT_PACK(1, -7, 9000000U), positiveOne));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(DM_FLOAT_PACK(1, -8, 9000000U), positiveOne));
   EXPECT_EQ(                   positiveOne, dm_float_add(DM_FLOAT_PACK(1, -9, 9000000U), positiveOne));

   dm_fesetround(DM_FE_TONEAREST);

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9000000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -1, 1000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9900000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -2, 1000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9990000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -3, 1000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -4, 1000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999900U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -5, 1000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999990U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -6, 1000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -7, 1000000U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -8, 1000000U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -9, 1000000U)));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 5000001U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -1, 4999999U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -2, 4999999U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -3, 4999999U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -4, 4999999U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -5, 4999999U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -6, 4999999U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -7, 4999999U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -8, 4999999U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -9, 4999999U)));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 5000000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -1, 5000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -2, 5000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -3, 5000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -4, 5000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -5, 5000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -6, 5000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -7, 5000000U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -8, 5000000U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -9, 5000000U)));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 4999999U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -1, 5000001U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9500000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -2, 5000001U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9950000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -3, 5000001U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9995000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -4, 5000001U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999500U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -5, 5000001U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999950U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -6, 5000001U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999995U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -7, 5000001U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -8, 5000001U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -9, 5000001U)));

   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 1000000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -1, 9000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9100000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -2, 9000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9910000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -3, 9000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9991000U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -4, 9000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999100U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -5, 9000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999910U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -6, 9000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999991U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -7, 9000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 9999999U), dm_float_add(positiveOne, DM_FLOAT_PACK(1, -8, 9000000U)));
   EXPECT_EQ(                   positiveOne, dm_float_add(positiveOne, DM_FLOAT_PACK(1, -9, 9000000U)));
 }

TEST(DMFloatTest, testSubs)
 {
   dm_float nan1         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 1U);
   dm_float nan2         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 2U);
   dm_float positiveOne  = DM_FLOAT_PACK(0, 0, 1000000U);

      // Every sub routine (He he he...) handles NaNs individually so that the resulting NaN doesn't have its sign mutated.
   EXPECT_EQ(nan1, dm_float_sub(nan1, positiveOne));
   EXPECT_EQ(nan1, dm_float_sub(positiveOne, nan1));
   EXPECT_EQ(nan2, dm_float_sub(nan2, nan1));

   EXPECT_EQ(nan1, dm_float_sub_r(nan1, positiveOne, DM_FE_DOWNWARD));
   EXPECT_EQ(nan1, dm_float_sub_r(positiveOne, nan1, DM_FE_DOWNWARD));
   EXPECT_EQ(nan2, dm_float_sub_r(nan2, nan1, DM_FE_DOWNWARD));

      // With the code in mind, any subtraction here will do.
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 4000000U), dm_float_sub(DM_FLOAT_PACK(0, 1, 1000000U), DM_FLOAT_PACK(0, 0, 6000000U))); // 10 - 6
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 5000000U), dm_float_sub_r(DM_FLOAT_PACK(0, 1, 1100000U), DM_FLOAT_PACK(0, 0, 6000000U), DM_FE_DOWNWARD)); // 11 - 6
 }

TEST(DMFloatTest, testMuls)
 {
   dm_float positiveZero = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U);
   dm_float negativeZero = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U);
   dm_float positiveInf  = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float negativeInf  = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float nan1         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 1U);
   dm_float nan2         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 2U);
   dm_float nan255       = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 255U);
   dm_float nnan255      = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 255U);
   dm_float positiveOne  = DM_FLOAT_PACK(0, 0, 1000000U);
   dm_float negativeOne  = DM_FLOAT_PACK(1, 0, 1000000U);

      // Nans
   EXPECT_EQ(nan1, dm_float_mul(nan1, positiveOne));
   EXPECT_EQ(nan1, dm_float_mul(positiveOne, nan1));
   EXPECT_EQ(nan2, dm_float_mul(nan2, nan1));

      // Inf * 0 = Nan
   EXPECT_EQ(nan255, dm_float_mul(positiveInf, positiveZero));
   EXPECT_EQ(nan255, dm_float_mul(positiveZero, positiveInf));
   EXPECT_EQ(nnan255, dm_float_mul(negativeInf, positiveZero));
   EXPECT_EQ(nnan255, dm_float_mul(positiveZero, negativeInf));
   EXPECT_EQ(nnan255, dm_float_mul(positiveInf, negativeZero));
   EXPECT_EQ(nnan255, dm_float_mul(negativeZero, positiveInf));
   EXPECT_EQ(nan255, dm_float_mul(negativeInf, negativeZero));
   EXPECT_EQ(nan255, dm_float_mul(negativeZero, negativeInf));

      // Infinities
   EXPECT_EQ(positiveInf, dm_float_mul(positiveInf, positiveOne));
   EXPECT_EQ(positiveInf, dm_float_mul(positiveOne, positiveInf));
   EXPECT_EQ(negativeInf, dm_float_mul(negativeInf, positiveOne));
   EXPECT_EQ(negativeInf, dm_float_mul(positiveOne, negativeInf));

      // Zeros
   EXPECT_EQ(positiveZero, dm_float_mul(positiveZero, positiveOne));
   EXPECT_EQ(negativeZero, dm_float_mul(negativeOne, positiveZero));
   EXPECT_EQ(negativeZero, dm_float_mul(negativeZero, positiveOne));
   EXPECT_EQ(positiveZero, dm_float_mul(negativeOne, negativeZero));

      // Overflows
   EXPECT_EQ(positiveInf, dm_float_mul(DM_FLOAT_PACK(0, 63, 9000000U), DM_FLOAT_PACK(0, 63, 9000000U)));
   EXPECT_EQ(positiveInf, dm_float_mul(DM_FLOAT_PACK(1, 63, 9000000U), DM_FLOAT_PACK(1, 63, 9000000U)));
   EXPECT_EQ(negativeInf, dm_float_mul(DM_FLOAT_PACK(1, 63, 9000000U), DM_FLOAT_PACK(0, 63, 9000000U)));
   EXPECT_EQ(negativeInf, dm_float_mul(DM_FLOAT_PACK(0, 63, 9000000U), DM_FLOAT_PACK(1, 63, 9000000U)));

      // Underflows
   EXPECT_EQ(positiveZero, dm_float_mul(DM_FLOAT_PACK(0, -63, 9000000U), DM_FLOAT_PACK(0, -10, 9000000U)));
   EXPECT_EQ(positiveZero, dm_float_mul(DM_FLOAT_PACK(1, -63, 9000000U), DM_FLOAT_PACK(1, -10, 9000000U)));
   EXPECT_EQ(negativeZero, dm_float_mul(DM_FLOAT_PACK(1, -63, 9000000U), DM_FLOAT_PACK(0, -10, 9000000U)));
   EXPECT_EQ(negativeZero, dm_float_mul(DM_FLOAT_PACK(0, -63, 9000000U), DM_FLOAT_PACK(1, -10, 9000000U)));

      // Standard cases, no rounding
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 6000000U), dm_float_mul(DM_FLOAT_PACK(0, 0, 2000000U), DM_FLOAT_PACK(0, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 1800000U), dm_float_mul(DM_FLOAT_PACK(0, 0, 6000000U), DM_FLOAT_PACK(0, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 6000000U), dm_float_mul(DM_FLOAT_PACK(1, 0, 2000000U), DM_FLOAT_PACK(1, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(1, 0, 6000000U), dm_float_mul(DM_FLOAT_PACK(0, 0, 2000000U), DM_FLOAT_PACK(1, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 9999998U), dm_float_mul(DM_FLOAT_PACK(0, 0, 9999999U), DM_FLOAT_PACK(0, 0, 9999999U)));

      // Flushes to infinity
   EXPECT_EQ(positiveInf, dm_float_mul(DM_FLOAT_PACK(0, 31, 2000000U), DM_FLOAT_PACK(0, 32, 5000000U)));
   EXPECT_EQ(negativeInf, dm_float_mul(DM_FLOAT_PACK(1, 31, 2000000U), DM_FLOAT_PACK(0, 32, 5000000U)));

      // Flushes to zero
   EXPECT_EQ(positiveZero, dm_float_mul(DM_FLOAT_PACK(0, -32, 2000000U), DM_FLOAT_PACK(0, -32, 3000000U)));
   EXPECT_EQ(negativeZero, dm_float_mul(DM_FLOAT_PACK(1, -32, 2000000U), DM_FLOAT_PACK(0, -32, 3000000U)));

      // Saved from zero
   EXPECT_EQ(DM_FLOAT_PACK(0, -63, 1200000U), dm_float_mul(DM_FLOAT_PACK(0, -32, 4000000U), DM_FLOAT_PACK(0, -32, 3000000U)));

      // Standard cases, rounding
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 2000000U), dm_float_mul(DM_FLOAT_PACK(0, 0, 2000000U), DM_FLOAT_PACK(0, 0, 9999999U)));
 }

TEST(DMFloatTest, testDivs)
 {
   dm_float positiveZero = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U);
   dm_float negativeZero = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U);
   dm_float positiveInf  = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float negativeInf  = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY);
   dm_float nan1         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 1U);
   dm_float nan2         = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 2U);
   dm_float nan255       = DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 255U);
   dm_float nnan255      = DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 255U);
   dm_float positiveOne  = DM_FLOAT_PACK(0, 0, 1000000U);
   dm_float negativeOne  = DM_FLOAT_PACK(1, 0, 1000000U);

      // Nans
   EXPECT_EQ(nan1, dm_float_div(nan1, positiveOne));
   EXPECT_EQ(nan1, dm_float_div(positiveOne, nan1));
   EXPECT_EQ(nan2, dm_float_div(nan2, nan1));

      // Inf / Inf = Nan, 0 / 0 = Nan
   EXPECT_EQ(nan255, dm_float_div(positiveInf, positiveInf));
   EXPECT_EQ(nan255, dm_float_div(negativeInf, negativeInf));
   EXPECT_EQ(nnan255, dm_float_div(negativeZero, positiveZero));
   EXPECT_EQ(nnan255, dm_float_div(positiveZero, negativeZero));

      // Infinities
   EXPECT_EQ(positiveInf, dm_float_div(positiveInf, positiveOne));
   EXPECT_EQ(positiveInf, dm_float_div(positiveOne, positiveZero));
   EXPECT_EQ(negativeInf, dm_float_div(negativeInf, positiveOne));
   EXPECT_EQ(negativeInf, dm_float_div(positiveOne, negativeZero));

      // Zeros
   EXPECT_EQ(positiveZero, dm_float_div(positiveOne, positiveInf));
   EXPECT_EQ(positiveZero, dm_float_div(positiveZero, positiveOne));
   EXPECT_EQ(negativeZero, dm_float_div(negativeOne, positiveInf));
   EXPECT_EQ(negativeZero, dm_float_div(negativeZero, positiveOne));

      // Overflows
   EXPECT_EQ(positiveInf, dm_float_div(DM_FLOAT_PACK(0, 63, 9000000U), DM_FLOAT_PACK(0, -63, 9000000U)));
   EXPECT_EQ(positiveInf, dm_float_div(DM_FLOAT_PACK(1, 63, 9000000U), DM_FLOAT_PACK(1, -63, 9000000U)));
   EXPECT_EQ(negativeInf, dm_float_div(DM_FLOAT_PACK(1, 63, 9000000U), DM_FLOAT_PACK(0, -63, 9000000U)));
   EXPECT_EQ(negativeInf, dm_float_div(DM_FLOAT_PACK(0, 63, 9000000U), DM_FLOAT_PACK(1, -63, 9000000U)));

      // Underflows
   EXPECT_EQ(positiveZero, dm_float_div(DM_FLOAT_PACK(0, -63, 9000000U), DM_FLOAT_PACK(0, 10, 9000000U)));
   EXPECT_EQ(positiveZero, dm_float_div(DM_FLOAT_PACK(1, -63, 9000000U), DM_FLOAT_PACK(1, 10, 9000000U)));
   EXPECT_EQ(negativeZero, dm_float_div(DM_FLOAT_PACK(1, -63, 9000000U), DM_FLOAT_PACK(0, 10, 9000000U)));
   EXPECT_EQ(negativeZero, dm_float_div(DM_FLOAT_PACK(0, -63, 9000000U), DM_FLOAT_PACK(1, 10, 9000000U)));

      // Standard cases, no rounding
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 2000000U), dm_float_div(DM_FLOAT_PACK(0, 0, 6000000U), DM_FLOAT_PACK(0, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 4000000U), dm_float_div(DM_FLOAT_PACK(0, 0, 1200000U), DM_FLOAT_PACK(0, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 2000000U), dm_float_div(DM_FLOAT_PACK(1, 0, 6000000U), DM_FLOAT_PACK(1, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(1, 0, 2000000U), dm_float_div(DM_FLOAT_PACK(0, 0, 6000000U), DM_FLOAT_PACK(1, 0, 3000000U)));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 9999999U), dm_float_div(DM_FLOAT_PACK(0, 0, 9999999U), DM_FLOAT_PACK(0, 0, 1000000U)));

      // Flushes to infinity
   EXPECT_EQ(positiveInf, dm_float_div(DM_FLOAT_PACK(0, 32, 9000000U), DM_FLOAT_PACK(0, -32, 3000000U)));
   EXPECT_EQ(negativeInf, dm_float_div(DM_FLOAT_PACK(1, 32, 9000000U), DM_FLOAT_PACK(0, -32, 3000000U)));

      // Flushes to zero
   EXPECT_EQ(positiveZero, dm_float_div(DM_FLOAT_PACK(0, -31, 1200000U), DM_FLOAT_PACK(0, 32, 3000000U)));
   EXPECT_EQ(negativeZero, dm_float_div(DM_FLOAT_PACK(1, -31, 1200000U), DM_FLOAT_PACK(0, 32, 3000000U)));

      // Saved from infinity
   EXPECT_EQ(DM_FLOAT_PACK(0, 63, 4000000U), dm_float_div(DM_FLOAT_PACK(0, 32, 1200000U), DM_FLOAT_PACK(0, -32, 3000000U)));

      // Standard cases, rounding
   EXPECT_EQ(DM_FLOAT_PACK(0, -1, 6666667U), dm_float_div(DM_FLOAT_PACK(0, 0, 2000000U), DM_FLOAT_PACK(0, 0, 3000000U)));
 }

TEST(DMFloatTest, testStringConversions)
 {
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 5000000U), dm_float_fromstring("5.0"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234567U), dm_float_fromstring("12345671"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234568U), dm_float_fromstring("12345679"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234568U), dm_float_fromstring("12345685"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234568U), dm_float_fromstring("12345675"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234566U), dm_float_fromstring("12345665"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 9, 1234567U), dm_float_fromstring("1234566501"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234567U), dm_float_fromstring("12345670"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 8, 1234567U), dm_float_fromstring("123456700"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 8, 1234567U), dm_float_fromstring("123456701"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 8, 1234567U), dm_float_fromstring("123456711"));
   EXPECT_EQ(DM_FLOAT_PACK(1, 0, 5000000U), dm_float_fromstring("-5.0"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234567U), dm_float_fromstring("1.2345671"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234568U), dm_float_fromstring("1.2345679"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234568U), dm_float_fromstring("1.2345685"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234568U), dm_float_fromstring("1.2345675"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234566U), dm_float_fromstring("1.2345665"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234566U), dm_float_fromstring("1.23456650"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234567U), dm_float_fromstring("1.234566501"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234567U), dm_float_fromstring("1.2345670"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234567U), dm_float_fromstring("1.23456700"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234567U), dm_float_fromstring("1.23456701"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 1234567U), dm_float_fromstring("1.23456711"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 1, 1000000U), dm_float_fromstring("9.9999999"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 10, 5000000U), dm_float_fromstring("5e10"));
   EXPECT_EQ(DM_FLOAT_PACK(0, 10, 5000000U), dm_float_fromstring("5e+10"));
   EXPECT_EQ(DM_FLOAT_PACK(0, -10, 5000000U), dm_float_fromstring("5e-10"));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY), dm_float_fromstring("5e100"));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U), dm_float_fromstring("5e-100"));
   EXPECT_EQ(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY), dm_float_fromstring("-5e100"));
   EXPECT_EQ(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U), dm_float_fromstring("-5e-100"));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY), dm_float_fromstring("5e200"));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U), dm_float_fromstring("5e-200"));
   EXPECT_EQ(DM_FLOAT_PACK(0, -2, 1000000U), dm_float_fromstring("0.01"));
   EXPECT_EQ(DM_FLOAT_PACK(0, -3, 1000000U), dm_float_fromstring("0.001"));
   EXPECT_EQ(DM_FLOAT_PACK(0, -2, 1000000U), dm_float_fromstring("0,01"));
   EXPECT_EQ(DM_FLOAT_PACK(0, -3, 1000000U), dm_float_fromstring("0,001"));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U), dm_float_fromstring("0e1000000"));
 }

#define DM_DOUBLE_PACK_ALT(sign, exponent, significand) ((((uint64_t)(exponent)) << 54) | (significand) | ((sign) ? 0x20000000000000ULL : 0))

TEST(DMFloatTest, testDoubleConversions)
 {
   EXPECT_EQ(DM_FLOAT_PACK(0, 0, 5000000U), dm_float_from_dm_double(dm_double_fromstring("5.0")));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234567U), dm_float_from_dm_double(dm_double_fromstring("12345671")));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234568U), dm_float_from_dm_double(dm_double_fromstring("12345679")));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234568U), dm_float_from_dm_double(dm_double_fromstring("12345685")));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234568U), dm_float_from_dm_double(dm_double_fromstring("12345675")));
   EXPECT_EQ(DM_FLOAT_PACK(0, 7, 1234566U), dm_float_from_dm_double(dm_double_fromstring("12345665")));
   EXPECT_EQ(DM_FLOAT_PACK(1, 7, 1234566U), dm_float_from_dm_double(dm_double_fromstring("-12345665")));
   EXPECT_EQ(DM_FLOAT_PACK(0, 8, 1000000U), dm_float_from_dm_double(dm_double_fromstring("99999999")));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U), dm_float_from_dm_double(dm_double_fromstring("5e-75")));
   EXPECT_EQ(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U), dm_float_from_dm_double(dm_double_fromstring("-5e-75")));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY), dm_float_from_dm_double(dm_double_fromstring("5e200")));
   EXPECT_EQ(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY), dm_float_from_dm_double(dm_double_fromstring("-5e100")));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 0U), dm_float_from_dm_double(dm_double_fromstring("5e-1000")));
   EXPECT_EQ(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 0U), dm_float_from_dm_double(dm_double_fromstring("-5e-1000")));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY), dm_float_from_dm_double(dm_double_fromstring("5e1000")));
   EXPECT_EQ(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY), dm_float_from_dm_double(dm_double_fromstring("-5e1000")));
   EXPECT_EQ(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 255), dm_float_from_dm_double(DM_DOUBLE_PACK_ALT(0, -512, DM_INFINITY)));
   EXPECT_EQ(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 255), dm_float_from_dm_double(DM_DOUBLE_PACK_ALT(1, -512, DM_INFINITY)));

   EXPECT_EQ(dm_double_fromstring("0"), dm_double_from_dm_float(dm_float_fromstring("0")));
   EXPECT_EQ(dm_double_fromstring("1234567"), dm_double_from_dm_float(dm_float_fromstring("1234567")));
   EXPECT_EQ(dm_double_fromstring("-1234568"), dm_double_from_dm_float(dm_float_fromstring("-1234568")));
   EXPECT_EQ(DM_DOUBLE_PACK_ALT(0, -512, 0x1FFFFFFFFFFFFFULL), dm_double_from_dm_float(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, DM_INFINITY)));
   EXPECT_EQ(DM_DOUBLE_PACK_ALT(1, -512, 0x1FFFFFFFFFFFFFULL), dm_double_from_dm_float(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, DM_INFINITY)));
   EXPECT_EQ(DM_DOUBLE_PACK_ALT(0, -512, 255), dm_double_from_dm_float(DM_FLOAT_PACK(0, SPECIAL_EXPONENT, 255)));
   EXPECT_EQ(DM_DOUBLE_PACK_ALT(1, -512, 255), dm_double_from_dm_float(DM_FLOAT_PACK(1, SPECIAL_EXPONENT, 255)));
 }
