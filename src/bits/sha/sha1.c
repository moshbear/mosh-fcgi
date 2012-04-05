/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtom.org
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#include "rotl.h"
#include "sha1.h"

struct sha1_state {
	uint64_t length;
	uint32_t state[5], curlen;
	unsigned char buf[64];
};


/**
  @file sha1.c
  LTC_SHA1 code by Tom St Denis
*/


#define F0(x,y,z)  (z ^ (x & (y ^ z)))
#define F1(x,y,z)  (x ^ y ^ z)
#define F2(x,y,z)  ((x & y) | (z & (x | y)))
#define F3(x,y,z)  (x ^ y ^ z)

static void sha1_compress(sha1_state *state, const unsigned char *buf)
{
	uint32_t a, b, c, d, e, W[80], i;

	/* copy the state into 512-bits into W[0..15] */
	for (i = 0; i < 16; i++) {
		W[i] = ntohl(((const uint32_t *) buf)[i]);
	}

	/* copy state */
	a = state->state[0];
	b = state->state[1];
	c = state->state[2];
	d = state->state[3];
	e = state->state[4];

	/* expand it */
	for (i = 16; i < 80; i++) {
		W[i] = ROL(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
	}

	/* compress */
	/* round one */
#define FF0(a,b,c,d,e,i) e = (ROLc(a, 5) + F0(b,c,d) + e + W[i] + 0x5a827999UL); b = ROLc(b, 30);
#define FF1(a,b,c,d,e,i) e = (ROLc(a, 5) + F1(b,c,d) + e + W[i] + 0x6ed9eba1UL); b = ROLc(b, 30);
#define FF2(a,b,c,d,e,i) e = (ROLc(a, 5) + F2(b,c,d) + e + W[i] + 0x8f1bbcdcUL); b = ROLc(b, 30);
#define FF3(a,b,c,d,e,i) e = (ROLc(a, 5) + F3(b,c,d) + e + W[i] + 0xca62c1d6UL); b = ROLc(b, 30);

	for (i = 0; i < 20;) {
		FF0(a, b, c, d, e, i++);
		FF0(e, a, b, c, d, i++);
		FF0(d, e, a, b, c, i++);
		FF0(c, d, e, a, b, i++);
		FF0(b, c, d, e, a, i++);
	}

	/* round two */
	for (; i < 40;) {
		FF1(a, b, c, d, e, i++);
		FF1(e, a, b, c, d, i++);
		FF1(d, e, a, b, c, i++);
		FF1(c, d, e, a, b, i++);
		FF1(b, c, d, e, a, i++);
	}

	/* round three */
	for (; i < 60;) {
		FF2(a, b, c, d, e, i++);
		FF2(e, a, b, c, d, i++);
		FF2(d, e, a, b, c, i++);
		FF2(c, d, e, a, b, i++);
		FF2(b, c, d, e, a, i++);
	}

	/* round four */
	for (; i < 80;) {
		FF3(a, b, c, d, e, i++);
		FF3(e, a, b, c, d, i++);
		FF3(d, e, a, b, c, i++);
		FF3(c, d, e, a, b, i++);
		FF3(b, c, d, e, a, i++);
	}

#undef FF0
#undef FF1
#undef FF2
#undef FF3

	/* store */
	state->state[0] = state->state[0] + a;
	state->state[1] = state->state[1] + b;
	state->state[2] = state->state[2] + c;
	state->state[3] = state->state[3] + d;
	state->state[4] = state->state[4] + e;

}


/**
   Initialize the hash state
   @param md   The hash state you wish to initialize
   @return CRYPT_OK if successful
*/
sha1_state *sha1_new()
{
	sha1_state *state = malloc(sizeof(sha1_state));
	if (state == NULL)
		return NULL;
	state->state[0] = 0x67452301UL;
	state->state[1] = 0xefcdab89UL;
	state->state[2] = 0x98badcfeUL;
	state->state[3] = 0x10325476UL;
	state->state[4] = 0xc3d2e1f0UL;
	state->curlen = 0;
	state->length = 0;
	return state;
}

/**
   Process a block of memory though the hash
   @param md     The hash state
   @param in     The data to hash
   @param inlen  The length of the data (octets)
   @return CRYPT_OK if successful
*/
int sha1_process(sha1_state *state, const unsigned char *in, unsigned long inlen)
{
	unsigned long n;
	int err;
	if (state == NULL)
		goto einval;
	if (in == NULL)
		goto einval;
	if (state->curlen > 64)
		goto einval;
	while (inlen > 0) {
		if (state->curlen == 0 && inlen >= 64) {
			sha1_compress(state, in);
			state->length += (64 * 8);
			in += 64;
			inlen -= 64;
		} else {
			n = (64 - state->curlen);
			if (inlen <  n)
				n = inlen;
			memcpy(state->buf + state->curlen, in, n);
			state->curlen += n;
			inlen -= n;
			if (state->curlen == 64) {
				sha1_compress(state, state->buf);
				state->length += 64 * 8;
				state->curlen = 0;
			}
		}
	}
	return 0;
einval:
	errno = EINVAL;
	return -1;
}


/**
   Terminate the hash to get the digest
   @param md  The hash state
   @param out [out] The destination of the hash (20 bytes)
   @return CRYPT_OK if successful
*/
int sha1_done(sha1_state *state, unsigned char *out)
{
	int i;
	if (state == NULL)
		goto einval;
	if (out == NULL)
		goto einval;
	if (state->curlen > 64)
		goto einval;

	/* increase the length of the message */
	state->length += state->curlen * 8;

	/* append the '1' bit */
	memset(state->buf + state->curlen++, 0x80, 1);

	/* if the length is currently above 56 bytes we append zeros
	 * then compress.  Then we can fall back to padding zeros and length
	 * encoding like normal.
	 */
	if (state->curlen > 56) {
		memset(state->buf + state->curlen, 0, (64 - state->curlen));
		sha1_compress(state, state->buf);
		state->curlen = 0;
	}

	/* pad upto 56 bytes of zeroes */
	if (state->curlen < 56) {
		memset(state->buf + state->curlen, 0, (56 - state->curlen));
		state->curlen = 56;
	}

	/* store length */
	((uint32_t *)(state->buf))[14] = (state->length >> 32);
	((uint32_t *)(state->buf))[15] = (state->length & 0xFFFFFFFFUL);

	sha1_compress(state, state->buf);

	/* copy output */
	for (i = 0; i < 5; i++)
		((uint32_t *)out)[i] = htonl(state->state[i]);

	return 0;
einval:
	errno = EINVAL;
	return -1;
}

void sha1_destroy(sha1_state *state)
{
	free(state);
}

/* $Source: /cvs/libtom/libtomcrypt/src/hashes/sha1.c,v $ */
/* $Revision: 1.10 $ */
/* $Date: 2007/05/12 14:25:28 $ */
