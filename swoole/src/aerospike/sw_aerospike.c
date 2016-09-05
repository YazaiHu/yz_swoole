
#include "php_swoole.h"
#include <string.h>
#include <stdlib.h>
#include "ext/standard/php_var.h"
#if PHP_MAJOR_VERSION < 7
#include "ext/standard/php_smart_str.h"
#else
#include "zend_smart_str.h"
#endif

#ifdef SW_USE_AEROSPIKE
#include "sw_aerospike.h"
int get_spilt(char *pszSrc, char *pszSplit, char *pszDest, int nDestLen)
{
    int iPos;
    char *pPtr = NULL;
    int iSrcLen = strlen(pszSrc);
    char *sz = sw_malloc(iSrcLen + 1);
    
    pPtr = strstr(pszSrc, pszSplit);
    
    if( pPtr )
    {
        iPos = strlen(pszSrc) - strlen(pPtr);
        iPos = iPos > (nDestLen - 1) ? (nDestLen - 1) : iPos;
        strncpy(pszDest, pszSrc, iPos);
        strcpy(sz, pPtr + strlen(pszSplit));
        strcpy(pszSrc, sz);
        //strcpy(pszSrc, pPtr + strlen(pszSplit));  //wrong
        pszDest[iPos] = '\0';
    }
    else
    {
        iPos = strlen(pszSrc);
        iPos = iPos > (nDestLen - 1) ? (nDestLen - 1): iPos;
        strncpy(pszDest, pszSrc, iPos);
        pszDest[iPos] = '\0';
        pszSrc[0] = '\0';
        sw_free(sz);
        return 0;
    }
    sw_free(sz);
    return 1;
}

bool set_as_key(as_key* askey, char* namespace, char* set, zval* zkey) {
    bool bRet = false;
    switch (Z_TYPE_P(zkey))
    {
        case IS_LONG:
            if (as_key_init_int64(askey, namespace, set, Z_LVAL_P(zkey)) != NULL ) {
                bRet = true;
            } else {
                swWarn("zval key is unvalid LONG.");
            }
            break;
        case IS_STRING:
            //raw和str是不同的key
            if (as_key_init_str(askey, namespace, set, Z_STRVAL_P(zkey)) != NULL) {
            //if (as_key_init_raw(askey, namespace, set, Z_STRVAL_P(zkey), Z_STRLEN_P(zkey)) != NULL) {
                bRet = true;
            } else {
                swWarn("zval key is unvalid string.");
            }
            break;
        default:
            swWarn("zval key is unsupport type: %d.", Z_TYPE_P(zkey));
    }
    return bRet;
}

as_bytes* serialized_to_asbytes(zval* zvalue) {
    smart_str serialized_data = {0};
    php_serialize_data_t var_hash;
    char *data_str;
    int data_len = 0;
    //need serialize
    //serialize
    PHP_VAR_SERIALIZE_INIT(var_hash);
    sw_php_var_serialize(&serialized_data, zvalue, &var_hash TSRMLS_CC);
    PHP_VAR_SERIALIZE_DESTROY(var_hash);
#if PHP_MAJOR_VERSION<7
    data_str = serialized_data.c;
    data_len = serialized_data.len;
#else
    data_str = serialized_data.s->val;
    data_len = serialized_data.s->len;
#endif
    
    as_bytes * bytes = as_bytes_new(data_len);
    as_bytes_set(bytes, 0, (uint8_t*)data_str, data_len);
    as_bytes_set_type(bytes, AS_BYTES_PHP);
    //TODO:
    smart_str_free(&serialized_data);
    return bytes;
}

//反序列化php的二进制序列化数据
zval* unserialized_from_asbytes(as_bytes* bytes) {
    zval* zvalue = NULL;
    SW_ALLOC_INIT_ZVAL(zvalue);
    //zval *unserialized_data;
    char *zdata_str;
    int zdata_len = 0;
    php_unserialize_data_t var_hash;
    
    PHP_VAR_UNSERIALIZE_INIT(var_hash);
    zdata_str = (char*)as_bytes_get(bytes);
    zdata_len = as_bytes_size(bytes);
    if (!sw_php_var_unserialize(&zvalue, (const unsigned char **) &zdata_str,
                                (const unsigned char *) (zdata_str + zdata_len), &var_hash TSRMLS_CC))
    {
        SW_ZVAL_STRINGL(zvalue, zdata_str, zdata_len, 1);
        swWarn("unserialized_from_asbytes: sw_php_unserialize failed, %d.", zdata_len);
    }
    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
    return zvalue;
}

//判断array是否是完全按顺序的list array格式:["aa", 123, "fff"];
bool check_array_list(zval *zarray) {
    bool bRet = false;
    HashTable *ht_data = Z_ARRVAL_P(zarray);
    int array_num = zend_hash_num_elements(ht_data);
    int i = 0;

#if PHP_MAJOR_VERSION < 7
    char *key_name;
    uint32_t key_len;
    int key_type;
    ulong_t idx = 0;
    zval **tmp = NULL;
    for (zend_hash_internal_pointer_reset(ht_data); (key_type = sw_zend_hash_get_current_key(ht_data, &key_name, &key_len, &idx)) != HASH_KEY_NON_EXISTENT; zend_hash_move_forward(ht_data))
    {
        if (zend_hash_get_current_data(ht_data, (void**)&tmp) == FAILURE) {
            continue;
        }
        
        if (HASH_KEY_IS_LONG == key_type) {
            if (idx != i) {
                break;
            }
        } else {
            break;
        }
        i++;
    }
#else 
    zend_string *key;
    ulong_t idx = 0;
    zval *value = NULL;

    ZEND_HASH_FOREACH_KEY_VAL(ht_data, idx, key, value) {
        if (key) {
            //HASH_KEY_IS_STRING
            break;
        } else {
            if (idx != i) {
                break;
            }
        }
        i++;
    } ZEND_HASH_FOREACH_END();
#endif
    
    if (array_num == i) {
        bRet = true;
    }
    return bRet;
}

