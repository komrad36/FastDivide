# FastDivide

### It probably shouldn't be possible to divide (or modulo) two 64-bit unsigned integers on x86 faster than the hardware `div` instruction, but, it is. About 30% faster. ###

At least on Intel, where `div` is crazy slow.

Technically the hardware `div` instruction divides a 128-bit numerator by a 64-bit denominator, but there's no reason it couldn't check for empty high bits or have a 64-bit version. What's worse, the 128-bit capability is very rarely used, because if the result doesn't fit in 64 bits, it explodes with a hardware exception! (Instead of, say, returning a truncated result and setting some flags, like most arithmetic instructions.)

Because the implementation is completely free of any branches or memory accesses, it also doesn't leak any side-channel information about its arguments (at least, not via timing or memory!), so it could be useful for crypto applications while actually improving performance.

For denominators known at compile-time, there's no need to use this library. Just express the division/modulo directly in C/C++ and let the compiler handle it, as it can generate quite optimal codegen for that case.

Because a lot of the work is precomputable for a given denominator/divisor, a class is also provided allowing you to perform this precomputation, then repeatedly divide (or modulo) different numerators by that same denominator.

If you need to divide/modulo by the same divisor a *HUGE* number of times, it is possible to get even more efficient than this library by taking different approaches based on the specific divisor (they fall into a few different possible categories), but then you need to branch your entire workload based on the category, and the set-up process also becomes much slower. The distinction with this library is that setup is extremely cheap, so cheap that it beats hardware even for just a single division, and you don't need to branch your workload code.

Supports Clang, GCC, and MSVC.

#### Theory: ####

First, `floor(2^64/D)` is computed with extreme care to efficiency. The first correct result bit is obtained via leading zero counts, the second via shifts, then subsequently the correct bit count is doubled by each of 5 Newton-Raphson iterations to obtain 64 correct bits. Careful construction avoids excessive shifts, branching, or the need to track more than 64 bits at a time during this process.

Then, a multiply-high of this value with any desired numerator produces either the correct answer, or 1 below. This condition is checked and fixed, and the result is returned.

In some cases intermediate fixup of the value `floor(2^64/D)` is skipped, causing the final answer to be either correct, 1 below, or 2 below, when this condition can be fixed more efficiently than separately fixing both the intermediate and final answer.

The result of division/modulo by 0 is undefined.

#### Example usage: ####

Divide two arbitrary 64-bit integers, producing a 64-bit result, faster than hardware:
```
    std::cout << FastDivide(N, D) << std::endl;
```

Modulo (remainder after division) two arbitrary 64-bit integers, producing a 64-bit result, faster than hardware:
```
    std::cout << FastModulo(N, D) << std::endl;
```

Compute division and modulo simultaneously for two arbitrary 64-bit integers, producing 64-bit results, faster than hardware:
```
    uint64_t rem;
    uint64_t div = FastDivMod(N, D, rem);
    std::cout << div << ", " << rem << std::endl;
```

Precompute for a given divisor and then perform multiple fast divisions, modulos, or both simultaneously:
```
    FastDivider divider(D);
    
    std::cout << divider.Divide(N1) << std::endl;
    std::cout << divider.Divide(N2) << std::endl;
    std::cout << divider.Divide(N3) << std::endl;
    
    std::cout << divider.Modulo(N1) << std::endl;
    std::cout << divider.Modulo(N2) << std::endl;
    std::cout << divider.Modulo(N3) << std::endl;
    
    uint64_t rem1, rem2, rem3;
    uint64_t div1 = divider.DivMod(N1, rem1);
    uint64_t div2 = divider.DivMod(N2, rem2);
    uint64_t div3 = divider.DivMod(N3, rem3);
```
