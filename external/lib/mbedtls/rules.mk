LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_SRCS += \
	$(LOCAL_DIR)/library/aes.c \
	$(LOCAL_DIR)/library/aesni.c \
	$(LOCAL_DIR)/library/aesce.c \
	$(LOCAL_DIR)/library/aria.c \
	$(LOCAL_DIR)/library/asn1parse.c \
	$(LOCAL_DIR)/library/asn1write.c \
	$(LOCAL_DIR)/library/base64.c \
	$(LOCAL_DIR)/library/bignum.c \
	$(LOCAL_DIR)/library/bignum_core.c \
	$(LOCAL_DIR)/library/bignum_mod.c \
	$(LOCAL_DIR)/library/bignum_mod_raw.c \
	$(LOCAL_DIR)/library/camellia.c \
	$(LOCAL_DIR)/library/ccm.c \
	$(LOCAL_DIR)/library/chacha20.c \
	$(LOCAL_DIR)/library/chachapoly.c \
	$(LOCAL_DIR)/library/cipher.c \
	$(LOCAL_DIR)/library/cipher_wrap.c \
	$(LOCAL_DIR)/library/cmac.c \
	$(LOCAL_DIR)/library/constant_time.c \
	$(LOCAL_DIR)/library/ctr_drbg.c \
	$(LOCAL_DIR)/library/des.c \
	$(LOCAL_DIR)/library/dhm.c \
	$(LOCAL_DIR)/library/ecdh.c \
	$(LOCAL_DIR)/library/ecdsa.c \
	$(LOCAL_DIR)/library/ecjpake.c \
	$(LOCAL_DIR)/library/ecp.c \
	$(LOCAL_DIR)/library/ecp_curves.c \
	$(LOCAL_DIR)/library/ecp_curves_new.c \
	$(LOCAL_DIR)/library/entropy.c \
	$(LOCAL_DIR)/library/entropy_poll.c \
	$(LOCAL_DIR)/library/error.c \
	$(LOCAL_DIR)/library/gcm.c \
	$(LOCAL_DIR)/library/hkdf.c \
	$(LOCAL_DIR)/library/hmac_drbg.c \
	$(LOCAL_DIR)/library/lmots.c \
	$(LOCAL_DIR)/library/lms.c \
	$(LOCAL_DIR)/library/md.c \
	$(LOCAL_DIR)/library/md5.c \
	$(LOCAL_DIR)/library/memory_buffer_alloc.c \
	$(LOCAL_DIR)/library/nist_kw.c \
	$(LOCAL_DIR)/library/oid.c \
	$(LOCAL_DIR)/library/padlock.c \
	$(LOCAL_DIR)/library/pem.c \
	$(LOCAL_DIR)/library/pk.c \
	$(LOCAL_DIR)/library/pk_wrap.c \
	$(LOCAL_DIR)/library/pkcs12.c \
	$(LOCAL_DIR)/library/pkcs5.c \
	$(LOCAL_DIR)/library/pkparse.c \
	$(LOCAL_DIR)/library/pkwrite.c \
	$(LOCAL_DIR)/library/platform.c \
	$(LOCAL_DIR)/library/platform_util.c \
	$(LOCAL_DIR)/library/poly1305.c \
	$(LOCAL_DIR)/library/psa_crypto.c \
	$(LOCAL_DIR)/library/psa_crypto_aead.c \
	$(LOCAL_DIR)/library/psa_crypto_cipher.c \
	$(LOCAL_DIR)/library/psa_crypto_client.c \
	$(LOCAL_DIR)/library/psa_crypto_driver_wrappers_no_static.c \
	$(LOCAL_DIR)/library/psa_crypto_ecp.c \
	$(LOCAL_DIR)/library/psa_crypto_ffdh.c \
	$(LOCAL_DIR)/library/psa_crypto_hash.c \
	$(LOCAL_DIR)/library/psa_crypto_mac.c \
	$(LOCAL_DIR)/library/psa_crypto_pake.c \
	$(LOCAL_DIR)/library/psa_crypto_rsa.c \
	$(LOCAL_DIR)/library/psa_crypto_se.c \
	$(LOCAL_DIR)/library/psa_crypto_slot_management.c \
	$(LOCAL_DIR)/library/psa_crypto_storage.c \
	$(LOCAL_DIR)/library/psa_its_file.c \
	$(LOCAL_DIR)/library/psa_util.c \
	$(LOCAL_DIR)/library/ripemd160.c \
	$(LOCAL_DIR)/library/rsa.c \
	$(LOCAL_DIR)/library/rsa_alt_helpers.c \
	$(LOCAL_DIR)/library/sha1.c \
	$(LOCAL_DIR)/library/sha256.c \
	$(LOCAL_DIR)/library/sha512.c \
	$(LOCAL_DIR)/library/sha3.c \
	$(LOCAL_DIR)/library/threading.c \
	$(LOCAL_DIR)/library/timing.c \
	$(LOCAL_DIR)/library/version.c \
	$(LOCAL_DIR)/library/version_features.c \
	# This line is intentionally left blank

include make/module.mk
