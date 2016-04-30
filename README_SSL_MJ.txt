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


# Getting openssl generated certifcates to work with the C# HelloSslCSharpWithClientAuth/HelloSslClient client

I went through the following failures, before reaching success:

Github test cert made with existing hello private key.
challenge password different from before.

Server did not work with a github C# client using machine (target host) argument "mycompany.com" :
Client connected.
Certificate error: RemoteCertificateNameMismatch, RemoteCertificateChainErrors
Exception: The remote certificate is invalid according to the validation procedure.

What does work is 
--password hello --cert hello_certificate.pem --private-key hello_priv_key.pem --diffie-hellman dh1024.pem

This is because it was made from the makecert scripts ran in the HelloSslCSharpWithClientAuth project and
converted using the  mingw conversion scripts. This was a complete matching set.

Suspect:
examine cert on the hello certificate that works shows:
Subject: CN=mycompany.com

examine cert on the test github certificate that didn't work shows:
Subject: C=AU, ST=Vic, L=St Kilda, O=michael-jones253 development, OU=software, CN=Mike/emailAddress=mikej253@me.com

Next attempt:
make another certificate specifying CN (Common Name) as mycompany.com and entering '.' for email address.

This gave a remote certificate invalid exception.
Client connected.
Certificate error: RemoteCertificateChainErrors
Exception: The remote certificate is invalid according to the validation procedure

NB note the "RemoteCertificateNameMismatch" error has gone. However, the "RemoteCertificateChainErrors" is still
there.

Suspect:
The mingw generated certificate is not in the windows certificate store.
Next step: convert mingw generated certificate into a windows .cer format and install into windows store.

run:
mingw_convert_pem_to_cer.sh

Then run MMC - see HelloSslCSharpWithClientAuth/MJ_README.txt
This worked!
