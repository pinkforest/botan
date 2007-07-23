/*************************************************
* IDEA Source File                               *
* (C) 1999-2007 The Botan Project                *
*************************************************/

#include <botan/idea.h>
#include <botan/bit_ops.h>

namespace Botan {

namespace {

/*************************************************
* Multiplication modulo 65537                    *
*************************************************/
inline void mul(u16bit& a, u16bit b)
   {
   if(a && b)
      {
      u32bit temp = static_cast<u32bit>(a) * b;
      a = static_cast<u16bit>(temp >> 16);
      b = static_cast<u16bit>(temp & 0xFFFF);
      a = static_cast<u16bit>(b - a + ((b < a) ? 1 : 0));
      }
   else
      a = static_cast<u16bit>(1 - a - b);
   }

}

/*************************************************
* IDEA Encryption                                *
*************************************************/
void IDEA::enc(const byte in[], byte out[]) const
   {
   u16bit X1 = load_be<u16bit>(in, 0);
   u16bit X2 = load_be<u16bit>(in, 1);
   u16bit X3 = load_be<u16bit>(in, 2);
   u16bit X4 = load_be<u16bit>(in, 3);

   for(u32bit j = 0; j != 8; ++j)
      {
      mul(X1, EK[6*j+0]);
      X2 += EK[6*j+1];
      X3 += EK[6*j+2];
      mul(X4, EK[6*j+3]);
      u16bit T0 = X3;
      X3 ^= X1;
      mul(X3, EK[6*j+4]);
      u16bit T1 = X2;
      X2 = static_cast<u16bit>((X2 ^ X4) + X3);
      mul(X2, EK[6*j+5]);
      X3 += X2;
      X1 ^= X2;
      X4 ^= X3;
      X2 ^= T0;
      X3 ^= T1;
      }

   mul(X1, EK[48]); X2 += EK[50]; X3 += EK[49]; mul(X4, EK[51]);

   store_be(out, X1, X3, X2, X4);
   }

/*************************************************
* IDEA Decryption                                *
*************************************************/
void IDEA::dec(const byte in[], byte out[]) const
   {
   u16bit X1 = load_be<u16bit>(in, 0);
   u16bit X2 = load_be<u16bit>(in, 1);
   u16bit X3 = load_be<u16bit>(in, 2);
   u16bit X4 = load_be<u16bit>(in, 3);

   for(u32bit j = 0; j != 8; ++j)
      {
      mul(X1, DK[6*j+0]);
      X2 += DK[6*j+1];
      X3 += DK[6*j+2];
      mul(X4, DK[6*j+3]);
      u16bit T0 = X3;
      X3 ^= X1;
      mul(X3, DK[6*j+4]);
      u16bit T1 = X2;
      X2 = static_cast<u16bit>((X2 ^ X4) + X3);
      mul(X2, DK[6*j+5]);
      X3 += X2;
      X1 ^= X2;
      X4 ^= X3;
      X2 ^= T0;
      X3 ^= T1;
      }

   mul(X1, DK[48]); X2 += DK[50]; X3 += DK[49]; mul(X4, DK[51]);

   store_be(out, X1, X3, X2, X4);
   }

/*************************************************
* Find multiplicative inverses modulo 65537      *
*************************************************/
u16bit IDEA::mul_inv(u16bit x)
   {
   if(x <= 1)
      return x;

   u16bit t0 = static_cast<u16bit>(65537 / x), t1 = 1;
   u16bit y = static_cast<u16bit>(65537 % x);

   while(y != 1)
      {
      u16bit q = static_cast<u16bit>(x / y);
      x %= y;
      t1 += static_cast<u16bit>(q * t0);
      if(x == 1)
         return t1;
      q = static_cast<u16bit>(y / x);
      y %= x;
      t0 += static_cast<u16bit>(q * t1);
      }
   return static_cast<u16bit>(1 - t0);
   }

/*************************************************
* IDEA Key Schedule                              *
*************************************************/
void IDEA::key(const byte key[], u32bit)
   {
   for(u32bit j = 0; j != 8; ++j)
      EK[j] = load_be<u16bit>(key, j);

   for(u32bit j = 1, k = 8, offset = 0; k != 52; j %= 8, ++j, ++k)
      {
      EK[j+7+offset] = static_cast<u16bit>((EK[(j     % 8) + offset] << 9) |
                                           (EK[((j+1) % 8) + offset] >> 7));
      offset += (j == 8) ? 8 : 0;
      }

   DK[51] = mul_inv(EK[3]);
   DK[50] = -EK[2];
   DK[49] = -EK[1];
   DK[48] = mul_inv(EK[0]);

   for(u32bit j = 1, k = 4, counter = 47; j != 8; ++j, k += 6)
      {
      DK[counter--] = EK[k+1];
      DK[counter--] = EK[k];
      DK[counter--] = mul_inv(EK[k+5]);
      DK[counter--] = -EK[k+3];
      DK[counter--] = -EK[k+4];
      DK[counter--] = mul_inv(EK[k+2]);
      }

   DK[5] = EK[47];
   DK[4] = EK[46];
   DK[3] = mul_inv(EK[51]);
   DK[2] = -EK[50];
   DK[1] = -EK[49];
   DK[0] = mul_inv(EK[48]);
   }

}
