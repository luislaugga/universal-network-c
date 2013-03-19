/*
* Copyright (cc) 2012 Luis Laugga. Some rights reserved, all wrongs deserved.
* Licensed under a Creative Commons Attribution, Share Alike 3.0 Unported License (CC BY-SA 3.0).
*
* stream.c
* universal-network-c
*/

#include "stream.h"
#include "stream_internal.h"

#define kStreamListCapacity 5

#pragma mark -
#pragma mark Setup

NetError streamSetup(StreamConfiguration * config, const unsigned int port, void * context, StreamUpdateCallback updateCallback, StreamReceiveCallback receiveCallback, StreamTimeoutCallback timeoutCallback, StreamSuspendCallback suspendCallback)
{	
	// Socket
	NetError socketError = NetNoError;
	config->socket = net_socket_create(&socketError, AF_INET, "0.0.0.0", port); // "0.0.0.0" tistening on all network interfaces
	if(socketError) // Check error
	{
		mNetworkLog("Error creating socket (NetError %d)", socketError);
		return socketError;
	}
	net_socket_set_receive_callback(config->socket, config, streamSocketReceiveCallback); // Set socket callback
    
    // Address
    net_socket_local_addr(config->socket, &config->address);
	
	// Callbacks
	config->updateCallback = updateCallback;
	config->receiveCallback = receiveCallback;
	config->timeoutCallback = timeoutCallback;
    config->suspendCallback = suspendCallback;
	config->context = context;
	if(!config->updateCallback || !config->receiveCallback || !config->timeoutCallback) // Check null callbacks
	{
		mNetworkLog("Error invalid stream callbacks (NetError %d)", NetInvalidError);
		return NetInvalidError;
	}
	
	// Dispatch queue 
	config->streamDispatchQueue = dispatch_queue_create("com.laugga.streamDispatchQueue", NULL);
    
	// Dispatch timer
	config->streamDispatchTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, config->streamDispatchQueue);    
    dispatch_source_set_timer(config->streamDispatchTimer, dispatch_time(DISPATCH_TIME_NOW, 0), kStreamTimerUpdateInterval * NSEC_PER_SEC, kStreamTimerUpdateInterval * NSEC_PER_SEC);
	dispatch_set_context(config->streamDispatchTimer, config);    
	dispatch_source_set_event_handler_f(config->streamDispatchTimer, &streamTimerCallback);
	
	// Start inactive by default
	config->active = false;
	
	// Streams list
	config->streams = list_create(kStreamListCapacity); // Start with initial capacity of kStreamListCapacity

	return NetNoError;
}

void streamTeardown(StreamConfiguration * config)
{
	dispatch_release(config->streamDispatchQueue);
	dispatch_source_cancel(config->streamDispatchTimer); // Atomic, guarantees the event handler will not fire again 				
	if(config->active == false) 						 // after the source is resumed in case it is suspended
		dispatch_resume(config->streamDispatchTimer);
	dispatch_release(config->streamDispatchTimer);
	list_destroy(config->streams);
	net_socket_destroy(config->socket);
}

#pragma mark -
#pragma mark Stream

Stream * streamCreate(const net_addr_t * address)
{
	Stream * stream = (Stream *)malloc(sizeof(Stream));
	if(stream)
	{
		memset(stream, 0, sizeof(Stream)); // reset (all values to zero)
		
		stream->state = StreamWaiting; // Waiting state, until getting back from address
		streamReliabilityClear(&stream ->reliability);
		streamFlowClear(&stream ->flow);
		net_addr_copy(&stream->address, address); // address
	}
	
	return stream;
}

void streamDestroy(Stream ** stream)
{
	if(*stream)
	{
		free(*stream);
		*stream = NULL;
	}	
}

bool streamUpdate(StreamConfiguration * config, Stream * stream)
{
	streamReliabilityUpdate(&stream->reliability, kStreamTimerUpdateInterval);
	streamFlowUpdate(&stream->flow, stream->reliability.rtt, kStreamTimerUpdateInterval);
	
	stream->timeoutAccumulator += kStreamTimerUpdateInterval;
	
	if(stream->timeoutAccumulator > kStreamTimeout)
	{
		streamTimeout(config, stream);
	}
	
	stream->updateAccumulator += kStreamTimerUpdateInterval;
	
	if(stream->updateAccumulator >= stream->flow.updateInterval)
	{
		stream->updateAccumulator = 0.0f;
		
		return true; // needs update
	}
	
	return false;
}

