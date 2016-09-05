
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
#include "sw_aerospike_event_loop.h"

//回调集合结构体
as_event_swoole_callback g_as_event_callback = {0};
int g_InitNum = 0;

/*************************************************************************
*php method
*************************************************************************/
ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_init, 0, 0, 3)
ZEND_ARG_INFO(0, user)
ZEND_ARG_INFO(0, password)
ZEND_ARG_INFO(0, addrlist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_get_async, 0, 0, 5)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_put_async, 0, 0, 6)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, value)
ZEND_ARG_INFO(0, write_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_put_simple_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, bin_value)
ZEND_ARG_INFO(0, write_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_key_select_async, 0, 0, 6)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name_list)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_key_exists_async, 0, 0, 5)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_key_remove_async, 0, 0, 5)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, write_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_key_operate_async, 0, 0, 6)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, operations)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_incr_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, value)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_operate_strp_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, value)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_append_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, value)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_insert_async, 0, 0, 8)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, value)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_pop_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_pop_range_async, 0, 0, 8)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, count)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_pop_range_from_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_remove_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_remove_range_async, 0, 0, 8)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, count)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_remove_range_from_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_clear_async, 0, 0, 6)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_set_async, 0, 0, 8)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, value)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_trim_async, 0, 0, 8)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, count)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_get_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_get_range_async, 0, 0, 8)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, count)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_get_range_from_async, 0, 0, 7)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, index)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_list_size_async, 0, 0, 6)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, bin_name)
ZEND_ARG_INFO(0, record_callback)
ZEND_ARG_INFO(0, policy)
ZEND_END_ARG_INFO()

/*
ZEND_BEGIN_ARG_INFO_EX(arginfo_swoole_aerospike_key_apply_async, 0, 0, 5)
ZEND_ARG_INFO(0, namespace)
ZEND_ARG_INFO(0, set)
ZEND_ARG_INFO(0, key)
ZEND_ARG_INFO(0, apply_policy)
ZEND_ARG_INFO(0, value_callback)
ZEND_END_ARG_INFO()
*/

static PHP_METHOD(swoole_aerospike, __construct);
static PHP_METHOD(swoole_aerospike, __destruct);
static PHP_METHOD(swoole_aerospike, init);
static PHP_METHOD(swoole_aerospike, uninit);
static PHP_METHOD(swoole_aerospike, connect);
static PHP_METHOD(swoole_aerospike, is_connected);
static PHP_METHOD(swoole_aerospike, close);
static PHP_METHOD(swoole_aerospike, get_async);
static PHP_METHOD(swoole_aerospike, put_async);
static PHP_METHOD(swoole_aerospike, put_simple_async);
static PHP_METHOD(swoole_aerospike, put_map_async);
static PHP_METHOD(swoole_aerospike, put_list_async);
static PHP_METHOD(swoole_aerospike, key_select_async);
static PHP_METHOD(swoole_aerospike, key_exists_async);
static PHP_METHOD(swoole_aerospike, key_remove_async);
static PHP_METHOD(swoole_aerospike, key_operate_async);
static PHP_METHOD(swoole_aerospike, incr_async);
static PHP_METHOD(swoole_aerospike, prepend_strp_async);
static PHP_METHOD(swoole_aerospike, append_strp_async);
static PHP_METHOD(swoole_aerospike, list_append_async);
static PHP_METHOD(swoole_aerospike, list_insert_async);
static PHP_METHOD(swoole_aerospike, list_pop_async);
static PHP_METHOD(swoole_aerospike, list_pop_range_async);
static PHP_METHOD(swoole_aerospike, list_pop_range_from_async);
static PHP_METHOD(swoole_aerospike, list_remove_async);
static PHP_METHOD(swoole_aerospike, list_remove_range_async);
static PHP_METHOD(swoole_aerospike, list_remove_range_from_async);
static PHP_METHOD(swoole_aerospike, list_clear_async);
static PHP_METHOD(swoole_aerospike, list_set_async);
static PHP_METHOD(swoole_aerospike, list_trim_async);
static PHP_METHOD(swoole_aerospike, list_get_async);
static PHP_METHOD(swoole_aerospike, list_get_range_async);
static PHP_METHOD(swoole_aerospike, list_get_range_from_async);
static PHP_METHOD(swoole_aerospike, list_size_async);

