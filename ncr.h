#ifndef L_NCR_H
#define L_NCR_H

#include <linux/types.h>
#ifndef __KERNEL__
#define __user
#endif

/* Serves to make sure the structure is suitably aligned to continue with
   a struct nlattr without external padding.

   4 is NLA_ALIGNTO from <linux/netlink.h>, but if we
   included <linux/netlink.h>, the user would have to include <sys/socket.h>
   as well for no obvious reason.  "4" is fixed by ABI. */
#define __NL_ATTRIBUTES char __align[] __attribute__((aligned(4)))

/* In all ioctls, input_size specifies size of the ncr_* structure and the
   following attributes.

   output_size specifies space available for returning output, including the
   initial ncr_* structure, and is updated by the ioctl() with the space
   actually used.

   There are two special cases: input_size 0 means not attributes are supplied,
   and is treated equivalent to sizeof(struct ncr_*).  output_size 0 means no
   space for output attributes is available, and is not updated. */

/* FIXME: better names for algorithm parameters? */
/* FIXME: Split key generation/derivation attributes to decrease the number
   of attributes used for the frequent operations? */
enum {
	NCR_ATTR_UNSPEC,	      /* 0 is special in lib/nlattr.c. */
	/* FIXME: Use NLA_STRING for this, later */
	NCR_ATTR_ALGORITHM,	      /* NLA_U32 - ncr_algorithm_t */
	NCR_ATTR_DERIVATION_ALGORITHM, /* NLA_U32 - ncr_algorithm_t */
	NCR_ATTR_WRAPPING_ALGORITHM,  /* NLA_U32 - ncr_wrap_algorithm_t */
	NCR_ATTR_KEY_FLAGS,	      /* NLA_U32 - NCR_KEY_FLAG_* */
	NCR_ATTR_KEY_ID,	      /* NLA_BINARY */
	NCR_ATTR_KEY_TYPE,	      /* NLA_U32 - ncr_key_type_t */
	NCR_ATTR_IV,		      /* NLA_BINARY */
	NCR_ATTR_SECRET_KEY_BITS,     /* NLA_U32 */
	NCR_ATTR_RSA_MODULUS_BITS,    /* NLA_U32 */
	NCR_ATTR_RSA_E,		      /* NLA_BINARY */
	NCR_ATTR_DSA_P_BITS,	      /* NLA_U32 */
	NCR_ATTR_DSA_Q_BITS,	      /* NLA_U32 */
	NCR_ATTR_DH_PRIME,	      /* NLA_BINARY */
	NCR_ATTR_DH_BASE,	      /* NLA_BINARY */
	NCR_ATTR_DH_PUBLIC,	      /* NLA_BINARY */
	NCR_ATTR_WANTED_ATTRS,	      /* NLA_BINARY - array of u16 IDs */

	/* Add new attributes here */

	NCR_ATTR_END__,
	NCR_ATTR_MAX = NCR_ATTR_END__ - 1
};

#define NCR_CIPHER_MAX_BLOCK_LEN 32
#define NCR_HASH_MAX_OUTPUT_SIZE  64

typedef enum {
	NCR_ALG_NONE,
	NCR_ALG_NULL,
	NCR_ALG_3DES_CBC,
	NCR_ALG_AES_CBC,
	NCR_ALG_CAMELLIA_CBC,
	NCR_ALG_ARCFOUR,
	NCR_ALG_AES_ECB,
	NCR_ALG_CAMELLIA_ECB,
	NCR_ALG_AES_CTR,
	NCR_ALG_CAMELLIA_CTR,

	NCR_ALG_SHA1=40,
	NCR_ALG_MD5,
	NCR_ALG_SHA2_224,
	NCR_ALG_SHA2_256,
	NCR_ALG_SHA2_384,
	NCR_ALG_SHA2_512,

	NCR_ALG_HMAC_SHA1=80,
	NCR_ALG_HMAC_MD5,
	NCR_ALG_HMAC_SHA2_224,
	NCR_ALG_HMAC_SHA2_256,
	NCR_ALG_HMAC_SHA2_384,
	NCR_ALG_HMAC_SHA2_512,

	NCR_ALG_RSA=140,
	NCR_ALG_DSA,
	NCR_ALG_DH, /* DH as in PKCS #3 */
} ncr_algorithm_t;