void streamSend(StreamConfiguration * config, Stream * stream, StreamObject * object)
{
	if(stream->state == StreamConnected || stream->state == StreamWaiting) 
	{
        //mNetworkLog("Stream send to...");
            
		net_packet_t packet = net_packet_alloc(config->socket);
		
		if(packet)
		{
			bitstream_t * bitstream = &packet->bitstream;
	
			// Pack Header
			streamProtocolPackHeader(bitstream, stream->reliability.sequence, stream->reliability.ack, stream->reliability.ackBits);
	
			// Pack Data
			streamProtocolPackData(bitstream, object);
	
			// Set packet stream remote addresss
			net_packet_addr(packet, &stream->address);
	        //mNetworkLog("Send from %d to %d", net_addr_get_port(&config->address), net_addr_get_port(&stream->address));
		
			// Send stream packet
			net_socket_send(config->socket, packet);
	
			// Mark as sent
			streamReliabilityPacketSent(&stream->reliability, packet->length);
	
			// Release packet
			net_packet_release(config->socket, packet);
		}
	}
}

void streamReceive(StreamConfiguration * config, Stream * stream, Sequence sequence, Ack ack, AckBitField ackBitField, StreamObject * object)
{
    // Check ack is less than last sent packet sequence
	if(ack <= stream->reliability.sequence)
	{ 
		// Reset timeout accumulator
		stream->timeoutAccumulator = 0.0f;
	
		// Mark as received
		streamReliabilityPacketReceived(&stream->reliability, sequence, ack, ackBitField);
	
		// Forward object
		config->receiveCallback(config->context, &stream->address, &object->bitstream);
        
	}
}

void streamTimeout(StreamConfiguration * config, Stream * stream)
{
	dispatch_async(config->streamDispatchQueue, ^{
		stream->state = StreamTimeout;
        mNetworkLog("Stream timeout");
        net_addr_log(&stream->address);
		config->timeoutCallback(config->context, &stream->address); // forward
	});
}

void streamLog(Stream * stream)
{
	mNetworkLog("\nrtt %.1fms, sent %d, acked %d, lost %d (%.1f%%), sent bandwidth = %.1fkbps, acked bandwidth = %.1fkbps, flow = %s\n", 
					stream->reliability.rtt * 1000.0f, 
					stream->reliability.totalSentPackets, 
					stream->reliability.totalAckedPackets, 
					stream->reliability.totalLostPackets, 
					stream->reliability.totalSentPackets > 0.0f ? (float) stream->reliability.totalLostPackets / (float) stream->reliability.totalSentPackets * 100.0f : 0.0f, 
					stream->reliability.sentBandwidth, 
					stream->reliability.ackedBandwidth, 
					stream->flow.mode == StreamFlowModeGood ? "good" : "bad");
}

#pragma mark -
#pragma mark Pause/Resume

void streamSuspend(StreamConfiguration * config)
{
    mNetworkPrettyLog;
    
	//dispatch_async(config->streamDispatchQueue, ^{
		// Always suspend
		if(config->active == true)
		{
			dispatch_suspend(config->streamDispatchTimer);
			config->active = false;
            
            // Remove all streams		
//            list_iterate(config->streams, ^(list_object_t object){
//                Stream * stream = (Stream *)object;
//                list_remove(config->streams, stream);		
//            });
            
            // callback
            config->suspendCallback(config->context);
		}
	//});
}

void streamResume(StreamConfiguration * config)
{
	//dispatch_async(config->streamDispatchQueue, ^{
		if(config->active == false)
		{
			// Resume only if stream list is not empty
			if(list_is_empty(config->streams) == false)
			{
				config->active = true;
				dispatch_resume(config->streamDispatchTimer);	
			}
		}
	//});
}

#pragma mark -
#pragma mark Add/Remove 

void streamAdd(StreamConfiguration * config, const net_addr_t * streamRemoteAddress)
{
    // Copy address (operation is asynchronous)
    net_addr_t streamAddress;
    net_addr_copy(&streamAddress, streamRemoteAddress); // copy to stack variable, to be captured by the block
   
    mNetworkLog("streamAdd %s:%d", inet_ntoa(streamAddress.sin_addr), ntohs(streamAddress.sin_port));
    
    dispatch_async(config->streamDispatchQueue, ^{
        
        // Find
		Stream * stream = list_find(config->streams, ^(list_object_t object){
			Stream * _stream = (Stream *)object;
			return net_addr_is_equal(&_stream->address, &streamAddress);
		});
        
        // Create and add if doesn't exist yet
		if(!stream)
		{
            // Create
			stream = streamCreate(&streamAddress);
            
            // Add
			list_add(config->streams, stream);
            
            // Log...
            mNetworkLog("Added remote stream %d to local %d", net_addr_get_port(&stream->address), net_addr_get_port(&config->address));
            net_addr_log(&stream->address);
            
            // Resume timer when list has at least one
            streamResume(config);
		}
    });
}

