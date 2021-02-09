/*******************************************************************
*
*    Author: Kareem Omar
*    kareem.h.omar@gmail.com
*    https://github.com/komrad36
*
*    Last updated Feb 9, 2021
*******************************************************************/

#pragma once

#include <cstdint>

#if !defined(__clang__) && !defined(__GNUC__)
#ifdef _MSC_VER
#include <intrin.h>
#else
#error UNSUPPORTED
#endif
#endif

// Precompute information needed for repeated fast division by a particular denominator.
// The result of division by 0 is undefined.
class FastDivider
{
private:
    static inline uint64_t Magic64(uint64_t D)
    {
#if defined(__clang__) || defined(__GNUC__)
        uint64_t A, E, F;
        asm("\
    lzcnt %[C], %[A]           \n\
    neg %[C]                   \n\
    shlx %[A], %[C], %[E]      \n\
    mov %[A], %[D]             \n\
    neg %b[A]                  \n\
    cmovz %[A], %[E]           \n\
    shrx %[A], %[E], %[A]      \n\
    bts %[D], %[A]             \n\
    mov %[A], %[E]             \n\
    imul %[C], %[A]            \n\
    mul %[E]                   \n\
    add %[D], %[E]             \n\
    mov %[E], %[A]             \n\
    imul %[C], %[A]            \n\
    mul %[E]                   \n\
    add %[D], %[E]             \n\
    mov %[E], %[A]             \n\
    imul %[C], %[A]            \n\
    mul %[E]                   \n\
    add %[D], %[E]             \n\
    mov %[E], %[A]             \n\
    imul %[C], %[A]            \n\
    mul %[E]                   \n\
    add %[D], %[E]             \n\
    mov %[E], %[A]             \n\
    imul %[C], %[A]            \n\
    mul %[E]                   \n\
    add %[D], %[E]             \n\
    mov %[E], %[A]             \n\
    imul %[C], %[E]            \n\
    neg %[C]                   \n\
    cmp %[C], %[E]             \n\
    sbb $-1, %[A]              \n\
    adc $-1, %[A]"
            : [C] "+&c" (D), [A] "=&a" (A), [D] "=&d" (E), [E] "=&r" (F)
            :
            : "cc"
        );
        return A;
#elif defined(_MSC_VER)
        uint64_t C = _lzcnt_u64(D);
        D = (uint64_t)-(int64_t)D;
        uint64_t E = D << C;
        uint64_t F = C;
        C = (uint64_t)-(int64_t)C;
        E = !C ? C : E;
        C = E >> C;
        C |= 1ULL << F;
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        F = C * D;
        D = (uint64_t)-(int64_t)D;
        unsigned char carry = _subborrow_u64(0, F, D, &F);
        carry = _subborrow_u64(carry, C, ~0ULL, &C);
        _addcarry_u64(carry, C, ~0ULL, &C);
        return C;
#else
#error UNSUPPORTED
#endif
    }

    static inline uint64_t MagicDiv64(uint64_t N, uint64_t D, uint64_t magic)
    {
#if defined(__clang__) || defined(__GNUC__)
        uint64_t E;
        asm("\
    mul %[N]                   \n\
    mov %[rdx], %[A]           \n\
    imul %[D], %[rdx]          \n\
    sub %[rdx], %[N]           \n\
    cmp %[D], %[N]             \n\
    sbb $-1, %[A]"
            : [N] "+&r" (N), [A] "+&a" (magic), [rdx] "=&d" (E)
            : [D] "r" (D)
            : "cc"
        );
        return magic;
#elif defined(_MSC_VER)
        magic = __umulh(magic, N);
        unsigned char carry = _subborrow_u64(0, N - magic * D, D, &N);
        _subborrow_u64(carry, magic, ~0ULL, &magic);
        return magic;
#else
#error UNSUPPORTED
#endif
    }

public:
    inline FastDivider(uint64_t divisor) : m_D(divisor), m_M(Magic64(divisor)) {}

    inline uint64_t GetDivisor() const { return m_D; }
    inline uint64_t Divide(uint64_t N) const { return MagicDiv64(N, m_D, m_M); }

private:
    uint64_t m_D;
    uint64_t m_M;
};

// Perform a single fast division of 64-bit quantities, N / D, producing a 64-bit result.
// The result of division by 0 is undefined.
static inline uint64_t FastDivide(uint64_t N, uint64_t D)
{
#if defined(__clang__) || defined(__GNUC__)
    uint64_t A, E, F;
    asm("\
    lzcnt %[D], %[A]           \n\
    neg %[D]                   \n\
    shlx %[A], %[D], %[E]      \n\
    mov %[A], %[F]             \n\
    neg %b[A]                  \n\
    cmovz %[A], %[E]           \n\
    shrx %[A], %[E], %[A]      \n\
    bts %[F], %[A]             \n\
    mov %[D], %[E]             \n\
    mov %[A], %[F]             \n\
    imul %[E], %[A]            \n\
    mul %[F]                   \n\
    add %[D], %[F]             \n\
    mov %[F], %[A]             \n\
    imul %[E], %[A]            \n\
    mul %[F]                   \n\
    add %[D], %[F]             \n\
    mov %[F], %[A]             \n\
    imul %[E], %[A]            \n\
    mul %[F]                   \n\
    add %[D], %[F]             \n\
    mov %[F], %[A]             \n\
    imul %[E], %[A]            \n\
    mul %[F]                   \n\
    add %[D], %[F]             \n\
    mov %[F], %[A]             \n\
    imul %[E], %[A]            \n\
    mul %[F]                   \n\
    add %[D], %[F]             \n\
    mov %[F], %[A]             \n\
    imul %[E], %[F]            \n\
    neg %[E]                   \n\
    cmp %[E], %[F]             \n\
    sbb $-1, %[A]              \n\
    adc $-1, %[A]              \n\
    mul %[N]                   \n\
    mov %[D], %[A]             \n\
    imul %[E], %[D]            \n\
    sub %[D], %[N]             \n\
    cmp %[E], %[N]             \n\
    sbb $-1, %[A]"
        : [N] "+&c" (N), [D] "+&d" (D), [A] "=&a" (A), [E] "=&r" (E), [F] "=&r" (F)
        :
        : "cc"
    );
    return A;
#elif defined(_MSC_VER)
    uint64_t C = _lzcnt_u64(D);
    D = (uint64_t)-(int64_t)D;
    uint64_t E = D << C;
    uint64_t F = C;
    C = (uint64_t)-(int64_t)C;
    E = !C ? C : E;
    C = E >> C;
    C |= 1ULL << F;
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    F = C * D;
    D = (uint64_t)-(int64_t)D;
    unsigned char carry = _subborrow_u64(0, F, D, &F);
    carry = _subborrow_u64(carry, C, ~0ULL, &C);
    _addcarry_u64(carry, C, ~0ULL, &C);
    C = __umulh(C, N);
    carry = _subborrow_u64(0, N - C * D, D, &N);
    _subborrow_u64(carry, C, ~0ULL, &C);
    return C;
#else
#error UNSUPPORTED
#endif
}
