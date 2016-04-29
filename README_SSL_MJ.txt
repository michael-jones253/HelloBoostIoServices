Work flow of certificate generation:
1. Generate private key first. Will ask for password. The password is
   for protecting the private key in case it gets in the wrong hands.
2. Create certificate sign request. Will ask for paraphrase.
   (use different to above private key password). The paraphrase is for use 
   with the CA (I don't know exactly what for)
3. Sign own server certificate. (create root cert). Asks for key password only.

Running server - supply server certificate, private key and private key password in context.

Running client - supply list of trusted certificates - simply copy server certificate into ca.pem. No password needed.


#  For msys/mingw
export OPENSSL_CONF=/usr/local/ssl/openssl.cnf