as_list* parse_array_to_as_list(zval* zarray) {
    HashTable *ht_data = Z_ARRVAL_P(zarray);
    int array_num = zend_hash_num_elements(ht_data);
    as_arraylist *list = as_arraylist_new(array_num, 0);

#if PHP_MAJOR_VERSION < 7
    char *key_name;
    uint32_t key_len;
    int key_type;
    zval *zvalue = NULL;
    zval **tmp = NULL;
    ulong_t idx;
    for (zend_hash_internal_pointer_reset(ht_data); (key_type = sw_zend_hash_get_current_key(ht_data, &key_name, &key_len, &idx)) != HASH_KEY_NON_EXISTENT; zend_hash_move_forward(ht_data))
    {
        if (zend_hash_get_current_data(ht_data, (void**)&tmp) == FAILURE) {
            continue;
        }
        zvalue = *tmp;
        if (HASH_KEY_IS_LONG == key_type) {
            set_list_value((as_list*)list, zvalue);
        } else {
            swWarn("parse_array_to_as_list: is not support, %d.", key_type);
        }
    }
#else
    zend_string *key;
    ulong_t idx = 0;
    zval *zvalue = NULL;

    ZEND_HASH_FOREACH_KEY_VAL(ht_data, idx, key, zvalue) {
        if (key) {
            swWarn("parse_array_to_as_list: string is not support.");
        } else {
            set_list_value((as_list*)list, zvalue);
        }
    } ZEND_HASH_FOREACH_END();
#endif
    return (as_list*)list;
}

as_map* parse_array_to_as_map(zval* zarray) {
    HashTable *ht_data = Z_ARRVAL_P(zarray);
    int array_num = zend_hash_num_elements(ht_data);
    as_hashmap *map = as_hashmap_new(array_num);
    
#if PHP_MAJOR_VERSION < 7
    char *key_name;
    uint32_t key_len;
    int key_type;
    zval *zvalue = NULL;
    zval **tmp = NULL;
    ulong_t idx;
    for (zend_hash_internal_pointer_reset(ht_data); (key_type = sw_zend_hash_get_current_key(ht_data, &key_name, &key_len, &idx)) != HASH_KEY_NON_EXISTENT; zend_hash_move_forward(ht_data))
    {
        if (zend_hash_get_current_data(ht_data, (void**)&tmp) == FAILURE) {
            continue;
        }

        zvalue = *tmp;
        if (HASH_KEY_IS_STRING == key_type)
        {
            set_stringmap_value(map, key_name, zvalue);
        } else if (HASH_KEY_IS_LONG == key_type) {
            set_longmap_value(map, idx, zvalue);
        } else {
            swWarn("parse_array_to_as_map: is not support, %d.", key_type);
        }
    }
#else 
    zend_string *key_name;
    ulong_t idx = 0;
    zval *zvalue = NULL;
    
    ZEND_HASH_FOREACH_KEY_VAL(ht_data, idx, key_name, zvalue) {
        if (key_name) {
            set_stringmap_value((as_map*)map, key_name->val, zvalue);
        } else {
            set_longmap_value((as_map*)map, idx, zvalue);
        }
    } ZEND_HASH_FOREACH_END();
#endif
    return (as_map*)map;
}

as_val* parse_zval_array(zval *zarray) {
    if (check_array_list(zarray)) {
        return (as_val*)parse_array_to_as_list(zarray);
    } else {
        return (as_val*)parse_array_to_as_map(zarray);
    }
}

bool set_map_value(as_hashmap* map, as_val* map_key_val, zval* zvalue) {
    as_val* map_value_val = NULL;
    switch (SW_Z_TYPE_P(zvalue))
    {
        case IS_NULL:
            as_hashmap_set(map, map_key_val, &as_nil);
            break;
            /*
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
#else
        case IS_TRUE:
#endif            
            map_value_val = (as_val*)as_integer_new(Z_BVAL_P(zvalue));
            as_hashmap_set(map, map_key_val, map_value_val);
            break;
            */
        case IS_LONG:
            map_value_val = (as_val*)as_integer_new(Z_LVAL_P(zvalue));
            as_hashmap_set(map, map_key_val, map_value_val);
            break;
        case IS_DOUBLE:
            map_value_val = (as_val*)as_double_new(Z_DVAL_P(zvalue));
            as_hashmap_set(map, map_key_val, map_value_val);
            break;
        case IS_STRING:
            map_value_val = (as_val*)as_string_new_wlen(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), false);
            as_hashmap_set(map, map_key_val, map_value_val);
            break;
        case IS_ARRAY:
            map_value_val = (as_val*)parse_zval_array(zvalue);
            as_hashmap_set(map, map_key_val, map_value_val);
            break;
        case IS_OBJECT:
            map_value_val = (as_val*)serialized_to_asbytes(zvalue);
            as_hashmap_set(map, map_key_val, map_value_val);
            break;
        default:
            swWarn("map value is unsupport type: %d.", Z_TYPE_P(zvalue));
    }
    return true;
}

bool set_longmap_value(as_hashmap* map, long map_key, zval* zvalue){
    as_val *key_val = (as_val*)as_integer_new(map_key);
    set_map_value(map, key_val, zvalue);
    return true;
}


bool set_stringmap_value(as_hashmap* map, const char* map_key_name, zval* zvalue) {
    as_val *key_val = (as_val*)as_string_new_strdup(map_key_name);
    set_map_value(map, key_val, zvalue);
    return true;
}

//TODO
bool set_list_value(as_list* list, zval* zvalue) {
    as_val* list_value_val = NULL;
    switch (SW_Z_TYPE_P(zvalue))
    {
        case IS_NULL:
            as_arraylist_append((as_arraylist*)list, (as_val*)&as_nil);
            break;
            /*
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
#else
        case IS_TRUE:
#endif            
            list_value_val = (as_val*)as_integer_new(Z_BVAL_P(zvalue));
            as_arraylist_append((as_arraylist*)list, list_value_val);
            break;
        */
        case IS_LONG:
            list_value_val = (as_val*)as_integer_new(Z_LVAL_P(zvalue));
            as_arraylist_append((as_arraylist*)list, list_value_val);
            break;
        case IS_DOUBLE:
            list_value_val = (as_val*)as_double_new(Z_DVAL_P(zvalue));
            as_arraylist_append((as_arraylist*)list, list_value_val);
            break;
        case IS_STRING:
            list_value_val = (as_val*)as_string_new_wlen(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), false);
            as_arraylist_append((as_arraylist*)list, list_value_val);
            break;
        case IS_ARRAY:
            list_value_val = parse_zval_array(zvalue);
            as_arraylist_append((as_arraylist*)list, list_value_val);
            break;
        case IS_OBJECT:
            list_value_val = (as_val*)serialized_to_asbytes(zvalue);
            as_arraylist_append((as_arraylist*)list, list_value_val);
            break;
        default:
            swWarn("list value is unsupport type: %d.", Z_TYPE_P(zvalue));
    }
    return true;
}