//static PHP_METHOD(swoole_aerospike, key_apply_async);

static zend_class_entry swoole_aerospike_ce;
zend_class_entry *swoole_aerospike_class_entry_ptr;

static const zend_function_entry swoole_aerospike_methods[] =
{
    PHP_ME(swoole_aerospike, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(swoole_aerospike, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    PHP_ME(swoole_aerospike, init, arginfo_swoole_aerospike_init, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, uninit, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, connect, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, is_connected, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, close, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, get_async, arginfo_swoole_aerospike_get_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, put_async, arginfo_swoole_aerospike_put_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, put_simple_async, arginfo_swoole_aerospike_put_simple_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, put_map_async, arginfo_swoole_aerospike_put_simple_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, put_list_async, arginfo_swoole_aerospike_put_simple_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, key_select_async, arginfo_swoole_aerospike_key_select_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, key_exists_async, arginfo_swoole_aerospike_key_exists_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, key_remove_async, arginfo_swoole_aerospike_key_remove_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, key_operate_async, arginfo_swoole_aerospike_key_operate_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, incr_async, arginfo_swoole_aerospike_incr_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, prepend_strp_async, arginfo_swoole_aerospike_operate_strp_async, ZEND_ACC_PUBLIC)
    
    PHP_ME(swoole_aerospike, append_strp_async, arginfo_swoole_aerospike_operate_strp_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_append_async, arginfo_swoole_aerospike_list_append_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_insert_async, arginfo_swoole_aerospike_list_insert_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_pop_async, arginfo_swoole_aerospike_list_pop_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_pop_range_async, arginfo_swoole_aerospike_list_pop_range_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_pop_range_from_async, arginfo_swoole_aerospike_list_pop_range_from_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_remove_async, arginfo_swoole_aerospike_list_remove_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_remove_range_async, arginfo_swoole_aerospike_list_remove_range_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_remove_range_from_async, arginfo_swoole_aerospike_list_remove_range_from_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_clear_async, arginfo_swoole_aerospike_list_clear_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_set_async, arginfo_swoole_aerospike_list_set_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_trim_async, arginfo_swoole_aerospike_list_trim_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_get_async, arginfo_swoole_aerospike_list_get_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_get_range_async, arginfo_swoole_aerospike_list_get_range_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_get_range_from_async, arginfo_swoole_aerospike_list_get_range_from_async, ZEND_ACC_PUBLIC)
    PHP_ME(swoole_aerospike, list_size_async, arginfo_swoole_aerospike_list_size_async, ZEND_ACC_PUBLIC)
    //PHP_ME(swoole_aerospike, key_apply_async, arginfo_swoole_aerospike_key_apply_async, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

void swoole_aerospike_init(int module_number TSRMLS_DC)
{
    SWOOLE_INIT_CLASS_ENTRY(swoole_aerospike_ce, "swoole_aerospike", "Swoole\\Aerospike", swoole_aerospike_methods);
    swoole_aerospike_class_entry_ptr = zend_register_internal_class(&swoole_aerospike_ce TSRMLS_CC);
    init_aerospike_callback(&g_as_event_callback);
    
}

bool my_log_callback(as_log_level level, const char * func, const char * file, uint32_t line,
    const char * fmt, ...)
{
    char msg[1024] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, 1024, fmt, ap);
    msg[1023] = '\0';
    va_end(ap);
    switch (level) {
        case AS_LOG_LEVEL_INFO:
            //swInfo("[%s:%d][%s] %s", file, line, func, msg);
            break;
        case AS_LOG_LEVEL_WARN:
            swWarn("[%s:%d][%s] %s", file, line, func, msg);
            break;
        case AS_LOG_LEVEL_ERROR:
            swWarn("[%s:%d][%s] ERROR - %s", file, line, func, msg);
            break;
        default:
            //swInfo("[%s:%d][%s] %s", file, line, func, msg);
            break;
    }
    return true;
}

static PHP_METHOD(swoole_aerospike, __construct)
{
    swAerospikeClient *asClient = sw_malloc(sizeof(swAerospikeClient));
    bzero(asClient, sizeof(swAerospikeClient));
    
#if PHP_MAJOR_VERSION < 7
    asClient->object = getThis();
#else
    asClient->object = &asClient->_object;
    memcpy(asClient->object, getThis(), sizeof(zval));
#endif
    sw_zval_add_ref(&asClient->object);
    swoole_set_object(getThis(), asClient);
    php_swoole_check_reactor();
    php_swoole_check_timer(1000);
    if (0 == g_InitNum) {
        as_event_set_external_loop_capacity(1);
        as_event_set_external_loop(SwooleG.main_reactor);
        as_log_set_level(AS_LOG_LEVEL_INFO);
        as_log_set_callback(my_log_callback);
    }
    g_InitNum++;
}

bool sw_aerospike_close(swAerospikeClient* asClient) {
    aerospike* as = asClient->as;
    if (as != NULL) {
        as_error err;
        as_error_init(&err);
        if (AEROSPIKE_OK == aerospike_close(as, &err)) {
            return true;
        } else {
            swWarn("sw_aerospike_close close failed, err: %s.", as_error_string(err.code));
            return false;
        }
    }
    return true;
}

bool sw_aerospike_uninit(swAerospikeClient* asClient) {
    if (!asClient)
    {
        return false;
    }
    sw_aerospike_close(asClient);
    aerospike* as = asClient->as;
    if (as != NULL) {
        as_error err;
        as_error_init(&err);
        aerospike_destroy(as);
        asClient->is_init = false;
        asClient->as = NULL;
    }
    return true;
}

static PHP_METHOD(swoole_aerospike, __destruct)
{
    swAerospikeClient *asClient = swoole_get_object(getThis());
    if (!asClient)
    {
        return;
    }
    sw_aerospike_uninit(asClient);
    if (g_InitNum > 0) {
        if (1 == g_InitNum) {
            as_event_close_loops();
        }
        g_InitNum--;
    }

    sw_free(asClient);
}

static PHP_METHOD(swoole_aerospike, init)
{
    char *user;
    zend_size_t user_len;
    char *password;
    zend_size_t password_len;
    char *addrlist;
    zend_size_t addrlist_len;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &user, &user_len, &password, &password_len,
                              &addrlist, &addrlist_len) == FAILURE)
    {
        RETURN_FALSE;
    }
    
    if (addrlist_len <= 0) {
        swWarn("addrlist is empty.");
        RETURN_FALSE;
    }
    swAerospikeClient *asClient = swoole_get_object(getThis());
    if (!asClient->is_init) {
        as_config config;
        as_config_init(&config);
        char* s_addrlist = sw_malloc(addrlist_len + 1);
        memcpy(s_addrlist, addrlist, addrlist_len);
        char s_addr[32];
        char s_port[8];
        int i;
        for (i = 0; get_spilt(s_addrlist, ";", s_addr, sizeof(s_addr)) == 1; i++) {
            if (strlen(s_addr) > 0) {
                if (get_spilt(s_addr, ":", asClient->sz_iplist[i], sizeof(asClient->sz_iplist[i])) == 1) {
                    if (get_spilt(s_addr, ":", s_port, sizeof(s_port)) == 0) {
                        as_config_add_host(&config, asClient->sz_iplist[i], atoi(s_port));
                    }
                }
            }
        }
        if (strlen(s_addr) > 0) {
            if (get_spilt(s_addr, ":", asClient->sz_iplist[i], sizeof(asClient->sz_iplist[i])) == 1) {
                if (get_spilt(s_addr, ":", s_port, sizeof(s_port)) == 0) {
                    as_config_add_host(&config, asClient->sz_iplist[i], atoi(s_port));
                }
            }
        }
        if (user_len != 0) {
            as_config_set_user(&config, user, password);
        }
        aerospike* as = aerospike_new(&config);
        asClient->as = as;
        asClient->is_init = true;
        sw_free(s_addrlist);
    }
    RETURN_TRUE;
}

