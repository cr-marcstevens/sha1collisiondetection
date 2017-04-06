/***
* Copyright 2017 Marc Stevens <marc@marc-stevens.nl>, Dan Shumow <danshu@microsoft.com>
* Distributed under the MIT Software License.
* See accompanying file LICENSE.txt or copy at
* https://opensource.org/licenses/MIT
***/

#ifndef SHA1DC_SHA1_H
#define SHA1DC_SHA1_H

#ifdef _MSC_VER
#define SHA1DC_API __cdecl
#else
#define SHA1DC_API
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

/* uses SHA-1 message expansion to expand the first 16 words of W[] to 80 words */
/* void sha1_message_expansion(uint32_t W[80]); */

/* sha-1 compression function; first version takes a message block pre-parsed as 16 32-bit integers, second version takes an already expanded message) */
/* void sha1_compression(uint32_t ihv[5], const uint32_t m[16]);
void sha1_compression_W(uint32_t ihv[5], const uint32_t W[80]); */

/* same as sha1_compression_W, but additionally store intermediate states */
/* only stores states ii (the state between step ii-1 and step ii) when DOSTORESTATEii is defined in ubc_check.h */
void sha1_compression_states(uint32_t[5], const uint32_t[16], uint32_t[80], uint32_t[80][5]);

/*
// function type for sha1_recompression_step_T (uint32_t ihvin[5], uint32_t ihvout[5], const uint32_t me2[80], const uint32_t state[5])
// where 0 <= T < 80
//       me2 is an expanded message (the expansion of an original message block XOR'ed with a disturbance vector's message block difference)
//       state is the internal state (a,b,c,d,e) before step T of the SHA-1 compression function while processing the original message block
// the function will return:
//       ihvin: the reconstructed input chaining value
//       ihvout: the reconstructed output chaining value
*/
typedef void(*sha1_recompression_type)(uint32_t*, uint32_t*, const uint32_t*, const uint32_t*);

/* table of sha1_recompression_step_0, ... , sha1_recompression_step_79 */
/* extern sha1_recompression_type sha1_recompression_step[80];*/

/* a callback function type that can be set to be called when a collision block has been found: */

#ifdef SHA1DC_CALLBACK_USES_PARAM

    // void collision_block_callback(void* param, uint64_t byteoffset, const uint32_t ihvin1[5], const uint32_t ihvin2[5], const uint32_t m1[80], const uint32_t m2[80])
    typedef void(SHA1DC_API * collision_block_callback)(void* param, uint64_t, const uint32_t*, const uint32_t*, const uint32_t*, const uint32_t*);

#else

    // void collision_block_callback(uint64_t byteoffset, const uint32_t ihvin1[5], const uint32_t ihvin2[5], const uint32_t m1[80], const uint32_t m2[80])
    typedef void(SHA1DC_API * collision_block_callback)(uint64_t, const uint32_t*, const uint32_t*, const uint32_t*, const uint32_t*);

#endif

/* the SHA-1 context */
typedef struct {
	uint64_t total;
	uint32_t ihv[5];
	unsigned char buffer[64];
	int found_collision;
	int safe_hash;
	int detect_coll;
	int ubc_check;
	int reduced_round_coll;
	collision_block_callback callback;
#ifdef SHA1DC_CALLBACK_USES_PARAM
	void* callback_param;
#endif
	uint32_t ihv1[5];
	uint32_t ihv2[5];
	uint32_t m1[80];
	uint32_t m2[80];
	uint32_t states[80][5];
} SHA1_CTX;

/* initialize SHA-1 context */
void SHA1DC_API SHA1DCInit(SHA1_CTX*);

/*
// function to enable safe SHA-1 hashing:
// collision attacks are thwarted by hashing a detected near-collision block 3 times
// think of it as extending SHA-1 from 80-steps to 240-steps for such blocks:
//   the best collision attacks against SHA-1 have complexity about 2^60,
//   thus for 240-steps an immediate lower-bound for the best cryptanalytic attacks would 2^180
//   an attacker would be better off using a generic birthday search of complexity 2^80
//
// enabling safe SHA-1 hashing will result in the correct SHA-1 hash for messages where no collision attack was detected
// but it will result in a different SHA-1 hash for messages where a collision attack was detected
// this will automatically invalidate SHA-1 based digital signature forgeries
// enabled by default
*/
void SHA1DC_API SHA1DCSetSafeHash(SHA1_CTX*, int);

/* function to disable or enable the use of Unavoidable Bitconditions (provides a significant speed up) */
/* enabled by default */
void SHA1DC_API SHA1DCSetUseUBC(SHA1_CTX*, int);

/* function to disable or enable the use of Collision Detection */
/* enabled by default */
void SHA1DC_API SHA1DCSetUseDetectColl(SHA1_CTX*, int);

/* function to disable or enable the detection of reduced-round SHA-1 collisions */
/* disabled by default */
void SHA1DC_API SHA1DCSetDetectReducedRoundCollision(SHA1_CTX*, int);

/* function to set a callback function, pass NULL to disable */
/* by default no callback set */

#ifdef SHA1DC_CALLBACK_USES_PARAM
void SHA1DC_API SHA1DCSetCallback(SHA1_CTX*, collision_block_callback, void*);
#else
void SHA1DC_API SHA1DCSetCallback(SHA1_CTX*, collision_block_callback);
#endif

/* update SHA-1 context with buffer contents */
void SHA1DC_API SHA1DCUpdate(SHA1_CTX*, const char*, size_t);

/* obtain SHA-1 hash from SHA-1 context */
/* returns: 0 = no collision detected, otherwise = collision found => warn user for active attack */
int SHA1DC_API SHA1DCFinal(unsigned char[20], SHA1_CTX*); 

#if defined(__cplusplus)
}
#endif

#endif