bool set_record_bin_array_to_as_list(as_record* asrecord, const char* bin_name, zval *zarray) {
    as_list *list = parse_array_to_as_list(zarray);
    as_record_set_list(asrecord, bin_name, list);
    return true;
}

bool set_record_bin_array_to_as_map(as_record* asrecord, const char* bin_name, zval *zarray) {
    as_map *map = parse_array_to_as_map(zarray);
    as_record_set_map(asrecord, bin_name, map);
    return true;
}

bool set_record_bin_array(as_record* asrecord, const char* bin_name, zval* zarray) {
    if (check_array_list(zarray)) {
        return set_record_bin_array_to_as_list(asrecord, bin_name, zarray);
    } else {
        return set_record_bin_array_to_as_map(asrecord, bin_name, zarray);
    }
}

//序列化php object
bool set_record_bin_object(as_record* asrecord, const char* bin_name, zval *zdata) {
    as_bytes *bytes = serialized_to_asbytes(zdata);
    return as_record_set_bytes(asrecord, bin_name, bytes);
}

bool set_record_bin_long(as_record* asrecord, char* bin_name, long bin_value) {
    return as_record_set_int64(asrecord, bin_name, bin_value);
}

bool set_record_bin_double(as_record* asrecord, char* bin_name, double bin_value) {
    return as_record_set_double(asrecord, bin_name, bin_value);
}

bool set_record_bin_string(as_record* asrecord, char* bin_name, char* bin_value, long bin_value_len) {
    return as_record_set_string(asrecord, bin_name, as_string_new_wlen(bin_value, bin_value_len, false));
}

bool set_record_bin_zval(as_record* asrecord, char* bin_name, zval* zvalue) {
    bool bRet = false;
    switch (SW_Z_TYPE_P(zvalue))
    {
        case IS_NULL:
            bRet = as_record_set_nil(asrecord, bin_name);
            if (false == bRet) {
                swWarn("zval record bin value is unvalid NULL.");
            }
            break;
            /*
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
#else
        case IS_TRUE:
#endif
            swWarn("zval record bin value is BOOL.");
            break;
        */
        case IS_LONG:
            //swWarn("zval record bin value is LONG.");
            bRet = set_record_bin_long(asrecord, bin_name, Z_LVAL_P(zvalue));
            if (!bRet) {
                swWarn("zval record bin value is unvalid LONG.");
            }
            break;
        case IS_DOUBLE:
            //swWarn("zval record bin value is DOUBLE.");
            bRet = set_record_bin_double(asrecord, bin_name, Z_DVAL_P(zvalue));
            if (!bRet) {
                swWarn("zval record bin value is unvalid DOUBLE.");
            }
            break;
        case IS_STRING:
            /*
            swWarn("zval record bin value is STRING, bin name: %s, value%s.", bin_name, Z_STRVAL_P(zvalue));
            asvalue = as_string_new_wlen(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), false);
            bRet = as_record_set_string(asrecord, bin_name, asvalue);
            as_record_set_string中使用的也是这个对象，所以此处不能销毁
            在record_destroy的时候会自动销毁该as_string
            as_string_destroy(asvalue);
            */
            bRet = set_record_bin_string(asrecord, bin_name, Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue));
            if (!bRet) {
                swWarn("zval record bin value is unvalid STRING.");
            }
            break;
        case IS_RESOURCE:
            swWarn("zval record bin value is RESOURCE.");
            break;
        case IS_ARRAY:
            bRet = set_record_bin_array(asrecord, bin_name, zvalue);
            if (!bRet) {
                swWarn("zval record set record ARRAY err.");
            }
            break;
        case IS_OBJECT:
            bRet = set_record_bin_object(asrecord, bin_name, zvalue);
            if (!bRet) {
                swWarn("zval record set record OBJECT err.");
            }
            break;
        default:
            swWarn("zval record bin value is unknown.");
    }
    return bRet;
}

//将array数据变成as_record
bool set_record(as_record* asrecord, zval* zrecord) {
    bool bRet = false;
    if (Z_TYPE_P(zrecord) == IS_ARRAY) {
        HashTable *ht_data = Z_ARRVAL_P(zrecord);
        int array_num = zend_hash_num_elements(ht_data);
        as_record_init(asrecord, array_num);
#if PHP_MAJOR_VERSION < 7
        char *bin_name;
        int bin_type;
        uint32_t bin_len;
        ulong_t idx;
        zval **tmp = NULL;
        zval *zvalue = NULL;
        for (zend_hash_internal_pointer_reset(ht_data); (bin_type = sw_zend_hash_get_current_key(ht_data, &bin_name, &bin_len, &idx)) != HASH_KEY_NON_EXISTENT; zend_hash_move_forward(ht_data))
        {
            if (zend_hash_get_current_data(ht_data, (void**)&tmp) == FAILURE) {
                continue;
            }
            zvalue = *tmp;
            if (HASH_KEY_IS_STRING == bin_type)
            {
                set_record_bin_zval(asrecord, bin_name, zvalue);

            } else if (HASH_KEY_IS_LONG == bin_type) {
                //php中对于array中的key，若为"70"这种字符串都会自动转成数字，然而bin_name必须为字符串，所以做个强转
                char buf[32];
                swoole_itoa(buf, idx);
                set_record_bin_zval(asrecord, buf, zvalue);
            } else {
                swWarn("zval record bin is not string or long.");
            }
        }
#else
        zend_string *key_name;
        ulong_t idx = 0;
        zval *zvalue = NULL;
        
        ZEND_HASH_FOREACH_KEY_VAL(ht_data, idx, key_name, zvalue) {
            if (key_name) {
                set_record_bin_zval(asrecord, key_name->val, zvalue);
            } else {
                //php中对于array中的key，若为"70"这种字符串都会自动转成数字，然而bin_name必须为字符串，所以做个强转
                char buf[32];
                int s_len = swoole_itoa(buf, idx);
                set_record_bin_zval(asrecord, buf, zvalue);
            }
        } ZEND_HASH_FOREACH_END();
#endif
        bRet = true;
    } else {
        swWarn("zval record is not array.");
    }
    return bRet;
}

