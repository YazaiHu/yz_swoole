//
//  swoole_aerospike_api.h
//  yz-swoole
//
//  Created by 石伟 on 16/5/12.
//  Copyright (c) 2016年 石伟. All rights reserved.
//

#ifndef sw_aerospike_h
#define sw_aerospike_h

#ifdef SW_USE_AEROSPIKE
#include "sw_aerospike_event_loop.h"

typedef struct {
    aerospike *as;
    bool is_init;
    char sz_iplist[16][32];   //预先分配16个host地址，因为aerospike config的 add_host是直接使用上层的地址。
    zval *object;
    zval _object;
    as_event_swoole_callback * as_event_callback;
} swAerospikeClient;

typedef struct {
    zval* cb;
    //zval* cb_data;
#if PHP_MAJOR_VERSION >= 7
    zval _cb;
    //zval _cb_data;
#endif
}swCallBackData;

int get_spilt(char *pszSrc, char *pszSplit, char *pszDest, int nDestLen);

bool set_as_key(as_key* askey, char* namespace, char* set, zval* zkey);

bool set_stringmap_value(as_hashmap* map, const char* map_key_name, zval* zvalue);

bool set_longmap_value(as_hashmap* map, long map_key, zval* zvalue);

bool set_list_value(as_list* list, zval* zvalue);

bool check_array_list(zval *zarray);

bool set_map_value(as_hashmap* map, as_val* map_key_val, zval* zvalue);

bool set_longmap_value(as_hashmap* map, long map_key, zval* zvalue);

bool set_stringmap_value(as_hashmap* map, const char* map_key_name, zval* zvalue);

bool set_list_value(as_list* list, zval* zvalue);

bool set_record_bin_array_to_as_list(as_record* asrecord, const char* bin_name, zval *zarray);

bool set_record_bin_array_to_as_map(as_record* asrecord, const char* bin_name, zval *zarray);

bool set_record_bin_array(as_record* asrecord, const char* bin_name, zval* zarray);

bool set_record_bin_object(as_record* asrecord, const char* bin_name, zval *zdata);

bool set_record_bin_long(as_record* asrecord, char* bin_name, long bin_value);

bool set_record_bin_double(as_record* asrecord, char* bin_name, double bin_value);

bool set_record_bin_string(as_record* asrecord, char* bin_name, char* bin_value, long bin_value_len);

bool set_record_bin_zval(as_record* asrecord, char* bin_name, zval* zvalue);

bool set_record(as_record* asrecord, zval* zrecord);

void add_array_by_string_key(zval* zvalue, as_string* key, as_val* value);

void add_array_by_long_key(zval* zvalue, as_integer* key, as_val* value);

void add_array_by_list_value(zval* zvalue, as_val *value);

bool parse_as_record(as_record* asrecord, zval* zrecord);

bool set_policy_write(as_policy_write* policy, zval* zpolicy_write, int* ttl);

bool set_policy_read(as_policy_read* policy, zval* zpolicy_read);

bool set_policy_remove(as_policy_remove* policy, zval* zpolicy_remove);

bool set_policy_operate(as_policy_operate* policy, zval* zpolicy_operate);

bool set_policy_apply(as_policy_apply* policy, zval* zpolicy_apply);

bool set_bin_name_list(char**bin_name_list[], zval* zbin_name_list);

bool set_as_operations_add_incr(as_operations* ops, char* bin_name, zval *value);

bool set_as_operations_prepend_strp(as_operations* ops, char* bin_name, zval* value);

bool set_as_operations_append_strp(as_operations* ops, char* bin_name, zval* value);

bool set_as_operations_list_append(as_operations* ops, char* bin_name, zval* zvalue);

bool set_as_operations_list_insert(as_operations* ops, char* bin_name, int index, zval* zvalue);

bool set_as_operations_list_pop(as_operations* ops, char* bin_name, int index);

bool set_as_operations_list_pop_range(as_operations* ops, char* bin_name, int index, int count);

bool set_as_operations_list_pop_range_from(as_operations* ops, char* bin_name, int index);

bool set_as_operations_list_remove(as_operations* ops, char* bin_name, int index);

bool set_as_operations_list_remove_range(as_operations* ops, char* bin_name, int index, int count);

bool set_as_operations_list_remove_range_from(as_operations* ops, char* bin_name, int index);

bool set_as_operations_list_clear(as_operations* ops, char* bin_name);

bool set_as_operations_list_set(as_operations* ops, char* bin_name, int index, zval* zvalue);

bool set_as_operations_list_trim(as_operations* ops, char* bin_name, int index, int count);

bool set_as_operations_list_get(as_operations* ops, char* bin_name, int index);

bool set_as_operations_list_get_range(as_operations* ops, char* bin_name, int index, int count);

bool set_as_operations_list_get_range_from(as_operations* ops, char* bin_name, int index);

bool set_as_operations_list_size(as_operations* ops, char* bin_name);

bool set_as_operations(as_operations* ops, zval* zoperations);

as_bytes* serialized_to_asbytes(zval* zvalue);

zval* unserialized_from_asbytes(as_bytes* bytes);

as_list* parse_array_to_as_list(zval* zarray);

as_map* parse_array_to_as_map(zval* zarray);

as_val* parse_zval_array(zval *zarray);

zval* parse_as_list_to_array(as_list* aslist);

zval* parse_as_map_to_array(as_hashmap* asmap);

void swoole_as_async_record_cb(as_error* err, as_record* record, void* udata, as_event_loop* event_loop);

void swoole_as_async_write_cb(as_error* err, void* udata, as_event_loop* event_loop);

void swoole_as_async_value_cb (as_error* err, as_val* val, void* udata, as_event_loop* event_loop);

#endif
#endif
