/*******************************************************************
*
*    Author: Kareem Omar
*    kareem.h.omar@gmail.com
*    https://github.com/komrad36
*
*    Last updated Feb 10, 2021
*******************************************************************/

#pragma once

#include <cstdint>
#include <immintrin.h>

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
    static inline uint64_t Magic64(uint64_t C, uint64_t D)
    {
#if defined(__clang__) || defined(__GNUC__)
        uint64_t E = C, F;
        asm("\
    neg %b[A]                  \n\
    shlx %[D], %[C], %[E]      \n\
    shrx %[A], %[E], %[E]      \n\
    cmovnz %[E], %[A]          \n\
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
    add %[C], %[E]             \n\
    adc $0, %[A]               \n\
    sbb $0, %[A]"
            : [C] "+&c" (D), [A] "+&a" (C), [D] "+&d" (E), [E] "=&r" (F)
            :
            : "cc"
        );
        return C;
#elif defined(_MSC_VER)
        uint64_t F = C;
        C = (uint64_t)-(int64_t)C;
        uint64_t E = D << F;
        E >>= C;
        C = C ? E : C;
        C |= 1ULL << F;
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        C += __umulh(C, C * D);
        F = C * D;
        unsigned char carry = _addcarry_u64(0, F, D, &F);
        carry = _addcarry_u64(carry, C, 0, &C);
        _subborrow_u64(carry, C, 0, &C);
        return C;
#else
#error UNSUPPORTED
#endif
    }

    static inline uint64_t MagicDiv64(uint64_t N, uint64_t D, uint64_t M)
    {
#if defined(__clang__) || defined(__GNUC__)
        uint64_t E;
        asm("\
    mul %[N]                   \n\
    mov %[rdx], %[A]           \n\
    imul %[D], %[rdx]          \n\
    add %[rdx], %[N]           \n\
    add %[D], %[N]             \n\
    adc $0, %[A]"
            : [N] "+&r" (N), [A] "+&a" (M), [rdx] "=&d" (E)
            : [D] "r" (D)
            : "cc"
        );
        return M;
#elif defined(_MSC_VER)
        M = __umulh(M, N);
        _addcarry_u64(_addcarry_u64(0, N + M * D, D, &N), M, 0, &M);
        return M;
#else
#error UNSUPPORTED
#endif
    }

    static inline uint64_t MagicMod64(uint64_t N, uint64_t D, uint64_t M)
    {
#if defined(__clang__) || defined(__GNUC__)
        uint64_t E;
        asm("\
    mul %[N]                   \n\
    imul %[D], %[rdx]          \n\
    add %[rdx], %[N]           \n\
    mov %[N], %[A]             \n\
    add %[D], %[N]             \n\
    cmovc %[N], %[A]"
            : [N] "+&r" (N), [A] "+&a" (M), [rdx] "=&d" (E)
            : [D] "r" (D)
            : "cc"
        );
        return M;
#elif defined(_MSC_VER)
        M = __umulh(M, N);
        N += M * D;
        M = N;
        if (_addcarry_u64(0, N, D, &N))
            M = N;
        return M;
#else
#error UNSUPPORTED
#endif
    }

    static inline uint64_t MagicDivMod64(uint64_t N, uint64_t D, uint64_t M, uint64_t& rem)
    {
#if defined(__clang__) || defined(__GNUC__)
        uint64_t E;
        asm("\
    mul %[N]                   \n\
    mov %[rdx], %[A]           \n\
    imul %[D], %[rdx]          \n\
    add %[rdx], %[N]           \n\
    mov %[N], %[rdx]           \n\
    add %[D], %[N]             \n\
    cmovnc %[rdx], %[N]        \n\
    adc $0, %[A]"
            : [N] "+&r" (N), [A] "+&a" (M), [rdx] "=&d" (E)
            : [D] "r" (D)
            : "cc"
        );
        rem = N;
        return M;
#elif defined(_MSC_VER)
        M = __umulh(M, N);
        N += M * D;
        // super dumb codegen, leads to minor slowdown vs inline asm. best we can do if stuck with MSVC
        unsigned char carry = _addcarry_u64(0, D, N, &D);
        if (carry)
            N = D;
        _addcarry_u64(carry, M, 0, &M);
        rem = N;
        return M;
#else
#error UNSUPPORTED
#endif
    }

public:
    inline FastDivider(uint64_t divisor) : m_D((uint64_t)-(int64_t)divisor), m_M(Magic64(_lzcnt_u64(divisor), m_D)) {}

    inline uint64_t GetDivisor() const { return m_D; }
    inline uint64_t Divide(uint64_t N) const { return MagicDiv64(N, m_D, m_M); }
    inline uint64_t Modulo(uint64_t N) const { return MagicMod64(N, m_D, m_M); }
    inline uint64_t DivMod(uint64_t N, uint64_t& rem) const { return MagicDivMod64(N, m_D, m_M, rem); }

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
    mov %[A], %[F]             \n\
    neg %b[A]                  \n\
    shlx %[F], %[D], %[E]      \n\
    shrx %[A], %[E], %[E]      \n\
    cmovnz %[E], %[A]          \n\
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
    mul %[N]                   \n\
    mov %[D], %[A]             \n\
    imul %[E], %[D]            \n\
    add %[D], %[N]             \n\
    mov %[N], %[D]             \n\
    add %[E], %[N]             \n\
    cmovnc %[D], %[N]          \n\
    adc $0, %[A]               \n\
    add %[E], %[N]             \n\
    adc $0, %[A]"
        : [N] "+&c" (N), [D] "+&d" (D), [A] "=&a" (A), [E] "=&r" (E), [F] "=&r" (F)
        :
        : "cc"
    );
    return A;
