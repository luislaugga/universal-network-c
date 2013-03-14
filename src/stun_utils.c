/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stun_utils.c
* universal-network-c
*/

#include "stun_utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if TARGET_OS_IPHONE // SecRandomCopyBytes is only available on iOS devices due to app sandboxing
    #import <Security/Security.h>
#else
    #include <fcntl.h>
    #include <errno.h>
    #include <unistd.h>

    typedef const struct __SecRandom * SecRandomRef;

    const SecRandomRef kSecRandomDefault = NULL;

    int SecRandomCopyBytes(SecRandomRef rnd, size_t count, uint8_t * bytes);

    int SecRandomCopyBytes(SecRandomRef rnd, size_t count, uint8_t * bytes) 
    {
        int fd;
        if((fd = open("/dev/urandom", O_RDONLY)) < 0)
            return -1;
      
        ssize_t bytesRead;
        uint8_t *p = bytes;
      
        do 
        {
            bytesRead = read( fd, p, count - ( p -bytes ) );
            if(bytesRead > 0)
                p += bytesRead;
        } 
        while(bytesRead > 0 || ( bytesRead < 0 && errno == EINTR ));
      
        close(fd);
      
        return bytesRead < 0 ? -1 : 0;
    }
#endif

void stunGenerateTransactionId(StunTransactionId * transactionId)
{
	if(transactionId)
	{
		uint32_t entropy = 0;

		SecRandomCopyBytes(kSecRandomDefault, sizeof(uint32_t), (uint8_t *)&entropy);
		// Alternative:
        // entropy = arc4random(); // arc4random() not available in linux

	    srand(entropy);

	    for (int b = 0; b < kStunTransactionIdLen; ++b)
	    {
	        transactionId->id[b] = (uint8_t)(rand() % 256);
	    }
	}
}

bool stunIsValidTransactionId(StunTransactionId * transactionId)
{
	uint8_t zerobytes[kStunTransactionIdLen] = {}; // zero-init
	memset(zerobytes, 0, kStunTransactionIdLen);
    return memcmp(&transactionId->id, zerobytes, kStunTransactionIdLen) != 0;
}

bool stunIsEqualTransactionId(StunTransactionId * transactionIdA, StunTransactionId * transactionIdB)
{
	return memcmp(&transactionIdA->id, &transactionIdB->id, sizeof(StunTransactionId)) == 0;
}