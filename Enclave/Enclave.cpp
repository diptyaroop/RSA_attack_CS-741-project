/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char* fmt, ...)
{
    /*char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;*/
	return 0;
}

int print(char *str, char msg)
{
	ocall_print_values2(str,msg);
	return 0;
}

void ecall_generate_RSA_key(void)
{
	printf("%s\n", "start");
	BIGNUM *pub_exp = BN_new();
	const BIGNUM *d,*e,*n,*p,*q;
	int ret = BN_set_word(pub_exp,RSA_F4);
//	int ret = BN_set_word(pub_exp,3);
	if (!ret)
	{
		// print error message
	}
   	RSA *rsa = RSA_new();
   	RSA_generate_key_ex(rsa,8192,pub_exp,NULL);
	RSA_get0_key(rsa,&n,&e,&d);
	RSA_get0_factors(rsa,&p,&q);

	char *tmpn = BN_bn2hex(n);
	print(tmpn,'n');

	char *tmpe = BN_bn2hex(e);
	print(tmpe,'e');

	char *tmpd = BN_bn2hex(d);
	print(tmpd,'d');

	char *tmpp = BN_bn2hex(p);
	print(tmpp,'p');

	char *tmpq = BN_bn2hex(q);
	print(tmpq,'q');
	printf("%s\n", "end");
}