#elif defined(_MSC_VER)
    uint64_t C = _lzcnt_u64(D);
    D = (uint64_t)-(int64_t)D;
    uint64_t F = C;
    C = (uint64_t)-(int64_t)C;
    uint64_t E = D << F;
    E >>= C;
    C = C ? E : C;
    C |= 1ULL << F;
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C = __umulh(C, N);
    N += C * D;
    E = N;
    // super dumb codegen, leads to minor slowdown vs inline asm. best we can do if stuck with MSVC
    unsigned char carry = _addcarry_u64(0, N, D, &N);
    if (!carry)
        N = E;
    _addcarry_u64(carry, C, 0, &C);
    _addcarry_u64(_addcarry_u64(0, N, D, &N), C, 0, &C);
    return C;
#else
#error UNSUPPORTED
#endif
}

// Perform a single fast modulo of 64-bit quantities, N % D, producing a 64-bit result.
// The result of modulo by 0 is undefined.
static inline uint64_t FastModulo(uint64_t N, uint64_t D)
{
#if defined(__clang__) || defined(__GNUC__)
    uint64_t A, E, F;
    asm("\
    lzcnt %[D], %[A]           \n\
    neg %[D]                   \n\
    mov %[A], %[F]             \n\
    neg %b[A]                  \n\
    shlx %[F], %[D], %[E]      \n\
    shrx %[A], %[E], %[E]      \n\
    cmovnz %[E], %[A]          \n\
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
    mul %[N]                   \n\
    imul %[E], %[D]            \n\
    add %[D], %[N]             \n\
    mov %[N], %[A]             \n\
    add %[E], %[N]             \n\
    cmovnc %[A], %[N]          \n\
    mov %[N], %[A]             \n\
    add %[E], %[N]             \n\
    cmovc %[N], %[A]"
        : [N] "+&c" (N), [D] "+&d" (D), [A] "=&a" (A), [E] "=&r" (E), [F] "=&r" (F)
        :
        : "cc"
    );
    return A;
#elif defined(_MSC_VER)
    uint64_t C = _lzcnt_u64(D);
    D = (uint64_t)-(int64_t)D;
    uint64_t F = C;
    C = (uint64_t)-(int64_t)C;
    uint64_t E = D << F;
    E >>= C;
    C = C ? E : C;
    C |= 1ULL << F;
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C = __umulh(C, N);
    N += C * D;
    C = N;
    if (!_addcarry_u64(0, N, D, &N))
        N = C;
    C = N;
    if (_addcarry_u64(0, N, D, &N))
        C = N;
    return C;
#else
#error UNSUPPORTED
#endif
}

// Perform a single fast simultaneous division and modulo of 64-bit quantities, N / D and N % D, both
// 64-bit results.
//
// The quotient (division result) is returned, while 'rem' is populated with the remainder (modulo result).
//
// The result of division/modulo by 0 is undefined.
static inline uint64_t FastDivMod(uint64_t N, uint64_t D, uint64_t& rem)
{
#if defined(__clang__) || defined(__GNUC__)
    uint64_t A, E, F;
    asm("\
    lzcnt %[D], %[A]           \n\
    neg %[D]                   \n\
    mov %[A], %[F]             \n\
    neg %b[A]                  \n\
    shlx %[F], %[D], %[E]      \n\
    shrx %[A], %[E], %[E]      \n\
    cmovnz %[E], %[A]          \n\
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
    mul %[N]                   \n\
    mov %[D], %[A]             \n\
    imul %[E], %[D]            \n\
    add %[D], %[N]             \n\
    mov %[N], %[D]             \n\
    add %[E], %[N]             \n\
    cmovnc %[D], %[N]          \n\
    adc $0, %[A]               \n\
    mov %[N], %[D]             \n\
    add %[E], %[N]             \n\
    cmovnc %[D], %[N]          \n\
    adc $0, %[A]"
        : [N] "+&c" (N), [D] "+&d" (D), [A] "=&a" (A), [E] "=&r" (E), [F] "=&r" (F)
        :
        : "cc"
    );
    rem = N;
    return A;
#elif defined(_MSC_VER)
    uint64_t C = _lzcnt_u64(D);
    D = (uint64_t)-(int64_t)D;
    uint64_t F = C;
    C = (uint64_t)-(int64_t)C;
    uint64_t E = D << F;
    E >>= C;
    C = C ? E : C;
    C |= 1ULL << F;
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C += __umulh(C, C * D);
    C = __umulh(C, N);
    N += C * D;
    // super dumb codegen, leads to minor slowdown vs inline asm. best we can do if stuck with MSVC
    unsigned char carry;
    E = N;
    carry = _addcarry_u64(0, N, D, &N);
    if (!carry)
        N = E;
    _addcarry_u64(carry, C, 0, &C);
    E = N;
    carry = _addcarry_u64(0, N, D, &N);
    if (!carry)
        N = E;
    _addcarry_u64(carry, C, 0, &C);
    rem = N;
    return C;
#else
#error UNSUPPORTED
#endif
}
