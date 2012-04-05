#ifndef TOM_SHA1_H
#define TOM_SHA1_H

#ifdef __cplusplus
extern "C" {
#endif

struct sha1_state;
typedef struct sha1_state sha1_state;

sha1_state *sha1_new();
int sha1_process(sha1_state *state, const unsigned char *in, unsigned long inlen);
int sha1_done(sha1_state *state, unsigned char *hash);
void sha1_destroy(sha1_state *state);

#ifdef __cplusplus
}
#endif

#endif

/* $Source: /cvs/libtom/libtomcrypt/src/headers/tomcrypt_hash.h,v $ */
/* $Revision: 1.22 $ */
/* $Date: 2007/05/12 14:32:35 $ */
