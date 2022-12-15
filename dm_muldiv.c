/*
BSD 3-Clause License

Copyright (c) 2022, Thomas DiModica
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
#include "dm_muldiv.h"

void dm_muldiv_loadFrom(dm_muldiv_t dest, uint64_t source)
 {
#ifndef DM_NO_128_BIT_TYPE
   *dest = source;
#else // DM_NO_128_BIT_TYPE
   dest[3] = 0;
   dest[2] = 0;
   dest[1] = source >> 32;
   dest[0] = source;
#endif
 }

void dm_muldiv_mulBy(dm_muldiv_t dest, uint64_t rhs)
 {
#ifndef DM_NO_128_BIT_TYPE
   *dest *= rhs;
#else // DM_NO_128_BIT_TYPE
/*
   A  B  C  D
         X  Y
   ----------
  AY BY CY DY
  BX CX DX

   While this looks suspicious, it can't overflow:
      9 + 9 + 9 * 9 = 99
      (BASE - 1) + (BASE - 1) + (BASE - 1) * (BASE - 1)
      (2 * BASE - 2) + (BASE ^ 2 - 2 * BASE + 1)
      BASE ^ 2 - 1
   We take the high half of the previous low product,
      plus the low half of the current high product,
      plus the current low product.
*/
   uint64_t low = rhs & 0xFFFFFFFF;
   uint64_t high = rhs >> 32;
   uint64_t temp1 = low * dest[0];
   uint64_t temp2 = high * dest[0];
   dest[0] = temp1;
   temp1 >>=32;
   temp1 = temp1 + (temp2 & 0xFFFFFFFF) + low * dest[1];
   temp2 >>= 32;
   temp2 = temp2 + high * dest[1];
   dest[1] = temp1;
   temp1 >>=32;
   temp1 = temp1 + (temp2 & 0xFFFFFFFF) + low * dest[2];
   temp2 >>= 32;
   temp2 = temp2 + high * dest[2];
   dest[2] = temp1;
   temp1 >>=32;
   temp1 = temp1 + (temp2 & 0xFFFFFFFF) + low * dest[3];
   dest[3] = temp1;
#endif
 }

#ifdef DM_NO_128_BIT_TYPE
void dm_muldiv_copy(dm_muldiv_t dest, dm_muldiv_t src)
 {
   dest[0] = src[0];
   dest[1] = src[1];
   dest[2] = src[2];
   dest[3] = src[3];
 }
#ifdef DM_USE_SLOW_BINARY
void dm_muldiv_shl1(dm_muldiv_t arg)
 {
   uint64_t temp = ((uint64_t)arg[0]) << 1;
   arg[0] = temp;
   temp >>= 32;
   temp = temp | ((uint64_t)arg[1]) << 1;
   arg[1] = temp;
   temp >>= 32;
   temp = temp | ((uint64_t)arg[2]) << 1;
   arg[2] = temp;
   temp >>= 32;
   temp = temp | ((uint64_t)arg[3]) << 1;
   arg[3] = temp;
 }

void dm_muldiv_shr1(dm_muldiv_t arg)
 {
   uint64_t temp = ((uint64_t)arg[3]);
   arg[3] = temp >> 1;
   temp <<= 32;
   temp = temp | ((uint64_t)arg[2]);
   arg[2] = temp >> 1;
   temp <<= 32;
   temp = temp | ((uint64_t)arg[1]);
   arg[1] = temp >> 1;
   temp <<= 32;
   temp = temp | ((uint64_t)arg[0]);
   arg[0] = temp >> 1;
 }
#else /* Use faster word division */
void dm_muldiv_shl(dm_muldiv_t arg, int count)
 {
   if (32 < count)
    {
      arg[3] = arg[2];
      arg[2] = arg[1];
      arg[1] = arg[0];
      arg[0] = 0U;
      count -= 32;
    }
   uint64_t temp = ((uint64_t)arg[0]) << count;
   arg[0] = temp;
   temp >>= 32;
   temp = temp | ((uint64_t)arg[1]) << count;
   arg[1] = temp;
   temp >>= 32;
   temp = temp | ((uint64_t)arg[2]) << count;
   arg[2] = temp;
   temp >>= 32;
   temp = temp | ((uint64_t)arg[3]) << count;
   arg[3] = temp;
 }