zval* parse_as_list_to_array(as_list* aslist);
zval* parse_as_map_to_array(as_hashmap* asmap);

void add_array_by_string_key(zval* zvalue, as_string* key, as_val* value) {
    switch (as_val_type(value)) {
        case AS_NIL:
            add_assoc_null(zvalue, as_string_get(key));
            break;
        case AS_BOOLEAN:
            add_assoc_bool(zvalue, as_string_get(key), as_boolean_get((as_boolean*)value));
            break;
        case AS_INTEGER:
            add_assoc_long(zvalue, as_string_get(key), as_integer_get((as_integer*)value));
            break;
        case AS_STRING:
            sw_add_assoc_stringl(zvalue, as_string_get(key), as_string_get((as_string*)value), as_string_len((as_string*)value), 1);
            break;
        case AS_LIST:
            add_assoc_zval(zvalue, as_string_get(key), parse_as_list_to_array((as_list*)value));
            break;
        case AS_MAP:
            add_assoc_zval(zvalue, as_string_get(key), parse_as_map_to_array((as_hashmap*)value));
            break;
        case AS_BYTES:
            if (as_bytes_get_type((as_bytes*)value) == AS_BYTES_PHP) {
                add_assoc_zval(zvalue, as_string_get(key), unserialized_from_asbytes((as_bytes*)value));
            } else {
                sw_add_assoc_stringl(zvalue, as_string_get(key), (char*)as_bytes_get((as_bytes*)value), as_bytes_size((as_bytes*)value), 1);
            }
            break;
        case AS_DOUBLE:
            add_assoc_double(zvalue, as_string_get(key), as_double_get((as_double*)value));
            break;
        case AS_UNDEF:
            swWarn("add_array_by_string_key value type is unvalid.");
            break;
        default:
            swWarn("add_array_by_string_key value type is unsupport: %d.\n", as_val_type(value));
            break;
    }
}

void add_array_by_long_key(zval* zvalue, as_integer* key, as_val* value) {
    switch (as_val_type(value)) {
        case AS_NIL:
            add_index_null(zvalue, as_integer_get(key));
            break;
        case AS_BOOLEAN:
            add_index_bool(zvalue, as_integer_get(key), as_boolean_get((as_boolean*)value));
            break;
        case AS_INTEGER:
            add_index_long(zvalue, as_integer_get(key), as_integer_get((as_integer*)value));
            break;
        case AS_STRING:
#if PHP_MAJOR_VERSION < 7
            add_index_stringl(zvalue, as_integer_get(key), as_string_get((as_string*)value), as_string_len((as_string*)value), 1);
#else
            add_index_stringl(zvalue, as_integer_get(key), as_string_get((as_string*)value), as_string_len((as_string*)value));
#endif
            break;
        case AS_LIST:
            add_index_zval(zvalue, as_integer_get(key), parse_as_list_to_array((as_list*)value));
            break;
        case AS_MAP:
            add_index_zval(zvalue, as_integer_get(key), parse_as_map_to_array((as_hashmap*)value));
            break;
        case AS_BYTES:
            if (as_bytes_get_type((as_bytes*)value) == AS_BYTES_PHP) {
                add_index_zval(zvalue, as_integer_get(key), unserialized_from_asbytes((as_bytes*)value));
            } else {
#if PHP_MAJOR_VERSION < 7
                add_index_stringl(zvalue, as_integer_get(key), (char*)as_bytes_get((as_bytes*)value), as_bytes_size((as_bytes*)value), 1);
#else
                add_index_stringl(zvalue, as_integer_get(key), (char*)as_bytes_get((as_bytes*)value), as_bytes_size((as_bytes*)value));
#endif
            }
            break;
        case AS_DOUBLE:
            add_index_double(zvalue, as_integer_get(key), as_double_get((as_double*)value));
            break;
        case AS_UNDEF:
            swWarn("add_array_by_long_key value type is unvalid.");
            break;
        default:
            swWarn("add_array_by_long_key value type is unsupport: %d.\n", as_val_type(value));
            break;
    }
}

void add_array_by_list_value(zval* zvalue, as_val *value) {
    switch (as_val_type(value)) {
        case AS_NIL:
            add_next_index_null(zvalue);
            break;
        case AS_BOOLEAN:
            add_next_index_bool(zvalue, as_boolean_get((as_boolean*)value));
            break;
        case AS_INTEGER:
            add_next_index_long(zvalue, as_integer_get((as_integer*)value));
            break;
        case AS_STRING:
            sw_add_next_index_stringl(zvalue, as_string_get((as_string*)value), as_string_len((as_string*)value), 1);
            break;
        case AS_LIST:
            add_next_index_zval(zvalue, parse_as_list_to_array((as_list*)value));
            break;
        case AS_MAP:
            add_next_index_zval(zvalue, parse_as_map_to_array((as_hashmap*)value));
            break;
        case AS_BYTES:
            if (as_bytes_get_type((as_bytes*)value) == AS_BYTES_PHP) {
                add_next_index_zval(zvalue, unserialized_from_asbytes((as_bytes*)value));
            } else {
                sw_add_next_index_stringl(zvalue, (char*)as_bytes_get((as_bytes*)value), as_bytes_size((as_bytes*)value), 1);
            }
            break;
        case AS_DOUBLE:
            add_next_index_double(zvalue, as_double_get((as_double*)value));
            break;
        case AS_UNDEF:
            swWarn("add_array_by_list_value value type is unvalid.");
            break;
        default:
            swWarn("add_array_by_list_value value type is unsupport: %d.\n", as_val_type(value));
            break;
    }
}

