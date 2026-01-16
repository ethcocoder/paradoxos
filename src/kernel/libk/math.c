#include <stdint.h>

uint32_t k_pow(int32_t base, int32_t exponent) {
    if (exponent == 0) return 1;
    if (exponent < 0) {
        if (base == 0) return 0; 
        base = 1 / base;
        exponent = -exponent;
    }
    int32_t temp = 1;
    while (exponent > 1) {
        if (exponent % 2 == 0) {
            base = base * base;
            exponent = exponent / 2;
        } else {
            temp = base * temp;
            base = base * base;
            exponent = (exponent - 1) / 2;
        }
    }
    return base * temp;
}
