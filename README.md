# TCG-Block-Cipher
Insecure block cipher cryptography based on triangular congruential generators. http://charcoding.github.io/TCG-Block-Cipher/

## TCG Generator
A Triangular Congruential Generator is initialized with multiplier `a` (must be odd number), increment `b` ∈ ℕ, mask `m = 2**n - 1`, and seed `x`, 0 ≤ x < m.  
Each time `.next().value` is called, it returns x = [a(x<sup>2</sup> + x) / 2 + b] AND m  
With carefully chosen parameters, the TCG will have a period = mask + 1.  
When used in the cipher, they are generated from an index and a seed. The index determines `a` and `b` from the arrays of constants, and the seed determines the starting value `x`. Mask is either 65535 or 262143.

Note: In JavaScript, `>>>` is the **Zero-fill right shift** operator, not circular bitwise rotate.  
`>>> 1` is equivalent to dividing by 2 for positive integers.  
It is used because the `>>` operator have sign-propagation.  
Circular bitwise rotate is denoted by the function `ROTR(word, rotation)`.
### Constants
`TCG.A16` is an array of 128 multiplier `a` values for 16 bit TCGs.  
`TCG.B16` is an array of 128 increment `b` values for 16 bit TCGs.  
Same goes for `A18` and `B18`, except there are only 4 of each and they are for 18 bit TCGs.  
## Encryption
0. Initialize key from input, `k = Uint32Array(4);`
	- if it is too short, repeat the input again
	- if it is too long, add the rest back onto key
1. Generate round keys `ks = new Array(NUMBER_OF_ROUNDS);`
2. Round function:
    - XOR with round key (in the middle round, add the round key mod 2<sup>32</sup> instead)
    - SBOX
    - PBOX
3. Last round:
    - XOR with round key
    - SBOX
## Key scheduling
0. Find the hamming weight of the 128-bit key
1. Find the sum of the 4 ints
2. Find the integer product of the 4 ints
3. Create a 18-bit generator (index = hamming weight, bits 0\~1, seed = sum, bits 0\~17)
4. Create an array of 8 16-bit generators based on the current key
    - index = k[0] bits 0\~7, seed = k[1] bits 0\~15
    - index = k[0] bits 8\~15, seed = k[1] bits 16\~31
    - index = k[0] bits 16\~23, seed = k[2] bits 0\~15
    - index = k[0] bits 24\~31, seed = k[2] bits 16\~31
    - index = k[1] bits 0\~7, seed = sum, bits 0\~15
    - index = k[1] bits 8\~15, seed = sum, bits 16\~31
    - index = k[1] bits 16\~23, seed = product, bits 0\~15
    - index = k[1] bits 24\~31, seed = product, bits 16\~31
5. Initialize the new key by reversing the bits of each word in the previous key
6. Loop 32 times: (i = 31...0, rolled backwards)
    0. `let temp =` 18-bit generator `.next().value`
    1. Select a 16-bit generator from the array (index = temp, bits 0\~3)
    2. Call 16-bit generator `.next().value << 15`, bitwise OR with `temp >>> 15`
    3. Circular right shift the result i bits
    4. XOR the result with new key `nk[i & 3]`
7. New key is returned
## Key whitening
For all rounds except the middle round, each 128-bit data block is XORed with the key.  
For the middle round (if number of rounds is even, round down), add the key to each 128-bit data block, then mod 2<sup>32</sup>. (subtract if decrypting)
## Substitution box
0. Indices are calculated based on k[0], k[1], and hamming weight of k[2]:
    - `const gen = ROTR(k[0] << 1 ^ k[1] >>> 1, hammingWeight(k[2]));` (ROTR = circular right shift)
1. Seeds are calculated based on k[2], k[3]:
    - `const seeds = ~k[2] ^ k[3] << 3 ^ k[3] >> 4;`
2. Two 16-bit TCGs are created:
    - index = gen bits 8\~15, seed = seeds bits 16\~31
    - index = gen bits 0\~7, seed = seeds bits 0\~15
3. Swap the generators if decrypting (so that index and value are swapped in the lookup table)
4. Loop 65536 times:
    - For each 16-bit output of G0, its replacement will be the 16-bit output of G1:
      - `SBOX[G0.next().value] = G1.next().value;`
5. Go through the data array and replace each 16-bit subword with the value in the lookup table
## Permutation box
0. Index is calculated based on hamming weight of k[1], k[2], and k[3]:
    - `const gen = ROTR(k[3] << 2 ^ k[2] >>> 1, hammingWeight(k[1]));`
1. Seed is calculated based on k[0] and k[1]:
    - `const seed = reverseBits(k[1]) ^ k[0] << 4 ^ k[0] >>> 3;`
2. Create a 16-bit TCG:
    - index = gen bits 0\~7, seed = seed bits 0\~15
3. Get 128 values from the TCG
4. Sort the values, and perform the same swaps on an array of integers in ascending `order` 0...127, so that when the outputs of TCG are sorted, the integers 0...127 are permuted accordingly.
5. For each 128-bit block of data, copy the nth bit to the position `order[n]`. (vice versa for decryption)
## Things that it does well
* Avalanche effect
* Dynamic S-box and P-box
## Problems and vulnerabilities
* Uses a lot of memory and is kind of slow
* Key scheduling algorithm is basically security through obscurity
* TCG generators are fully predictable
* CTR mode only works with strings shorter than 2^20 characters
* Other security vulnerabilities (95 more...)
