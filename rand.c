#include "rand.h"
#include <string.h>
#include <assert.h>

extern uint32_t cmwc_next_i32(struct cmwc_rng *rng);
extern uint64_t cmwc_next_i64(struct cmwc_rng *rng);
extern uint32_t xorshift_next_i32(struct xorshift_rng *rng);
extern uint64_t xorshift_next_i64(struct xorshift_rng *rng);

void cmwc_init(struct cmwc_rng *rng, uint32_t seed) {
	assert(rng);
	rng->place = 0;
	rng->carry = (seed % CMWC_RNG_MULTIPLIER);

	/* use a 32-bit xorshift RNG to fill the state from the seed value */
	uint32_t x = seed ? seed : 123456789u;
	for (int i = 0; i < CMWC_RNG_LAG; ++i) {
		x ^= (x << 13);
		x ^= (x >> 17);
		x ^= (x << 5);
		rng->state[i] = x;
	}
}

void cmwc_init_full(struct cmwc_rng *rng, uint32_t carry, uint32_t *state) {
	assert(rng);
	rng->place = 0;
	rng->carry = carry;
	memcpy(rng->state, state, sizeof(uint32_t) * CMWC_RNG_LAG);
}

void xorshift_init(struct xorshift_rng *rng, uint32_t seed) {
	assert(rng);
	uint32_t x, y, z, w;
	x = seed ? seed : 123456789u;
	y = x^(x<<13); y ^= (y >> 17); y ^= (y << 5);
	z = y^(y<<13); z ^= (z >> 17); z ^= (z << 5);
	w = z^(z<<13); w ^= (w >> 17); w ^= (w << 5);
	rng->x = x; rng->y = y; rng->z = z; rng->w = w;
}
