# RSA_attack-CS-741-project-
CS 741 Project: Study and Implementation of Single Trace Attack against RSA key generation in Intel SGX SSL


Instructions to run:

Check keysize from Enclave.cpp file, following line :
  RSA_generate_key_ex(rsa,8192,pub_exp,NULL);
Keysize is 8192 here. Change it if required.
(If keysize is changed, run Makefile)

Run ./run.sh keysize
Voila!
