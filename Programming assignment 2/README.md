# Problem Statement
This program is a binary conversion function into a `to_base_n()` function that converts a non-negative base-10 integer into any base from 2-16. If the requested base is outside the allowed range of 2-16, the program prints an error message. The program also adds the expected prefixes for octal (`0`) and hexadecimal (`0x`) output.

# Describe the Solution
The solution uses repeated division and remainder. The remainder from `number % base` gives the next digit in the target base, and the number is reduced with `number / base` until it reaches zero. As a result of this process it produces digits in reverse order, the digits are stored in a character buffer and then printed backward. A digit lookup string, `"0123456789abcdef"`, allows the same function to handle bases above 10, where digits `a-f` are needed. The `main()` function reads the number and base from the command line, validates the argument count, and calls `to_base_n()` to display the converted value.

# Pros and Cons of your solution
## Pros
- Works for every base from 2 through 16 in one function.
- Correctly prints hexadecimal digits using `a-f`.
- Adds octal and hexadecimal prefixes to match standard notation.

## Cons
- Only handles non-negative integers.
- Input validation is basic.
- The output is printed directly instead of being returned as a string.