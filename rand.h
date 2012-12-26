#ifndef RAND_H
#define RAND_H

#include <assert.h>
#include <stdint.h>

/* these two values should be matched;
 * these choices come from G. Marsaglia (2003)
 *   Seeds for Random Number Generators */

#define CMWC_RNG_LAG 16

#if CMWC_RNG_LAG == 16
/* lag-16 has a period of p = approx. 2**540 */
#define CMWC_RNG_MULTIPLIER 487198574u
#elif CMWC_RNG_LAG == 8
/* lag-8 has a period of p = approx. 2**285 */
#define CMWC_RNG_MULTIPLIER 716514398u
#else
#error "No known-good multiplier value for the specified CMWC lag."
#endif

struct cmwc_rng {
	int place;
	uint32_t carry;
	uint32_t state[CMWC_RNG_LAG];
};

void cmwc_init(struct cmwc_rng *rng, uint32_t seed);
void cmwc_init_full(struct cmwc_rng *rng, uint32_t carry, uint32_t *state);

inline uint32_t cmwc_next_i32(struct cmwc_rng *rng) {
	assert(rng);
	uint32_t result = rng->state[rng->place];
	uint64_t t = (uint64_t)result * (uint64_t)CMWC_RNG_MULTIPLIER + (uint64_t)rng->carry;
	uint32_t c = t >> 32;
	uint32_t x = t;
	/* fix-up because b = 2**32 - 1 not 2**32 */
	if (t + c < c) {
		++c;
		++x;
	}
	rng->state[rng->place] = (uint64_t)0xfffffffeu - t;
	rng->carry = c;
	rng->place = (rng->place + 1) % CMWC_RNG_LAG;
	return result;
}

inline uint64_t cmwc_next_i64(struct cmwc_rng *rng) {
	assert(rng);
	uint64_t a = cmwc_next_i32(rng);
	uint64_t b = cmwc_next_i32(rng);
	return (a << 32) | b;
}

/* period 2**128 - 1 */
struct xorshift_rng {
	uint32_t x, y, z, w;
};

void xorshift_init(struct xorshift_rng *rng, uint32_t seed);

inline uint32_t xorshift_next_i32(struct xorshift_rng *rng) {
	assert(rng);
	const uint32_t x = rng->x, y = rng->y, z = rng->z, w = rng->w;
	uint32_t t = x^(x<<15); t = (w^(w>>21)) ^ (t^(t>>4));
	rng->x = y; rng->y = z; rng->z = w; rng->w = t;
	return t;
}

inline uint64_t xorshift_next_i64(struct xorshift_rng *rng) {
	assert(rng);
	uint64_t a = xorshift_next_i32(rng);
	uint64_t b = xorshift_next_i32(rng);
	return (a << 32) | b;
}

#endif
