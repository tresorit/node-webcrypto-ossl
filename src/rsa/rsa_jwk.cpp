#include "rsa_jwk.h"

Handle<JwkRsa> JwkRsa::From(Handle<ScopedEVP_PKEY> pkey, int &key_type) {
	LOG_FUNC();

	LOG_INFO("Check key_type");
	if (!(key_type == NODESSL_KT_PRIVATE || key_type == NODESSL_KT_PUBLIC)) {
		THROW_ERROR("Wrong value of key_type");
	}

	LOG_INFO("Check pkey");
	if (pkey == nullptr) {
		THROW_ERROR("Key value is nullptr");
	}
	if (EVP_PKEY_base_id(pkey->Get())!= EVP_PKEY_RSA) {
		THROW_ERROR("Key is not RSA type");
	}

	LOG_INFO("Create JWK Object");
	Handle<JwkRsa> jwk(new JwkRsa());

	RSA *rsa = EVP_PKEY_get1_RSA(pkey->Get());

	LOG_INFO("Convert RSA to JWK");
	jwk->type = key_type;
	LOG_INFO("Get RSA public key");
	const BIGNUM *n;
    const BIGNUM *e;
    const BIGNUM *d;
    const BIGNUM *p;
    const BIGNUM *q;
	const BIGNUM *dmp1;
    const BIGNUM *dmq1;
    const BIGNUM *iqmp;
	RSA_get0_key(rsa, &n, &e, NULL);
	RSA_get0_factors(rsa, &p, &q);
	RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);

	jwk->n = BN_dup(n);
	jwk->e = BN_dup(e);
	if (key_type == NODESSL_KT_PRIVATE) {
		LOG_INFO("Get RSA private key");
		RSA_get0_key(rsa, NULL, NULL, &d);
		jwk->p = BN_dup(p);
		jwk->q = BN_dup(q);
		jwk->d = BN_dup(d);
		jwk->dp = BN_dup(dmp1);
		jwk->dq = BN_dup(dmq1);
		jwk->qi = BN_dup(iqmp);
	}

	return jwk;
}

Handle<ScopedEVP_PKEY> JwkRsa::To(int &key_type) {
	LOG_FUNC();

	LOG_INFO("Check key_type");
	if (!(key_type == NODESSL_KT_PRIVATE || key_type == NODESSL_KT_PUBLIC)) {
		THROW_ERROR("Wrong value of key_type");
	}

	if (strcmp(this->kty, "RSA") != 0) {
		THROW_ERROR("JWK key is not RSA");
	}

	RSA* rsa_key = RSA_new();

	LOG_INFO("set public key");
	RSA_set0_key(rsa_key, BN_dup(this->n.Get()), BN_dup(this->e.Get()), NULL);

	if (key_type == NODESSL_KT_PRIVATE) {
		LOG_INFO("set private key");
		RSA_set0_key(rsa_key, NULL, NULL, BN_dup(this->d.Get()));
		RSA_set0_factors(rsa_key, BN_dup(this->p.Get()), BN_dup(this->q.Get()));
		RSA_set0_crt_params(rsa_key, BN_dup(this->dp.Get()), BN_dup(this->dq.Get()), BN_dup(this->qi.Get()));
	}

	LOG_INFO("set key");
	ScopedEVP_PKEY pkey(EVP_PKEY_new());
	EVP_PKEY_assign_RSA(pkey.Get(), rsa_key);

	return Handle<ScopedEVP_PKEY>(new ScopedEVP_PKEY(pkey));
}