static PHP_METHOD(swoole_aerospike, uninit)
{
    swAerospikeClient *asClient = swoole_get_object(getThis());
    RETURN_BOOL(sw_aerospike_uninit(asClient));
}


static PHP_METHOD(swoole_aerospike, connect)
{
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as)) {
        RETURN_TRUE;
    }
    
    as_error err;
    as_error_init(&err);
    as_status status = aerospike_connect(as, &err);
    if (status == AEROSPIKE_OK) {
        RETURN_TRUE;
    } else {
        swWarn("aerospike connect failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, is_connected)
{
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    bool is_connected = aerospike_cluster_is_connected(as);
    RETURN_BOOL(is_connected);
}

static PHP_METHOD(swoole_aerospike, close)
{
    swAerospikeClient *asClient = swoole_get_object(getThis());
    RETURN_BOOL(sw_aerospike_close(asClient));
}

static PHP_METHOD(swoole_aerospike, get_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    
    if (namespace_len <= 0)
    {
        swWarn("get_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("get_async set is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("get_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("aerospike cluster is closed.");
        RETURN_FALSE;
    }
    
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_policy_read aspolicy;
    if (set_policy_read(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));

#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
    //udata->cb_data = userdata;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
    //udata->cb_data = &udata->_cb_data;
    //memcpy(udata->cb_data, userdata, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    //sw_zval_add_ref(&udata->cb_data);

    if (aerospike_key_get_async(as, &err, &aspolicy, &askey, swoole_as_async_record_cb, udata, NULL, NULL) ==AEROSPIKE_OK )
    {
        as_key_destroy(&askey);
        RETURN_TRUE;
        
    } else {
        as_key_destroy(&askey);
        sw_free(udata);
        swWarn("aerospike get_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, put_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    zval *record;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszaz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &record, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("put_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("put_async set is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("put_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("aerospike cluster is closed.");
        RETURN_FALSE;
    }
    
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_record asrecord;
    if (set_record(&asrecord, record) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }

    as_policy_write aspolicy;
    if (set_policy_write(&aspolicy, policy, (int*)&asrecord.ttl) == false) {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_FALSE;
    }
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    
    sw_zval_add_ref(&udata->cb);
    if (AEROSPIKE_OK == aerospike_key_put_async(as, &err, &aspolicy, &askey, &asrecord,
                                                swoole_as_async_write_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        sw_free(udata);
        swWarn("aerospike put_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, put_simple_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *bin_value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &bin_value,
                              &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    
    if (namespace_len <= 0)
    {
        swWarn("put_simple_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("put_simple_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("put_simple_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (Z_TYPE_P(bin_value) != IS_STRING && Z_TYPE_P(bin_value) != IS_LONG && Z_TYPE_P(bin_value) != IS_DOUBLE) {
        swWarn("put_simple_async bin_value is not string or long or double.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("put_simple_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_record asrecord;
    as_record_init(&asrecord, 1);
    if (Z_TYPE_P(bin_value) == IS_STRING) {
        if (set_record_bin_string(&asrecord, bin_name, Z_STRVAL_P(bin_value), Z_STRLEN_P(bin_value)) == false) {
            as_key_destroy(&askey);
            RETURN_FALSE;
        }
    } else if (Z_TYPE_P(bin_value) == IS_LONG) {
        if (set_record_bin_long(&asrecord, bin_name, Z_LVAL_P(bin_value)) == false) {
            as_key_destroy(&askey);
            RETURN_FALSE;
        }
    } else if (Z_TYPE_P(bin_value) == IS_DOUBLE) {
        if (set_record_bin_double(&asrecord, bin_name, Z_DVAL_P(bin_value)) == false) {
            as_key_destroy(&askey);
            RETURN_FALSE;
        }
    }
    
    as_policy_write aspolicy;
    if (set_policy_write(&aspolicy, policy, (int*)&asrecord.ttl) == false) {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_put_async(as, &err, &aspolicy, &askey, &asrecord,
                                                swoole_as_async_write_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        sw_free(udata);
        swWarn("aerospike put_simple_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, put_map_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *bin_value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &bin_value,
                              &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    
    if (namespace_len <= 0)
    {
        swWarn("put_map_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("put_map_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("put_map_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (Z_TYPE_P(bin_value) != IS_ARRAY) {
        swWarn("put_map_async bin_value is not array.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("put_map_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_record asrecord;
    as_record_init(&asrecord, 1);
    //如果传入的是list型，则也会转成map存储进入aerospike中
    if (set_record_bin_array_to_as_map(&asrecord, bin_name, bin_value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_write aspolicy;
    if (set_policy_write(&aspolicy, policy, (int*)&asrecord.ttl) == false) {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_put_async(as, &err, &aspolicy, &askey, &asrecord,
                                                swoole_as_async_write_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        sw_free(udata);
        swWarn("aerospike put_simple_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

//TODO
static PHP_METHOD(swoole_aerospike, put_list_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *bin_value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &bin_value,
                              &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    
    if (namespace_len <= 0)
    {
        swWarn("put_list_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("put_list_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("put_list_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (Z_TYPE_P(bin_value) != IS_ARRAY) {
        swWarn("put_list_async bin_value is not array.");
        RETURN_FALSE;
    }
    
    if (check_array_list(bin_value) == false) {
        swWarn("put_list_async bin_value is not list array.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("put_list_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_record asrecord;
    as_record_init(&asrecord, 1);
    if (set_record_bin_array_to_as_list(&asrecord, bin_name, bin_value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    
    as_policy_write aspolicy;
    if (set_policy_write(&aspolicy, policy, (int*)&asrecord.ttl) == false) {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_put_async(as, &err, &aspolicy, &askey, &asrecord,
                                                swoole_as_async_write_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_record_destroy(&asrecord);
        sw_free(udata);
        swWarn("aerospike put_simple_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, key_select_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    zval *bin_name_list;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name_list, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("key_select_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("key_select_async set is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("key_select_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("key_select_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_policy_read aspolicy;
    if (set_policy_read(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    
    char **bins = NULL;
    if (set_bin_name_list(&bins, bin_name_list) == false) {
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);

    if (AEROSPIKE_OK == aerospike_key_select_async(as, &err, &aspolicy, &askey, (const char**)bins,
                                                   swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        if (bins) {
            sw_free(bins);
        }
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        sw_free(udata);
        if (bins) {
            sw_free(bins);
        }
        swWarn("aerospike key_select_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, key_exists_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("key_exists_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("key_exists_async set is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("key_exists_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("key_exists_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_policy_read aspolicy;
    if (set_policy_read(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_exists_async(as, &err, &aspolicy, &askey, swoole_as_async_record_cb,
                                                   udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        sw_free(udata);
        swWarn("aerospike key_exists_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, key_remove_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("key_remove_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("key_remove_async set is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("key_remove_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("key_remove_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_policy_remove aspolicy;
    if (set_policy_remove(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_remove_async(as, &err, &aspolicy, &askey, swoole_as_async_write_cb,
                                                   udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        sw_free(udata);
        swWarn("aerospike key_remove_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}


static PHP_METHOD(swoole_aerospike, key_operate_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    zval *operations;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &operations, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("key_operate_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("key_operate_async set is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("key_operate_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("key_operate_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_operations asoperations;
    if (set_as_operations(&asoperations, operations) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                   swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike key_operate_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, incr_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &value, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("incr_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("incr_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("incr_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("incr_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("incr_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_operations asoperations;
    if (set_as_operations_add_incr(&asoperations, bin_name, value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike incr_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, prepend_strp_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &value, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("prepend_strp_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("prepend_strp_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("prepend_strp_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("prepend_strp_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("prepend_strp_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_operations asoperations;
    
    if (set_as_operations_prepend_strp(&asoperations, bin_name, value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike prepend_strp_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, append_strp_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &value, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("append_strp_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("append_strp_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("append_strp_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("append_strp_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("append_strp_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_operations asoperations;
    
    if (set_as_operations_append_strp(&asoperations, bin_name, value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike append_strp_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_append_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &value, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_append_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_append_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_append_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_append_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_append_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_append(&asoperations, bin_name, value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_append_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_insert_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &value, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_insert_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_insert_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_insert_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_insert_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_insert_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_insert_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_insert(&asoperations, bin_name, Z_LVAL_P(index), value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_insert_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_pop_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_pop_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_pop_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_pop_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_pop_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn( "list_pop_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_pop_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_pop(&asoperations, bin_name, Z_LVAL_P(index)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_pop_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_pop_range_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *count;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &count, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_pop_range_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_pop_range_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_pop_range_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_pop_range_async index is not long type.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(count) != IS_LONG ) {
        swWarn("list_pop_range_async count is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_pop_range_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_pop_range_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_pop_range(&asoperations, bin_name, Z_LVAL_P(index), Z_LVAL_P(count)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_pop_range_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_pop_range_from_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_pop_range_from_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_pop_range_from_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_pop_range_from_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_pop_range_from_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_pop_range_from_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_pop_range_from_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_pop_range_from(&asoperations, bin_name, Z_LVAL_P(index)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_pop_range_from_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_remove_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_remove_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_remove_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_remove_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_remove_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_remove_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_remove_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_remove(&asoperations, bin_name, Z_LVAL_P(index)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_remove_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_remove_range_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *count;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &count, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_remove_range_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_remove_range_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_remove_range_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_remove_range_async index is not long type.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(count) != IS_LONG ) {
        swWarn("list_remove_range_async count is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_remove_range_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_remove_range_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_remove_range(&asoperations, bin_name, Z_LVAL_P(index), Z_LVAL_P(count)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_remove_range_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_remove_range_from_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_remove_range_from_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_remove_range_from_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_remove_range_from_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_remove_range_from_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_remove_range_from_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_remove_range_from_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_remove_range_from(&asoperations, bin_name, Z_LVAL_P(index)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_remove_range_from_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_clear_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszsz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_clear_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_clear_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_clear_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_clear_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_clear_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_clear(&asoperations, bin_name) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_clear_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_set_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *value;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &value, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_set_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_set_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_set_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_set_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_set_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_set_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_set(&asoperations, bin_name, Z_LVAL_P(index), value) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_set_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_trim_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *count;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &count, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_trim_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_trim_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_trim_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_trim_async index is not long type.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(count) != IS_LONG ) {
        swWarn("list_trim_async count is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_trim_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_trim_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_trim(&asoperations, bin_name, Z_LVAL_P(index), Z_LVAL_P(count)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_trim_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_get_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_get_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_get_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_get_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_get_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_get_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_get_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_get(&asoperations, bin_name, Z_LVAL_P(index)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_get_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_get_range_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *count;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszzz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &count, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_get_range_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_get_range_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_get_range_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_get_range_async index is not long type.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(count) != IS_LONG ) {
        swWarn("list_get_range_async count is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_get_range_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_get_range_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_get_range(&asoperations, bin_name, Z_LVAL_P(index), Z_LVAL_P(count)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_get_range_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_get_range_from_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *index;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszszz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &index, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_get_range_from_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_get_range_from_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_get_range_from_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    if (SW_Z_TYPE_P(index) != IS_LONG ) {
        swWarn("list_get_range_from_async index is not long type.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_get_range_from_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_get_range_from_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_get_range_from(&asoperations, bin_name, Z_LVAL_P(index)) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_get_range_from_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

static PHP_METHOD(swoole_aerospike, list_size_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    char *bin_name;
    zend_size_t bin_name_len;
    zval *callback;
    zval *policy = NULL;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszsz|z", &namespace, &namespace_len, &set,
                              &set_len, &key, &bin_name, &bin_name_len, &callback, &policy) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("list_size_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("list_size_async set is empty.");
        RETURN_FALSE;
    }
    
    if (bin_name_len <= 0) {
        swWarn("list_size_async bin_name is empty.");
        RETURN_FALSE;
    }
    
    char *func_name = NULL;
    if (!sw_zend_is_callable(callback, 0, &func_name TSRMLS_CC))
    {
        swWarn("list_size_async function '%s' is not callable.", func_name);
        efree(func_name);
        RETURN_FALSE;
    }
    efree(func_name);
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("list_size_async aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    
    as_operations asoperations;
    if (set_as_operations_list_size(&asoperations, bin_name) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_policy_operate aspolicy;
    if (set_policy_operate(&aspolicy, policy) == false) {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_FALSE;
    }
    
    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    
    if (AEROSPIKE_OK == aerospike_key_operate_async(as, &err, &aspolicy, &askey, &asoperations,
                                                    swoole_as_async_record_cb, udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_operations_destroy(&asoperations);
        sw_free(udata);
        swWarn("aerospike list_size_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}

/*
static PHP_METHOD(swoole_aerospike, key_apply_async)
{
    char *namespace;
    zend_size_t namespace_len;
    char *set;
    zend_size_t set_len;
    zval *key;
    zval *policy_apply;
    zval *callback;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sszzz", &namespace, &namespace_len, &set,
                              &set_len, &key, &policy_apply, &callback) == FAILURE)
    {
        RETURN_FALSE;
    }
    if (namespace_len <= 0)
    {
        swWarn("key_apply_async namespace is empty.");
        RETURN_FALSE;
    }
    
    if (set_len <= 0)
    {
        swWarn("key_apply_async set is empty.");
        RETURN_FALSE;
    }
    
    swAerospikeClient *asClient = swoole_get_object(getThis());
    aerospike* as = asClient->as;
    if (aerospike_cluster_is_connected(as) == false) {
        swWarn("aerospike cluster is closed.");
        RETURN_FALSE;
    }
    as_error err;
    as_error_init(&err);
    as_key askey;
    if (set_as_key(&askey, namespace, set, key) == false) {
        RETURN_FALSE;
    }
    as_policy_apply aspolicy;
    if (set_policy_apply(&aspolicy, policy_apply) == false) {
        as_key_destroy(&askey);
        RETURN_FALSE;
    }
    as_arraylist args;
    as_arraylist_inita(&args, 0);

    swCallBackData* udata = sw_malloc(sizeof(swCallBackData));
    
#if PHP_MAJOR_VERSION < 7
    udata->cb = callback;
    //udata->cb_data = userdata;
#else
    udata->cb = &udata->_cb;
    memcpy(udata->cb, callback, sizeof(zval));
    //udata->cb_data = &udata->_cb_data;
    //memcpy(udata->cb_data, userdata, sizeof(zval));
#endif
    sw_zval_add_ref(&udata->cb);
    //sw_zval_add_ref(&udata->cb_data);
    printf("key_apply_async 2222222222\n");

    if (AEROSPIKE_OK == aerospike_key_apply_async(as, &err, &aspolicy, &askey, "", "", &args,
                                                  swoole_as_async_value_cb , udata, NULL, NULL))
    {
        as_key_destroy(&askey);
        as_arraylist_destroy(&args);
        RETURN_TRUE;
    } else {
        as_key_destroy(&askey);
        as_arraylist_destroy(&args);
        sw_free(udata);
        swWarn("aerospike key_apply_async failed, err: %s.", as_error_string(err.code));
        RETURN_FALSE;
    }
}
*/

#endif