void dm_muldiv_shr(dm_muldiv_t arg, int count)
 {
   if (32 < count)
    {
      arg[0] = arg[1];
      arg[1] = arg[2];
      arg[2] = arg[3];
      arg[3] = 0U;
      count -= 32;
    }
   uint64_t temp = ((uint64_t)arg[3]);
   arg[3] = temp >> count;
   temp <<= 32;
   temp = temp | ((uint64_t)arg[2]);
   arg[2] = temp >> count;
   temp <<= 32;
   temp = temp | ((uint64_t)arg[1]);
   arg[1] = temp >> count;
   temp <<= 32;
   temp = temp | ((uint64_t)arg[0]);
   arg[0] = temp >> count;
 }
#endif /* DM_USE_SLOW_BINARY */
#endif /* DM_NO_128_BIT_TYPE */

void dm_muldiv_divBy(dm_muldiv_t lhs, uint64_t rhs, uint64_t* quo, uint64_t* rem)
 {
#ifndef DM_NO_128_BIT_TYPE
   *quo = *lhs / rhs;
   *rem = *lhs % rhs;
#else // DM_NO_128_BIT_TYPE
#ifdef DM_USE_SLOW_BINARY
   dm_muldiv_t d, m, n, q;
   dm_muldiv_loadFrom(d, rhs);
   dm_muldiv_loadFrom(m, 1U);
   dm_muldiv_copy(n, lhs);
   dm_muldiv_loadFrom(q, 0U);
   while (dm_muldiv_less(d, n))
    {
      dm_muldiv_shl1(d);
      dm_muldiv_shl1(m);
    }
   if (dm_muldiv_less(n, d))
    {
      dm_muldiv_shr1(d);
      dm_muldiv_shr1(m);
    }

   while (!dm_muldiv_zero(m))
    {
      dm_muldiv_sub(n, n, d);
      dm_muldiv_add(q, q, m);
      while (dm_muldiv_less(n, d))
       {
         dm_muldiv_shr1(d);
         dm_muldiv_shr1(m);
       }
    }
   dm_muldiv_extract(q, quo);
   dm_muldiv_extract(n, rem);
#else /* Use faster word division */
   if (0 == (lhs[3] | lhs[2])) // Is this a degenerate case?
    {
      uint64_t n = (((uint64_t)lhs[1]) << 32) | lhs[0];
      *quo = n / rhs;
      *rem = n % rhs;
      return;
    }
   dm_muldiv_t n, d, s, t;
   dm_muldiv_copy(n, lhs);
   dm_muldiv_loadFrom(d, rhs);
   uint64_t dig1, dig2;

   int count = 0;
   uint64_t byt = 0xFF00000000000000ULL;
   uint64_t bit = 0x8000000000000000ULL;
   while (0 == (rhs & byt))
    {
      byt >>= 8;
      bit >>= 8;
      count += 8;
    }
   while (0 == (rhs & bit))
    {
      bit >>= 1;
      ++count;
    }

   dm_muldiv_shl(n, count);
   dm_muldiv_shl(d, count);

   s[3] = 0U;
   s[2] = d[1];
   s[1] = d[0];
   s[0] = 0U;

   dm_muldiv_copy(t, s);

   dig1 = ((((uint64_t)n[3]) << 32) | n[2]) / d[1];

   dm_muldiv_mulBy(t, dig1);

   if (dm_muldiv_less(n, t)) // if t > n
    {
      --dig1;
      dm_muldiv_sub(t, t, s);
      if (dm_muldiv_less(n, t)) // According to Knuth, only need to do this twice.
       {
         --dig1;
         dm_muldiv_sub(t, t, s);
       }
    }
   dm_muldiv_sub(n, n, t);

   dm_muldiv_copy(s, d);
   dm_muldiv_copy(t, d);

   dig2 = ((((uint64_t)n[2]) << 32) | n[1]) / d[1];

   dm_muldiv_mulBy(t, dig2);

   if (dm_muldiv_less(n, t))
    {
      --dig2;
      dm_muldiv_sub(t, t, s);
      if (dm_muldiv_less(n, t))
       {
         --dig2;
         dm_muldiv_sub(t, t, s);
       }
    }
   dm_muldiv_sub(n, n, t);

   *quo = (dig1 << 32) | dig2;
   dm_muldiv_shr(n, count);
   dm_muldiv_extract(n, rem);
#endif /* DM_USE_SLOW_BINARY */
#endif /* DM_NO_128_BIT_TYPE */
 }

