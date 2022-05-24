#include <stdint.h>
#include <stdio.h>

int32_t is_prime(int32_t n) {
    int32_t factors = 0;
    for (int32_t i=2; i < n; i++) {
        if (n % i == 0) factors += 1;
    }
    return factors == 0;
}

int main() {
    int32_t limit = 20000;
    int32_t total = 0;
    for (int32_t i=2; i < limit; i++) {
        if (is_prime(i)) total += 1;
    }
    printf("%d\n", total);
}
