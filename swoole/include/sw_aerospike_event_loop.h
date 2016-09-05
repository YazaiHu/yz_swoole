//
//  swoole_aerospike.h
//  yz-swoole
//
//  Created by 石伟 on 16/4/21.
//  Copyright (c) 2016年 石伟. All rights reserved.
//

#ifndef sw_aerospike_event_loop_h
#define sw_aerospike_event_loop_h

#ifdef SW_USE_AEROSPIKE

#include "aerospike/as_event_swoole.h"
#include "aerospike/aerospike.h"
#include "aerospike/aerospike_key.h"
#include "aerospike/as_status.h"
#include "aerospike/as_event_internal.h"
#include "aerospike/as_admin.h"
#include "aerospike/as_async.h"
#include "aerospike/as_log_macros.h"
#include "aerospike/as_pipe.h"
#include "aerospike/as_proto.h"
#include "aerospike/as_socket.h"
#include "aerospike/as_string.h"
#include "aerospike/as_record.h"
#include "aerospike/as_record_iterator.h"
#include "aerospike/as_arraylist.h"
#include "aerospike/as_hashmap.h"
#include "aerospike/as_stringmap.h"
#include "aerospike/as_hashmap_iterator.h"
#include "aerospike/as_arraylist_iterator.h"
#include "aerospike/as_boolean.h"
#include "aerospike/as_nil.h"
#include "citrusleaf/alloc.h"
#include "citrusleaf/cf_byte_order.h"

//声明aerospike的回调函数
void init_aerospike_callback(as_event_swoole_callback* as_event_callback);
void uninit_aerospike_callback(as_event_swoole_callback* as_event_callback);


typedef void (*swoole_as_event_timeout_callback)(void* data);

typedef struct {
    swoole_as_event_timeout_callback cb;
    void * data;
} asTimeCallBack;

#endif

#endif