void dm_muldiv_extract(dm_muldiv_t source, uint64_t* dest)
 {
#ifndef DM_NO_128_BIT_TYPE
   *dest = *source;
#else // DM_NO_128_BIT_TYPE
   *dest = (((uint64_t)source[1]) << 32) | source[0];
#endif
 }


void dm_muldiv_add(dm_muldiv_t dest, dm_muldiv_t lhs, dm_muldiv_t rhs) // dest may alias lhs or rhs.
 {
#ifndef DM_NO_128_BIT_TYPE
   *dest = *lhs + *rhs;
#else // DM_NO_128_BIT_TYPE
   uint64_t temp = ((uint64_t)lhs[0]) + rhs[0];
   dest[0] = temp;
   temp >>= 32;
   temp = temp + lhs[1] + rhs[1];
   dest[1] = temp;
   temp >>= 32;
   temp = temp + lhs[2] + rhs[2];
   dest[2] = temp;
   temp >>= 32;
   temp = temp + lhs[3] + rhs[3];
   dest[3] = temp;
#endif
 }

void dm_muldiv_sub(dm_muldiv_t dest, dm_muldiv_t lhs, dm_muldiv_t rhs) // dest may alias lhs or rhs.
 {
#ifndef DM_NO_128_BIT_TYPE
   *dest = *lhs - *rhs;
#else // DM_NO_128_BIT_TYPE
   uint64_t temp = ((uint64_t)lhs[0]) - rhs[0];
   dest[0] = temp;
   temp >>= 32;
   temp = ((uint64_t)lhs[1]) - rhs[1] - (temp & 1);
   dest[1] = temp;
   temp >>= 32;
   temp = ((uint64_t)lhs[2]) - rhs[2] - (temp & 1);
   dest[2] = temp;
   temp >>= 32;
   temp = ((uint64_t)lhs[3]) - rhs[3] - (temp & 1);
   dest[3] = temp;
#endif
 }

int dm_muldiv_zero(dm_muldiv_t arg)
 {
#ifndef DM_NO_128_BIT_TYPE
   return 0 == *arg;
#else // DM_NO_128_BIT_TYPE
   return 0 == (arg[0] | arg[1] | arg[2] | arg[3]);
#endif
 }

int dm_muldiv_greater(dm_muldiv_t lhs, uint64_t rhs)
 {
#ifndef DM_NO_128_BIT_TYPE
   return *lhs > rhs;
#else // DM_NO_128_BIT_TYPE
   if (0 != (lhs[2] | lhs[3])) return 1;
   return ((((uint64_t)lhs[1]) << 32) | lhs[0]) > rhs;
#endif
 }

int dm_muldiv_less(dm_muldiv_t lhs, dm_muldiv_t rhs)
 {
#ifndef DM_NO_128_BIT_TYPE
   return *lhs < *rhs;
#else // DM_NO_128_BIT_TYPE
   if (lhs[3] != rhs[3]) return lhs[3] < rhs[3];
   if (lhs[2] != rhs[2]) return lhs[2] < rhs[2];
   if (lhs[1] != rhs[1]) return lhs[1] < rhs[1];
   return lhs[0] < rhs[0];
#endif
 }
