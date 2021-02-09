# FastDivide

### It probably shouldn't be possible to divide two 64-bit unsigned integers on x86 faster than the hardware `div` instruction, but, it is. About 30% faster. ###

Technically the hardware `div` instruction divides a 128-bit numerator by a 64-bit denominator, but there's no reason it couldn't check for empty high bits or have a 64-bit version. What's worse, the 128-bit capability is very rarely used, because if the result doesn't fit in 64 bits, it explodes with a hardware exception! (Instead of, say, returning a truncated result and setting some flags, like most arithmetic instructions.)

Because a lot of the work is precomputable for a given denominator/divisor, a class is also provided allowing you to perform this precomputation, then repeatedly divide different numerators by that same denominator.

Theory:
First, `floor(2^64/D)` is computed with extreme care to efficiency. The first correct result bit is obtained via leading bit counts, the second via shifts, then subsequently the correct bit count is doubled by each of 5 Newton-Raphson iterations to obtain 64 correct bits. Careful construction avoids excessive shifts, branching, or the need to track more than 64 bits at a time during this process.

Then, a multiply-high of this value with any desired numerator produces either the correct answer, or 1 below. This condition is checked and fixed, and the result is returned.

The result of division by 0 is undefined.

Example usage:

Divide two arbitrary 64-bit integers, producing a 64-bit result, faster than hardware:
```
    std::cout << FastDivide(N, D) << std::endl;
```

Precompute for a given divisor and then perform multiple fast divisions:
```
    FastDivider divider(D);
    std::cout << divider.Divide(N1) << std::endl;
    std::cout << divider.Divide(N2) << std::endl;
    std::cout << divider.Divide(N3) << std::endl;
```