typedef enum {
	NCR_WALG_AES_RFC3394, /* for secret keys only */
	NCR_WALG_AES_RFC5649, /* can wrap arbitrary key */
} ncr_wrap_algorithm_t;

typedef enum {
	NCR_KEY_TYPE_INVALID,
	NCR_KEY_TYPE_SECRET=1,
	NCR_KEY_TYPE_PUBLIC=2,
	NCR_KEY_TYPE_PRIVATE=3,
} ncr_key_type_t;

/* Key handling
 */

typedef __s32 ncr_key_t;

#define NCR_KEY_INVALID ((ncr_key_t)-1)

#define NCR_KEY_FLAG_EXPORTABLE 1
#define NCR_KEY_FLAG_WRAPPABLE (1<<1)
/* when generating a pair the flags correspond to private
 * and public key usage is implicit. For example when private
 * key can decrypt then public key can encrypt. If private key
 * can sign then public key can verify.
 */
#define NCR_KEY_FLAG_DECRYPT (1<<2)
#define NCR_KEY_FLAG_SIGN (1<<3)
/* This flag can only be set by administrator, to prevent
 * adversaries exporting wrappable keys with random ones.
 */
#define NCR_KEY_FLAG_WRAPPING (1<<4)

struct ncr_key_generate {
	__u32 input_size, output_size;
	ncr_key_t key;
	__NL_ATTRIBUTES;
};

struct ncr_key_generate_pair {
	__u32 input_size, output_size;
	ncr_key_t private_key;
	ncr_key_t public_key;
	__NL_ATTRIBUTES;
};

typedef enum {
	RSA_PKCS1_V1_5, /* both signatures and encryption */
	RSA_PKCS1_OAEP, /* for encryption only */
	RSA_PKCS1_PSS, /* for signatures only */
} ncr_rsa_type_t;

/* used in encryption
 */
struct ncr_key_params_st {
	/* this structure always corresponds to a key. Hence the
	 * parameters of the union selected are based on the corresponding
	 * key */
	union {
		struct {
			__u8 iv[NCR_CIPHER_MAX_BLOCK_LEN];
			__kernel_size_t iv_size;
		} cipher;
		struct {
			ncr_rsa_type_t type;
			ncr_algorithm_t oaep_hash; /* for OAEP */
			ncr_algorithm_t sign_hash; /* for signatures */
			unsigned int pss_salt; /* PSS signatures */
		} rsa;
		struct {
			ncr_algorithm_t sign_hash; /* for signatures */
		} dsa;
	} params;
};

typedef enum {
	NCR_DERIVE_DH=1,
} ncr_derive_t;


struct ncr_key_derive {
	__u32 input_size, output_size;
	ncr_key_t input_key;
	ncr_key_t new_key;
	__NL_ATTRIBUTES;
};

#define MAX_KEY_ID_SIZE 20

struct ncr_key_get_info {
	__u32 input_size, output_size;
	ncr_key_t key;
	__NL_ATTRIBUTES;
};

struct ncr_key_import {
	__u32 input_size, output_size;
	ncr_key_t key;
	const void __user *data;
	__u32 data_size;
	__NL_ATTRIBUTES;
};

struct ncr_key_export {
	__u32 input_size, output_size;
	ncr_key_t key;
	void __user *buffer;
	int buffer_size;
	__NL_ATTRIBUTES;
};

#define NCRIO_KEY_INIT			_IO('c', 204)
/* generate a secret key */
#define NCRIO_KEY_GENERATE     	_IOWR('c', 205, struct ncr_key_generate)
/* generate a public key pair */
#define NCRIO_KEY_GENERATE_PAIR _IOWR('c', 206, struct ncr_key_generate_pair)
/* derive a new key from an old one */
#define NCRIO_KEY_DERIVE        _IOWR('c', 207, struct ncr_key_derive)
/* return information on a key */
#define NCRIO_KEY_GET_INFO      _IOWR('c', 208, struct ncr_key_get_info)

