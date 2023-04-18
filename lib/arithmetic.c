#include <stdint.h>

intmax_t gcd(intmax_t a, intmax_t b) {
  if (b != 0) {
    while ((a %= b) && (b %= a)) {
    }
  }
  return a + b;
}

intmax_t lcm(intmax_t a, intmax_t b) {
  return a * b / gcd(a, b);
}
