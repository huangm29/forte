#ifndef _bitwise_operations_hpp_
#define _bitwise_operations_hpp_

#include "nmmintrin.h"

#define USE_builtin_popcountll 1

// double parity(uint64_t x) { return (x & 1) ? 1.0 : -1.0; }

// double parity(uint64_t x) {
//    x ^= x >> 1;
//    x ^= x >> 2;
//    x = (x & 0x1111111111111111UL) * 0x1111111111111111UL;
//    return (x >> 60) & 1;
//}

uint64_t ui64_bit_count(uint64_t x) {
//    return _mm_popcnt_u64(x);
#ifdef USE_builtin_popcountll
    // optimized version using popcnt
    return __builtin_popcountll(x);
#else
    // version based on bitwise operations
    x = (0x5555555555555555UL & x) + (0x5555555555555555UL & (x >> 1));
    x = (0x3333333333333333UL & x) + (0x3333333333333333UL & (x >> 2));
    x = (0x0f0f0f0f0f0f0f0fUL & x) + (0x0f0f0f0f0f0f0f0fUL & (x >> 4));
    x = (0x00ff00ff00ff00ffUL & x) + (0x00ff00ff00ff00ffUL & (x >> 8));
    x = (0x0000ffff0000ffffUL & x) + (0x0000ffff0000ffffUL & (x >> 16));
    x = (0x00000000ffffffffUL & x) + (0x00000000ffffffffUL & (x >> 32));
    return x;
#endif
    //    x = ((x>>1) & 0x5555555555555555UL) + (x & 0x5555555555555555UL);
    //    x = ((x>>2) & 0x3333333333333333UL) + (x & 0x3333333333333333UL);
    //    x = ((x>>4) + x) & 0x0f0f0f0f0f0f0f0fUL;
    //    x+=x>>8;
    //    x += x>>16;
    //    x += x>>32;
    //    return x & 0xff;

    //    x -= (x>>1) & 0x5555555555555555UL;
    //    x = ((x>>2) & 0x3333333333333333UL) + (x & 0x3333333333333333UL); // 0-4 in 4 bits
    //    x = ((x>>4) + x) & 0x0f0f0f0f0f0f0f0fUL; // 0-8 in 8 bits
    //    x *= 0x0101010101010101UL;
    //    return x>>56;
}

/// Returns the index of the least significant 1-bit of x, or if x is zero, returns ~0.
uint64_t lowest_one_idx(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    // optimized version using builtin functions
    return __builtin_ffsll(x) - 1;
#else
    // version based on bitwise operations
    if (1 >= x)
        return x - 1; // 0 if 1, ~0 if 0
    uint64_t r = 0;
    x &= -x; // isolate lowest bit
    if (x & 0xffffffff00000000UL)
        r += 32;
    if (x & 0xffff0000ffff0000UL)
        r += 16;
    if (x & 0xff00ff00ff00ff00UL)
        r += 8;
    if (x & 0xf0f0f0f0f0f0f0f0UL)
        r += 4;
    if (x & 0xccccccccccccccccUL)
        r += 2;
    if (x & 0xaaaaaaaaaaaaaaaaUL)
        r += 1;
    return r;
#endif
}

uint64_t clear_lowest_one(uint64_t x)
// Return word where the lowest bit set in x is cleared
// Return 0 for input == 0
{
    return x & (x - 1);
}

double ui64_sign(uint64_t x, int n) {
    // TODO PERF: speedup by avoiding the mask altogether
    // This implementation is 20 x times faster than one based on for loops
    // First build a mask with n bit set. Then & with string and count.
    // Example for 16 bit string
    //                 n            (n = 5)
    // want       11111000 00000000
    // mask       11111111 11111111
    // mask << 11 00000000 00011111 11 = 16 - 5
    // mask >> 11 11111000 00000000
    //
    // Note: This strategy does not work when n = 0 because ~0 << 64 = ~0 (!!!)
    // so we treat this case separately
    if (n == 0)
        return 1.0;
    uint64_t mask = ~0;
    mask = mask << (64 - n);             // make a string with n bits set
    mask = mask >> (64 - n);             // move it right by n
    mask = x & mask;                     // intersect with string
    mask = ui64_bit_count(mask);         // count bits in between
    return (mask % 2 == 0) ? 1.0 : -1.0; // compute sign
}

double ui64_sign_reverse(uint64_t x, int n) {
    // TODO PERF: speedup by avoiding the mask altogether
    // This implementation is 20 x times faster than one based on for loops
    // First build a mask with n bit set. Then & with string and count.
    // Example for 16 bit string
    //                 n            (n = 5)
    // want       00000011 11111111
    // mask       11111111 11111111
    // mask << 6  00000011 11111111 6 = 5 + 1
    //
    // Note: This strategy does not work when n = 0 because ~0 << 64 = ~0 (!!!)
    // so we treat this case separately
    if (n == 63)
        return 1.0;
    uint64_t mask = ~0;
    mask = mask << (n + 1);              // make a string with 64 - n - 1 bits set
    mask = x & mask;                     // intersect with string
    mask = ui64_bit_count(mask);         // count bits in between
    return (mask % 2 == 0) ? 1.0 : -1.0; // compute sign
}

double ui64_sign(uint64_t x, int m, int n) {
    // TODO PERF: speedup by avoiding the mask altogether
    // This implementation is a bit faster than one based on for loops
    // First build a mask with bit set between positions m and n. Then & with string and count.
    // Example for 16 bit string
    //              m  n            (m = 2, n = 5)
    // want       00011000 00000000
    // mask       11111111 11111111
    // mask << 14 00000000 00000011 14 = 16 + 1 - |5-2|
    // mask >> 11 00011000 00000000 11 = 16 - |5-2| -  min(2,5)
    // the mask should have |m - n| - 1 bits turned on
    uint64_t gap = std::abs(m - n);
    if (gap < 2) { // special cases
        return 1.0;
    }
    uint64_t mask = ~0;
    mask = mask << (65 - gap);                  // make a string with |m - n| - 1 bits set
    mask = mask >> (64 - gap - std::min(m, n)); // move it right after min(m, n)
    mask = x & mask;                            // intersect with string
    mask = ui64_bit_count(mask);                // count bits in between
    return (mask % 2 == 0) ? 1.0 : -1.0;        // compute sign
}

#endif // _bitwise_operations_hpp_

///**
// * @brief ui64_count Count the number of true bits
// * @param x input word
// * @param n position
// * @return the number of bits set to true from position 0 to n (included)
// */
// double ui64_bit_count(uint64_t x, int n) {
//    // TODO PERF: speedup by avoiding the mask altogether
//    // Example for 16 bit string
//    //                 n            (n = 5)
//    // x          ABCDEFGH IJKLMNOP
//    // x << 10    00000000 00ABCDEF 10 = 15 - 5
//    //
//    // Note: This strategy does not work when n = 0 because ~0 << 64 = ~0 (!!!)
//    // so we treat this case separately
//    x = x << (63 - n);
//    return ui64_bit_count(x);
//}

// double ui64_bit_count_reverse(uint64_t x, int n) {
//    // TODO PERF: speedup by avoiding the mask altogether
//    // Example for 16 bit string
//    //                 n            (n = 5)
//    // x          ABCDEFGH IJKLMNOP
//    // x >> 5     FGHIJKLM NOP00000
//    x = x >> n;
//    return ui64_bit_count(x);
//}
