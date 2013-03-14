/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stun_utils.h
* universal-network-c
*/

#ifndef __universal_network_stun_utils_h__
#define __universal_network_stun_utils_h__

#include <stdbool.h>

#include "stun_protocol.h"

void stunGenerateTransactionId(StunTransactionId *);
bool stunIsValidTransactionId(StunTransactionId *);
bool stunIsEqualTransactionId(StunTransactionId *, StunTransactionId *);

#endif