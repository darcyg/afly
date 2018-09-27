#include <string.h>
#include "hmac.h"
#include "sha1.h"

void hmac_sha1(const _U8 *key, _I32 keyLength,const _U8 *data, _I32 dataLength,_U8 *result, _I32 resultLength) {
  _I32 i = 0;
  SHA1_INFO ctx;
  _U8 hashed_key[SHA1_DIGEST_LENGTH];
  if (keyLength > 64) {
    // The key can be no bigger than 64 bytes. If it is, we'll hash it down to
    // 20 bytes.
    
    sha1_init(&ctx);
    sha1_update(&ctx, key, keyLength);
    sha1_final(&ctx, hashed_key);
    key = hashed_key;
    keyLength = SHA1_DIGEST_LENGTH;
  }

  // The key for the inner digest is derived from our key, by padding the key
  // the full length of 64 bytes, and then XOR'ing each byte with 0x36.
  
  _U8 tmp_key[64];
  for (i = 0; i < keyLength; ++i) {
    tmp_key[i] = key[i] ^ 0x36;
  }
  
  if (keyLength < 64) {
    memset(tmp_key + keyLength, 0x36, 64 - keyLength);
  }

  // Compute inner digest
  sha1_init(&ctx);
  sha1_update(&ctx, tmp_key, 64);
  sha1_update(&ctx, data, dataLength);
  _U8 sha[SHA1_DIGEST_LENGTH];
  sha1_final(&ctx, sha);

  // The key for the outer digest is derived from our key, by padding the key
  // the full length of 64 bytes, and then XOR'ing each byte with 0x5C.
  for (i = 0; i < keyLength; ++i) {
    tmp_key[i] = key[i] ^ 0x5C;
  }
  memset(tmp_key + keyLength, 0x5C, 64 - keyLength);

  // Compute outer digest
  sha1_init(&ctx);
  sha1_update(&ctx, tmp_key, 64);
  sha1_update(&ctx, sha, SHA1_DIGEST_LENGTH);
  sha1_final(&ctx, sha);

  // Copy result to output buffer and truncate or pad as necessary
  memset(result, 0, resultLength);
  if (resultLength > SHA1_DIGEST_LENGTH) {
    resultLength = SHA1_DIGEST_LENGTH;
  }
  memcpy(result, sha, resultLength);

  // Zero out all internal data structures
  memset(hashed_key, 0, sizeof(hashed_key));
  memset(sha, 0, sizeof(sha));
  memset(tmp_key, 0, sizeof(tmp_key));
}