void streamRemove(StreamConfiguration * config, const net_addr_t * streamRemoteAddress)
{
    // Copy address (operation is asynchronous)
    net_addr_t streamAddress;
    net_addr_copy(&streamAddress, streamRemoteAddress);
    
	dispatch_async(config->streamDispatchQueue, ^{
    
        // Find
        Stream * stream = list_find(config->streams, ^(list_object_t object){
			Stream * _stream = (Stream *)object;
			return net_addr_is_equal(&_stream->address, &streamAddress);
		});
        
        // Remove if exists
        if(stream)
        {
            mNetworkLog("Remove stream from list with address:");
            net_addr_log((net_addr_t*)&streamAddress);
            
            list_remove(config->streams, stream);
            
            // Suspend timer when list is empty
            if(list_is_empty(config->streams))
                streamSuspend(config);
        }
        else 
        {
            mNetworkLog("No stream found for address:");
            net_addr_log((net_addr_t*)&streamAddress);
        }
	});
}

bool streamDoesExist(StreamConfiguration * config, const net_addr_t * streamRemoteAddress)
{
	Stream * stream = list_find(config->streams, ^(list_object_t object){
		Stream * _stream = (Stream *)object;
		return net_addr_is_equal(&_stream->address, streamRemoteAddress);
	});
		
	return (stream != NULL);
}

bool streamListIsEmpty(StreamConfiguration * config)
{
	return list_is_empty(config->streams);
}

#pragma mark -
#pragma mark Timer Update

void streamTimerCallback(void * context)
{
	StreamConfiguration * config = (StreamConfiguration *)context;
	if(!list_is_empty(config->streams))
	{	
		// Flag indicates if all streams need to update data
		// Baseline is the stream with lower update frequency
		__block bool syncUpdate = true;
	
		// Update each stream		
		list_iterate(config->streams, ^(list_object_t object){
			Stream * stream = (Stream *)object;
			bool streamShouldUpdate = streamUpdate(config, stream);
			syncUpdate &= streamShouldUpdate; // all streams need to be true in order to update data		
		});
    
		// Send data
		if(syncUpdate)
		{
            //mNetworkLog("Retrieve + Send %d", net_addr_get_port(&config->address));
            
			// Retrieve application update data
			StreamObject updateObject;
            streamObjectSetup(&updateObject);
			config->updateCallback(config->context, &updateObject.bitstream); // Use bitstream to pack data
            StreamObject * updateObjectPtr = &updateObject;
			
            //mNetworkLog("Got data %d", net_addr_get_port(&config->address));
            
			// Send data for each stream
			list_iterate(config->streams, ^(list_object_t object){
                //mNetworkLog("Iterate %d", net_addr_get_port(&config->address));
				Stream * stream = (Stream *)object;
				streamSend(config, stream, updateObjectPtr);
			});
		}
	
		// Log status enabled in DEBUG only
#if (DEBUG|TEST)
		config->logAccumulator += kStreamTimerUpdateInterval;
		if(config->logAccumulator > kStreamLogStatusInterval)
		{
			config->logAccumulator = 0.0f;
			
			list_iterate(config->streams, ^(list_object_t object){
				Stream * stream = (Stream *)object;
				streamLog(stream);
			});
		}
#endif
	}
}

#pragma mark -
#pragma mark Socket Receive

void streamSocketReceiveCallback(void * context, net_packet_t packet)
{	 
	StreamConfiguration * config = (StreamConfiguration *)context;
	dispatch_async(config->streamDispatchQueue, ^{
        
		bitstream_t * bitstream = &packet->bitstream;
	
		Sequence sequence;
		Ack ack;
		AckBitField ackBitField;
	
		if(streamProtocolUnpackHeader(bitstream, &sequence, &ack, &ackBitField) == UnpackValid) // Only proceed if valid
		{
			Stream * stream = list_find(config->streams, ^(list_object_t object){
				Stream * _stream = (Stream *)object;
				return net_addr_is_equal(&_stream->address, &packet->addr);
			});
            
            if(stream == NULL)
            {
                streamAdd(config, &packet->addr); // TODO improve...
            }
			
			if(stream)
			{
				StreamObject receiveObject;
                streamObjectSetup(&receiveObject);
				streamProtocolUnpackData(bitstream, &receiveObject); // unpack	
                
				streamReceive(config, stream, sequence, ack, ackBitField, &receiveObject); // set received
			}
		}
			
		net_packet_release(config->socket, packet); // release packet
	});

}