/* export a secret key */
#define NCRIO_KEY_EXPORT       	_IOWR('c', 209, struct ncr_key_export)
/* import a secret key */
#define NCRIO_KEY_IMPORT       	_IOWR('c', 210, struct ncr_key_import)

#define NCRIO_KEY_DEINIT       _IOR ('c', 215, ncr_key_t)

/* Key wrap ioctls
 */
struct ncr_key_wrap {
	__u32 input_size, output_size;
	ncr_key_t wrapping_key;
	ncr_key_t source_key;
	void __user *buffer;
	int buffer_size;
	__NL_ATTRIBUTES;
};

struct ncr_key_unwrap {
	__u32 input_size, output_size;
	ncr_key_t wrapping_key;
	ncr_key_t dest_key;
	const void __user *data;
	__u32 data_size;
	__NL_ATTRIBUTES;
};

#define NCRIO_KEY_WRAP        _IOWR('c', 250, struct ncr_key_wrap)
#define NCRIO_KEY_UNWRAP        _IOWR('c', 251, struct ncr_key_unwrap)

/* Internal ops  */
struct ncr_master_key_st {
	__u8 __user * key;
	__u16 key_size;
};

#define NCRIO_MASTER_KEY_SET        _IOR ('c', 260, struct ncr_master_key_st)

/* These are similar to key_wrap and unwrap except that will store some extra
 * fields to be able to recover a key */
struct ncr_key_storage_wrap {
	__u32 input_size, output_size;
	ncr_key_t key;
	void __user *buffer;
	int buffer_size;
	__NL_ATTRIBUTES;
};

struct ncr_key_storage_unwrap {
	__u32 input_size, output_size;
	ncr_key_t key;
	const void __user *data;
	__u32 data_size;
	__NL_ATTRIBUTES;
};

#define NCRIO_KEY_STORAGE_WRAP        _IOWR('c', 261, struct ncr_key_storage_wrap)
#define NCRIO_KEY_STORAGE_UNWRAP        _IOWR('c', 262, struct ncr_key_storage_wrap)

/* Crypto Operations ioctls
 */

typedef enum {
	NCR_OP_ENCRYPT=1,
	NCR_OP_DECRYPT,
	NCR_OP_SIGN,
	NCR_OP_VERIFY,
} ncr_crypto_op_t;

typedef __s32 ncr_session_t;
#define NCR_SESSION_INVALID ((ncr_session_t)-1)

/* input of CIOCGSESSION */
struct ncr_session_st {
	/* input */
	ncr_algorithm_t algorithm;

	ncr_key_t key;
	struct ncr_key_params_st params;
	ncr_crypto_op_t op;

	/* output */
	ncr_session_t	ses;		/* session identifier */
};

typedef enum {
	NCR_SUCCESS = 0,
	NCR_ERROR_GENERIC = -1,
	NCR_VERIFICATION_FAILED = -2,
} ncr_error_t;

typedef enum {
	NCR_KEY_DATA,
	NCR_DIRECT_DATA,
} ncr_data_type_t;

struct ncr_session_op_st {
	/* input */
	ncr_session_t ses;

	union {
		struct {
			ncr_key_t input;
			void __user * output;  /* when verifying signature this is
					* the place of the signature.
					*/
			__kernel_size_t output_size;
		} kdata; /* NCR_KEY_DATA */
		struct {
			void __user * input;
			__kernel_size_t input_size;
			void __user * output;
			__kernel_size_t output_size;
		} udata; /* NCR_DIRECT_DATA */
	} data;
	ncr_data_type_t type;

	/* output of verification */
	ncr_error_t err;
};

struct ncr_session_once_op_st {
	struct ncr_session_st init;
	struct ncr_session_op_st op;
};

#define NCRIO_SESSION_INIT        _IOR ('c', 300, struct ncr_session_st)
#define NCRIO_SESSION_UPDATE        _IOWR ('c', 301, struct ncr_session_op_st)
#define NCRIO_SESSION_FINAL        _IOWR ('c', 302, struct ncr_session_op_st)

/* everything in one call */
#define NCRIO_SESSION_ONCE        _IOWR ('c', 303, struct ncr_session_once_op_st)

#endif
