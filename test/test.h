/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test.h
* universal-network-c
*/

#ifndef __universal_network_test_h__
#define __universal_network_test_h__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <Block.h>

#include "network.h"

#define LOG_SUITE_START(suitename) printf("Testing %s\n", suitename)
#define LOG_TEST_START //printf("Starting: %s\n", __PRETTY_FUNCTION__)
#define LOG_TEST_END printf("[OK] %s\n", __PRETTY_FUNCTION__)

#endif