zval* parse_as_list_to_array(as_list* aslist) {
    zval* zvalue = NULL;
    SW_ALLOC_INIT_ZVAL(zvalue);
    array_init(zvalue);
    
    as_arraylist_iterator it;
    as_arraylist_iterator_init(&it, (as_arraylist*)aslist);
    while ( as_arraylist_iterator_has_next(&it) ) {
        as_val * value = (as_val*)as_arraylist_iterator_next(&it);
        add_array_by_list_value(zvalue, value);
    }
    as_arraylist_iterator_destroy(&it);
    return zvalue;
}

zval* parse_as_map_to_array(as_hashmap* asmap) {
    zval* zvalue = NULL;
    SW_ALLOC_INIT_ZVAL(zvalue);
    array_init(zvalue);
    as_hashmap_iterator it;
    as_hashmap_iterator_init(&it, asmap);
    while ( as_hashmap_iterator_has_next(&it) ) {
        as_pair * pair_val = (as_pair*)as_hashmap_iterator_next(&it);
        as_val *key = as_pair_1(pair_val);
        as_val *value = as_pair_2(pair_val);
        if (as_val_type(key) == AS_INTEGER) {
            add_array_by_long_key(zvalue, (as_integer*)key, value);
        } else if (as_val_type(key) == AS_STRING) {
            add_array_by_string_key(zvalue, (as_string*)key, value);
        } else {
            printf("parse_array_from_as_map, unvalid key type.\n");
        }
    }
    as_hashmap_iterator_destroy(&it);
    return zvalue;
}

//TODO
bool parse_as_record(as_record* asrecord, zval* zrecord) {
    as_record_iterator it;
    as_record_iterator_init(&it, asrecord);
    while ( as_record_iterator_has_next(&it) ) {
        as_bin * bin = as_record_iterator_next(&it);
        as_bin_value* value = as_bin_get_value(bin);
        char* bin_name = as_bin_get_name(bin);
        zval* zvalue = NULL;
        switch (as_bin_get_type(bin)) {
            case AS_NIL:
                add_assoc_null(zrecord, bin_name);
                break;
            case AS_BOOLEAN:
                add_assoc_bool(zrecord, bin_name, as_boolean_get((as_boolean*)value));
                break;
            case AS_INTEGER:
                add_assoc_long(zrecord, bin_name, as_integer_get((as_integer*)value));
                break;
            case AS_DOUBLE:
                add_assoc_double(zrecord, bin_name, as_double_get((as_double*)value));
                break;
            case AS_STRING:
                sw_add_assoc_stringl(zrecord, bin_name, as_string_get((as_string*)value), as_string_len((as_string*)value), 1);
                break;
            case AS_GEOJSON:
                //???
                sw_add_assoc_stringl(zrecord, bin_name, (char*)as_bytes_get((as_bytes*)value), as_bytes_size((as_bytes*)value), 1);
                swWarn("parse_record, bin value type:AS_GEOJSON.");
                break;
            case AS_BYTES:
                if (as_bytes_get_type((as_bytes*)value) == AS_BYTES_PHP) {
                    zvalue = unserialized_from_asbytes((as_bytes*)value);
                    //add_assoc_zval 中的zvalue必须是sw_alloc_ini_zval或者 make_std_zval，所以zvalue的释放应该
                    //是会在释放zrecord时一起释放
                    add_assoc_zval(zrecord, bin_name, zvalue);
                    //swWarn("parse_record, bin value AS_BYTES_PHP.");
                } else {
                    swWarn("parse_record, bin value AS_BYTES, not AS_BYTES_PHP.");
                }
                break;
            case AS_LIST:
                zvalue = parse_as_list_to_array((as_list*)value);
                add_assoc_zval(zrecord, bin_name, zvalue);
                break;
            case AS_MAP:
                zvalue = parse_as_map_to_array((as_hashmap*)value);
                add_assoc_zval(zrecord, bin_name, zvalue);
                break;
            case AS_REC:
                swWarn("parse_record, bin value AS_REC.");
                break;
            case AS_UNDEF:
                swWarn("parse_record, bin value AS_UNDEF.");
                break;
            default:
                swWarn("parse_record, bin value unknown.");
                break;
        }
    }
    as_record_iterator_destroy(&it);
    return true;
}

