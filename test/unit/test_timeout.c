/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* test_timeout.c
* universal-network-c
*/

#include "test.h"
#include "timeout.h"

static int timeoutDidFire = 0;

void test_timeout_callback()
{
	timeoutDidFire = 1;
	//mNetworkLog("fire timeout timestamp: %lu", time(NULL));
}

static void test_timeout()
{
	LOG_TEST_START;

	timeout_t test_timeout;
	timeout_create(&test_timeout, test_timeout_callback, NULL, 2000);
	
	sleep(3);
	
	assert(timeoutDidFire);

	timeout_destroy(&test_timeout);

	LOG_TEST_END;
}

static void test_timeout_block()
{
	LOG_TEST_START;
	
	__block int blockTimeoutDidFire = 0;
	
	timeout_block_t test_timeoutBlock = ^{
		//mNetworkLog("fire timeout timestamp: %lu", time(NULL));
		blockTimeoutDidFire = 1;
	};

	timeout_t test_timeout;
	timeout_create_block(&test_timeout, test_timeoutBlock, 1000);
	
	sleep(2);
	
	assert(blockTimeoutDidFire);

	timeout_destroy(&test_timeout);

	LOG_TEST_END;
}

static void test_timeout_queue()
{
	LOG_TEST_START;
	
	__block int blockTimeoutDidFire = 0;
	
	timeout_block_t test_timeoutBlock = ^{
		blockTimeoutDidFire = 1;
	};

  dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

	timeout_t test_timeout;
	timeout_create_queue(&test_timeout, queue, 1000, test_timeoutBlock);
	
	sleep(2);
	
	assert(blockTimeoutDidFire);

	timeout_destroy(&test_timeout);

	LOG_TEST_END;
}


static void test_timeout_create_destroy()
{
	LOG_TEST_START;
	
	const long timeoutValue = 500; // 1/2 second
	
	__block bool blockTimeoutDidFire = false;
	
	timeout_block_t test_timeoutBlock = ^{
			//mNetworkLog("fire timeout timestamp: %lu", time(NULL));
			blockTimeoutDidFire = true;
	};
	
	for(int i=0; i<10; ++i)
	{
		timeout_t test_timeout;	
		timeout_create_block(&test_timeout, test_timeoutBlock, timeoutValue); // create
		
		if(i%2 == 0)
		{
			usleep(timeoutValue * 1.2 * 1000); // should fire
			assert(blockTimeoutDidFire == true);
			assert(test_timeout.valid == false); // check it autoreleased after fire
		}
		else
		{
			usleep(timeoutValue*0.1*1000); // should not fire
			assert(blockTimeoutDidFire == false);
			assert(test_timeout.valid == true); // check it didn't autorelease before fire
			
		}
			
		timeout_destroy(&test_timeout); // destroy
		blockTimeoutDidFire = false; // reset
	}

	LOG_TEST_END;
}

int main(void)
{
	LOG_SUITE_START("timeout");

	test_timeout();
	test_timeout_block();
	test_timeout_queue();
	test_timeout_create_destroy();
	
	return 0;
}
