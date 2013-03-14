#
# Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
# Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
#
# test/Makefile.am
# lBackend
#

testdir = $(top_srcdir)/build/test

TESTS = \
	test_pool \
 	test_queue \
	test_list \
	test_net \
	test_bitstream \
	test_timeout \
	test_hashtable \
	test_protocol \
	test_stream_flow \
	test_stream_reliability \
	test_stream_protocol \
	test_transaction_protocol \
	test_net_socket \
	test_stream \
	test_transaction \
	test_stun \
				 
check_PROGRAMS = \
	test_pool \
 	test_queue \
	test_list \
	test_net \
	test_bitstream \
	test_timeout \
	test_hashtable \
	test_protocol \
	test_stream_flow \
	test_stream_reliability \
	test_stream_protocol \
	test_transaction_protocol \
	test_net_socket \
	test_stream \
	test_transaction \
	test_stun \
				 
AM_CPPFLAGS = -DTEST -I$(top_srcdir)/src
	
SOURCES = \
	$(top_srcdir)/src/protocol.c \
	$(top_srcdir)/src/transaction.c \
	$(top_srcdir)/src/transaction_protocol.c \
	$(top_srcdir)/src/stream.c \
	$(top_srcdir)/src/stream_flow.c \
	$(top_srcdir)/src/stream_reliability.c \
	$(top_srcdir)/src/stream_protocol.c \
	$(top_srcdir)/src/net_error.c \
	$(top_srcdir)/src/net_addr.c \
	$(top_srcdir)/src/net_socket.c \
	$(top_srcdir)/src/net_packet.c \
	$(top_srcdir)/src/pool.c \
	$(top_srcdir)/src/queue.c \
	$(top_srcdir)/src/list.c \
	$(top_srcdir)/src/bitstream.c \
	$(top_srcdir)/src/hashtable.c \
	$(top_srcdir)/src/timeout.c \
	$(top_srcdir)/src/huid.c 
		
STUN_SOURCES = \
	$(top_srcdir)/src/stun.c \
	$(top_srcdir)/src/stun_protocol.c \
	$(top_srcdir)/src/stun_tests.c \
	$(top_srcdir)/src/stun_utils.c

test_pool_SOURCES = unit/test_pool.c $(SOURCES) $(STUN_SOURCES)
test_queue_SOURCES = unit/test_queue.c $(SOURCES) $(STUN_SOURCES)
test_list_SOURCES = unit/test_list.c $(SOURCES) $(STUN_SOURCES)
test_net_SOURCES = unit/test_net.c $(NET_SOURCES) $(SOURCES) $(STUN_SOURCES)
test_bitstream_SOURCES = unit/test_bitstream.c $(SOURCES) $(STUN_SOURCES)
test_timeout_SOURCES = unit/test_timeout.c $(SOURCES) $(STUN_SOURCES)
test_protocol_SOURCES = unit/test_protocol.c $(SOURCES) $(STUN_SOURCES)
test_hashtable_SOURCES = unit/test_hashtable.c $(SOURCES) $(STUN_SOURCES)
test_stream_flow_SOURCES = unit/test_stream_flow.c $(SOURCES) $(STUN_SOURCES)
test_stream_reliability_SOURCES = unit/test_stream_reliability.c $(SOURCES) $(STUN_SOURCES)
test_stream_protocol_SOURCES = unit/test_stream_protocol.c $(SOURCES) $(STUN_SOURCES)
test_transaction_protocol_SOURCES = unit/test_transaction_protocol.c $(SOURCES) $(STUN_SOURCES)
test_huid_SOURCES = unit/test_huid.c $(SOURCES)

test_net_socket_SOURCES = functional/test_net_socket.c $(SOURCES) $(STUN_SOURCES)
test_transaction_SOURCES = functional/test_transaction.c $(SOURCES) $(STUN_SOURCES)
test_stream_SOURCES = functional/test_stream.c $(SOURCES) $(STUN_SOURCES)
test_stun_SOURCES = functional/test_stun.c $(SOURCES) $(STUN_SOURCES)