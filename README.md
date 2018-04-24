# TCG-Block-Cipher
Insecure block cipher cryptography based on triangular congruential generators. http://charcoding.github.io/TCG-Block-Cipher/

## Things that it does well
* Avalanche effect
* Dynamic S-box and P-box

## Problems and vulnerabilities
* Uses a lot of memory and is kind of slow
* Key scheduling algorithm is basically security through obscurity
* TCG generators are fully predictable
* CTR mode only works with strings shorter than 2^20 characters
* Other security vulnerabilities (95 more...)