bool set_policy_write(as_policy_write* policy, zval* zpolicy_write, int* ttl) {
    *ttl = 0;
    as_policy_write_init(policy);
    if (NULL == zpolicy_write) {
        return true;
    }
    
    if (Z_TYPE_P(zpolicy_write) != IS_ARRAY) {
        swWarn("set_policy_write err: policy_write is not ARRAY.");
        return false;
    }
    zval *zvalue;
    HashTable *vht = Z_ARRVAL_P(zpolicy_write);
    if (sw_zend_hash_find(vht, ZEND_STRS("timeout"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->timeout = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("retry"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->retry = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("compression_threshold"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->compression_threshold = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("key"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->key = Z_LVAL_P(zvalue);
    } else {
        policy->key = AS_POLICY_KEY_SEND;
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("gen"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->gen = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("exists"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->exists = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("commit_level"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->commit_level = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("ttl"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        *ttl = Z_LVAL_P(zvalue);
    }
    
    return true;
}

bool set_policy_read(as_policy_read* policy, zval* zpolicy_read) {
    as_policy_read_init(policy);
    if (NULL == zpolicy_read) {
        return true;
    }
    if (Z_TYPE_P(zpolicy_read) != IS_ARRAY) {
        swWarn("set_policy_read err: policy_read is not ARRAY.");
        return false;
    }
    zval *zvalue;
    HashTable *vht = Z_ARRVAL_P(zpolicy_read);
    if (sw_zend_hash_find(vht, ZEND_STRS("timeout"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->timeout = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("retry"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->retry = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("key"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->key = Z_LVAL_P(zvalue);
    } else {
        policy->key = AS_POLICY_KEY_SEND;
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("replica"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->replica = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("consistency_level"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->consistency_level = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("deserialize"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        if (1 == Z_LVAL_P(zvalue)) {
            policy->deserialize = true;
        } else {
            policy->deserialize = false;
        }
    }
    return true;
}

bool set_policy_remove(as_policy_remove* policy, zval* zpolicy_remove) {
    as_policy_remove_init(policy);
    if (NULL == zpolicy_remove) {
        return true;
    }
    if (Z_TYPE_P(zpolicy_remove) != IS_ARRAY) {
        swWarn("set_policy_remove err: policy_remove is not ARRAY.");
        return false;
    }
    zval *zvalue;
    HashTable *vht = Z_ARRVAL_P(zpolicy_remove);
    if (sw_zend_hash_find(vht, ZEND_STRS("timeout"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->timeout = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("retry"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->retry = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("key"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->key = Z_LVAL_P(zvalue);
    } else {
        policy->key = AS_POLICY_KEY_SEND;
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("gen"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->gen = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("generation"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->generation = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("commit_level"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->commit_level = Z_LVAL_P(zvalue);
    }
    return true;
}


bool set_policy_operate(as_policy_operate* policy, zval* zpolicy_operate) {
    as_policy_operate_init(policy);
    if (NULL == zpolicy_operate) {
        return true;
    }
    if (Z_TYPE_P(zpolicy_operate) != IS_ARRAY) {
        swWarn("set_policy_operate err: policy_operate is not ARRAY.");
        return false;
    }
    zval *zvalue;
    HashTable *vht = Z_ARRVAL_P(zpolicy_operate);
    if (sw_zend_hash_find(vht, ZEND_STRS("timeout"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->timeout = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("retry"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->retry = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("key"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->key = Z_LVAL_P(zvalue);
    } else {
        policy->key = AS_POLICY_KEY_SEND;
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("gen"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->gen = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("replica"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->replica = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("consistency_level"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->consistency_level = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("commit_level"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->commit_level = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("deserialize"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        if (1 == Z_LVAL_P(zvalue)) {
            policy->deserialize = true;
        } else {
            policy->deserialize = false;
        }
    }
    return true;
}

bool set_policy_apply(as_policy_apply* policy, zval* zpolicy_apply) {
    as_policy_apply_init(policy);
    if (NULL == zpolicy_apply) {
        return true;
    }
    
    if (Z_TYPE_P(zpolicy_apply) != IS_ARRAY) {
        swWarn("set_policy_apply err: policy_apply is not ARRAY.");
        return false;
    }
    zval *zvalue;
    HashTable *vht = Z_ARRVAL_P(zpolicy_apply);
    if (sw_zend_hash_find(vht, ZEND_STRS("timeout"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->timeout = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("key"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->key = Z_LVAL_P(zvalue);
    } else {
        policy->key = AS_POLICY_KEY_SEND;
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("commit_level"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->commit_level = Z_LVAL_P(zvalue);
    }
    if (sw_zend_hash_find(vht, ZEND_STRS("ttl"), (void **) &zvalue) == SUCCESS)
    {
        convert_to_long(zvalue);
        policy->ttl = Z_LVAL_P(zvalue);
    }
    return true;
}

bool set_bin_name_list(char**bin_name_list[], zval* zbin_name_list) {
    HashTable *ht_data = Z_ARRVAL_P(zbin_name_list);
    int array_num = zend_hash_num_elements(ht_data);
    char **bin_name_list_tmp = sw_malloc(sizeof(char*) * (array_num + 1));
    if (bin_name_list_tmp == NULL)
    {
        swWarn("set_bin_name_list: malloc failed.");
        return false;
    }
    int num = 0;
    zval *element = NULL;
    SW_HASHTABLE_FOREACH_START(ht_data, element);
    {
        if (Z_TYPE_P(element) != IS_STRING)
        {
            swWarn("set_bin_name_list: element is not string.");
            continue;
        }
        bin_name_list_tmp[num] = Z_STRVAL_P(element);
        num++;
    }
    //aerospike select的bin接口需要最后一个array数组为null来判断
    bin_name_list_tmp[num] = NULL;
    SW_HASHTABLE_FOREACH_END();
    *bin_name_list = bin_name_list_tmp;
    return true;
}

bool set_as_operations_add_incr(as_operations* ops, char* bin_name, zval *value) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (Z_TYPE_P(value) == IS_LONG) {
        if (as_operations_add_incr(ops, bin_name, Z_LVAL_P(value))) {
            bRet = true;
        } else {
            swWarn("set_as_operations_add_incr: add incr long failed.");
        }
    } else if (Z_TYPE_P(value) == IS_DOUBLE) {
        if (as_operations_add_incr_double(ops, bin_name, Z_DVAL_P(value))) {
            bRet = true;
        } else {
            swWarn("set_as_operations_add_incr: add incr double failed.");
        }
    } else {
        swWarn("set_as_operations_add_incr: value is not long or double.");
    }
    return bRet;
}

bool set_as_operations_prepend_strp(as_operations* ops, char* bin_name, zval* value) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (Z_TYPE_P(value) == IS_STRING) {
        if (as_operations_add_prepend_str(ops, bin_name, Z_STRVAL_P(value))) {
            bRet = true;
        } else {
            swWarn("set_as_operations_prepend_strp failed.");
        }
    } else {
        swWarn("set_as_operations_prepend_strp: value is not string.");
    }
    return bRet;
}

bool set_as_operations_append_strp(as_operations* ops, char* bin_name, zval* value) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (Z_TYPE_P(value) == IS_STRING) {
        if (as_operations_add_append_str(ops, bin_name, Z_STRVAL_P(value))) {
            bRet = true;
        } else {
            swWarn("set_as_operations_append_strp failed.");
        }
    } else {
        swWarn("set_as_operations_append_strp: value is not string.");
    }
    return bRet;
}

bool set_as_operations_list_append(as_operations* ops, char* bin_name, zval* zvalue) {
    bool bRet = false;
    as_operations_init(ops, 1);
    as_val* value_val = NULL;
    switch (SW_Z_TYPE_P(zvalue)) {
        case IS_NULL:
            bRet = as_operations_add_list_append(ops, bin_name, (as_val*)&as_nil);
            break;
            /*
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
#else
        case IS_TRUE:
#endif
            value_val = (as_val*)as_integer_new(Z_BVAL_P(zvalue));
            bRet = as_operations_add_list_append(ops, bin_name, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_append bool type failed.");
            }
            break;
            */
        case IS_LONG:
            value_val = (as_val*)as_integer_new(Z_LVAL_P(zvalue));
            bRet = as_operations_add_list_append(ops, bin_name, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_append long type failed.");
            }
            break;
        case IS_DOUBLE:
            value_val = (as_val*)as_double_new(Z_DVAL_P(zvalue));
            bRet = as_operations_add_list_append(ops, bin_name, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_append double type failed.");
            }
            break;
        case IS_STRING:
            value_val = (as_val*)as_string_new_wlen(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), false);
            bRet = as_operations_add_list_append(ops, bin_name, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_append string type failed.");
            }
            break;
        case IS_ARRAY:
            value_val = (as_val*)parse_zval_array(zvalue);
            bRet = as_operations_add_list_append(ops, bin_name, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_append array type failed.");
            }
            break;
        case IS_OBJECT:
            value_val = (as_val*)serialized_to_asbytes(zvalue);
            bRet = as_operations_add_list_append(ops, bin_name, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_append object type failed.");
            }
            break;
        default:
            swWarn("set_as_operations_list_append, value is unsupport type: %d.", SW_Z_TYPE_P(zvalue));
    }
    return bRet;
}

bool set_as_operations_list_insert(as_operations* ops, char* bin_name, int index, zval* zvalue) {
    bool bRet = false;
    as_operations_init(ops, 1);
    as_val* value_val = NULL;
    switch (SW_Z_TYPE_P(zvalue)) {
        case IS_NULL:
            bRet = as_operations_add_list_insert(ops, bin_name, index, (as_val*)&as_nil);
            break;
            /*
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
#else
        case IS_TRUE:
#endif
            value_val = (as_val*)as_integer_new(Z_BVAL_P(zvalue));
            bRet = as_operations_add_list_insert(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_add_list_insert bool type failed.");
            }
            break;
            */
        case IS_LONG:
            value_val = (as_val*)as_integer_new(Z_LVAL_P(zvalue));
            bRet = as_operations_add_list_insert(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_add_list_insert long type failed.");
            }
            break;
        case IS_DOUBLE:
            value_val = (as_val*)as_double_new(Z_DVAL_P(zvalue));
            bRet = as_operations_add_list_insert(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_add_list_insert double type failed.");
            }
            break;
        case IS_STRING:
            value_val = (as_val*)as_string_new_wlen(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), false);
            bRet = as_operations_add_list_insert(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_add_list_insert string type failed.");
            }
            break;
        case IS_ARRAY:
            value_val = (as_val*)parse_zval_array(zvalue);
            bRet = as_operations_add_list_insert(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_add_list_insert array type failed.");
            }
            break;
        case IS_OBJECT:
            value_val = (as_val*)serialized_to_asbytes(zvalue);
            bRet = as_operations_add_list_insert(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_add_list_insert object type failed.");
            }
            break;
        default:
            swWarn("set_as_operations_add_list_insert, value is unsupport type: %d.", SW_Z_TYPE_P(zvalue));
    }
    return bRet;
}

bool set_as_operations_list_pop(as_operations* ops, char* bin_name, int index) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_pop(ops, bin_name, index)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_pop failed.");
    }
    return bRet;
}

bool set_as_operations_list_pop_range(as_operations* ops, char* bin_name, int index, int count) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_pop_range(ops, bin_name, index, count)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_pop failed.");
    }
    return bRet;
}

bool set_as_operations_list_pop_range_from(as_operations* ops, char* bin_name, int index) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_pop_range_from(ops, bin_name, index)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_pop_range_from failed.");
    }
    return bRet;
}

bool set_as_operations_list_remove(as_operations* ops, char* bin_name, int index) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_remove(ops, bin_name, index)) {
        bRet = true;
    } else {
        swWarn("as_operations_add_list_remove failed.");
    }
    return bRet;
}

bool set_as_operations_list_remove_range(as_operations* ops, char* bin_name, int index, int count) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_remove_range(ops, bin_name, index, count)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_remove_range failed.");
    }
    return bRet;
}


bool set_as_operations_list_remove_range_from(as_operations* ops, char* bin_name, int index) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_remove_range_from(ops, bin_name, index)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_remove_range_from failed.");
    }
    return bRet;
}

bool set_as_operations_list_clear(as_operations* ops, char* bin_name) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_clear(ops, bin_name)) {
        bRet = true;
    } else {
        swWarn("as_operations_add_list_clear failed.");
    }
    return bRet;
}

bool set_as_operations_list_set(as_operations* ops, char* bin_name, int index, zval* zvalue) {
    bool bRet = false;
    as_operations_init(ops, 1);
    as_val* value_val = NULL;
    switch (SW_Z_TYPE_P(zvalue)) {
        case IS_NULL:
            bRet = as_operations_add_list_set(ops, bin_name, index, (as_val*)&as_nil);
            break;
            /*
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
#else
        case IS_TRUE:
#endif
            value_val = (as_val*)as_integer_new(Z_BVAL_P(zvalue));
            bRet = as_operations_add_list_set(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_set bool type failed.");
            }
            break;
            */
        case IS_LONG:
            value_val = (as_val*)as_integer_new(Z_LVAL_P(zvalue));
            bRet = as_operations_add_list_set(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_set long type failed.");
            }
            break;
        case IS_DOUBLE:
            value_val = (as_val*)as_double_new(Z_DVAL_P(zvalue));
            bRet = as_operations_add_list_set(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_set double type failed.");
            }
            break;
        case IS_STRING:
            value_val = (as_val*)as_string_new_wlen(Z_STRVAL_P(zvalue), Z_STRLEN_P(zvalue), false);
            bRet = as_operations_add_list_set(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_set string type failed.");
            }
            break;
        case IS_ARRAY:
            value_val = (as_val*)parse_zval_array(zvalue);
            bRet = as_operations_add_list_set(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_set array type failed.");
            }
            break;
        case IS_OBJECT:
            value_val = (as_val*)serialized_to_asbytes(zvalue);
            bRet = as_operations_add_list_set(ops, bin_name, index, value_val);
            if (!bRet) {
                swWarn("set_as_operations_list_set object type failed.");
            }
            break;
        default:
            swWarn("set_as_operations_list_set, value is unsupport type: %d.", SW_Z_TYPE_P(zvalue));
    }
    return bRet;
}

bool set_as_operations_list_trim(as_operations* ops, char* bin_name, int index, int count) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_trim(ops, bin_name, index, count)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_trim failed.");
    }
    return bRet;
}

bool set_as_operations_list_get(as_operations* ops, char* bin_name, int index) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_get(ops, bin_name, index)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_get failed.");
    }
    return bRet;
}

bool set_as_operations_list_get_range(as_operations* ops, char* bin_name, int index, int count) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_get_range(ops, bin_name, index, count)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_get_range failed.");
    }
    return bRet;
}

bool set_as_operations_list_get_range_from(as_operations* ops, char* bin_name, int index) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_get_range_from(ops, bin_name, index)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_get_range_from failed.");
    }
    return bRet;
}

bool set_as_operations_list_size(as_operations* ops, char* bin_name) {
    bool bRet = false;
    as_operations_init(ops, 1);
    if (as_operations_add_list_size(ops, bin_name)) {
        bRet = true;
    } else {
        swWarn("set_as_operations_list_size failed.");
    }
    return bRet;
}

bool set_as_operations(as_operations* ops, zval* zoperations) {
    HashTable *ht_data = Z_ARRVAL_P(zoperations);
    int array_num = zend_hash_num_elements(ht_data);
    return false;
}

void swoole_as_async_record_cb(as_error* err, as_record* record, void* udata, as_event_loop* event_loop) {
    swCallBackData* data = (swCallBackData*)udata;
    zval **args[2];
    zval *retval = NULL;
    zval *zerr = NULL;
    zval *zrecord = NULL;
    
    SW_ALLOC_INIT_ZVAL(zerr);
    SW_ALLOC_INIT_ZVAL(zrecord);
    
    if (NULL == err) {
        SW_ZVAL_STRINGL(zerr, as_error_string(AEROSPIKE_OK), strlen(as_error_string(AEROSPIKE_OK)), 1);
        //为何这样写也没有问题
        //SW_ZVAL_STRINGL(zerr, as_error_string(err->code), strlen(as_error_string(err->code)), 0);
    } else {
        SW_ZVAL_STRINGL(zerr, as_error_string(err->code), strlen(as_error_string(err->code)), 1);
    }
    if (record != NULL) {
        array_init(zrecord);
        parse_as_record(record, zrecord);
    } else {
        //什么都不设置有问题么？
    }
    args[0] = &zerr;
    args[1] = &zrecord;
    //此处如果给php层是用zerr,必须要使用sw_zval_add_ref;
    sw_zval_add_ref(&zerr);
    sw_zval_add_ref(&zrecord);

    if (sw_call_user_function_ex(EG(function_table), NULL, data->cb, &retval, 2, args, 0, NULL TSRMLS_CC) == FAILURE)
    {
        swWarn("swoole_as_async_record_cb error: %s.", as_error_string(err->code));
    }
    
    if (zerr != NULL) {
        sw_zval_ptr_dtor(&zerr);
    }
    if (zrecord != NULL) {
        sw_zval_ptr_dtor(&zrecord);
    }

    if (retval != NULL)
    {
        sw_zval_ptr_dtor(&retval);
    }
    if (data != NULL) {
        
        if (data->cb) {
            //这样有问题么？？
            sw_zval_ptr_dtor(&data->cb);
        }
        sw_free(data);
    }
}

void swoole_as_async_write_cb(as_error* err, void* udata, as_event_loop* event_loop){
    swCallBackData* data = (swCallBackData*)udata;
    zval **args[1];
    zval *retval = NULL;
    zval *zerr = NULL;
    SW_MAKE_STD_ZVAL(zerr);
    if (NULL == err) {
        SW_ZVAL_STRINGL(zerr, as_error_string(AEROSPIKE_OK), strlen(as_error_string(AEROSPIKE_OK)), 1);
        //为何这样写也没有问题
        //SW_ZVAL_STRINGL(zerr, as_error_string(err->code), strlen(as_error_string(err->code)), 0);
    } else {
        SW_ZVAL_STRINGL(zerr, as_error_string(err->code), strlen(as_error_string(err->code)), 1);
    }
    
    args[0] = &zerr;
    //此处如果给php层是用zerr,必须要使用sw_zval_add_ref;
    sw_zval_add_ref(&zerr);
    if (sw_call_user_function_ex(EG(function_table), NULL, data->cb, &retval, 1, args, 0, NULL TSRMLS_CC) == FAILURE)
    {
        swWarn("swoole_as_async_write_cb error: %s.", as_error_string(err->code));
    }
    
    if (zerr != NULL) {
        sw_zval_ptr_dtor(&zerr);
    }
    if (retval != NULL)
    {
        sw_zval_ptr_dtor(&retval);
    }
    if (data != NULL) {
        if (data->cb) {
            //这样有问题么？？
            sw_zval_ptr_dtor(&data->cb);
        }
        sw_free(data);
    }
}

void swoole_as_async_value_cb (as_error* err, as_val* val, void* udata, as_event_loop* event_loop) {
    swCallBackData* data = (swCallBackData*)udata;
    zval **args[1];
    zval *retval = NULL;
    zval *zerr = NULL;
    SW_MAKE_STD_ZVAL(zerr);
    if (NULL == err) {
        SW_ZVAL_STRINGL(zerr, as_error_string(AEROSPIKE_OK), strlen(as_error_string(AEROSPIKE_OK)), 1);
    } else {
        SW_ZVAL_STRINGL(zerr, as_error_string(err->code), strlen(as_error_string(err->code)), 1);
    }
    
    args[0] = &zerr;
    //此处如果给php层是用zerr,必须要使用sw_zval_add_ref;
    sw_zval_add_ref(&zerr);
    printf("swoole_as_async_value_cb, as_val type is: %d\n", val->type);
    if (sw_call_user_function_ex(EG(function_table), NULL, data->cb, &retval, 1, args, 0, NULL TSRMLS_CC) == FAILURE)
    {
        swWarn("swoole_as_async_value_cb error: %s.", as_error_string(err->code));
    }
    
    if (zerr != NULL) {
        sw_zval_ptr_dtor(&zerr);
    }
    if (retval != NULL)
    {
        sw_zval_ptr_dtor(&retval);
    }
    if (data != NULL) {
        if (data->cb) {
            //这样有问题么？？
            sw_zval_ptr_dtor(&data->cb);
        }
        sw_free(data);
    }
}

#endif
