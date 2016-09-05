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
#include "sw_aerospike_event_loop.h"

#define AS_EVENT_WRITE_COMPLETE 0
#define AS_EVENT_WRITE_INCOMPLETE 1
#define AS_EVENT_WRITE_ERROR 2

int swoole_as_event_write(as_event_command* cmd)
{
    int fd = cmd->conn->fd;
    ssize_t bytes;
    
    do {
#if defined(__linux__)
        bytes = send(fd, cmd->buf + cmd->pos, cmd->len - cmd->pos, MSG_NOSIGNAL);
#else
        bytes = write(fd, cmd->buf + cmd->pos, cmd->len - cmd->pos);
#endif
        if (bytes > 0) {
            cmd->pos += bytes;
            continue;
        }
        
        if (bytes < 0) {
            if (errno == EWOULDBLOCK) {
                return AS_EVENT_WRITE_INCOMPLETE;
            }
            
            as_error err;
            as_error_update(&err, AEROSPIKE_ERR_ASYNC_CONNECTION, "Socket %d write failed: %d", fd, errno);
            as_event_socket_error(cmd, &err);
            swWarn("Socket %d write failed: %d.", fd, errno);
            return AS_EVENT_WRITE_ERROR;
        }
        else {
            as_error err;
            as_error_update(&err, AEROSPIKE_ERR_ASYNC_CONNECTION, "Socket %d write closed by peer", fd);
            as_event_socket_error(cmd, &err);
            swWarn("Socket %d write closed by peer.", fd);
            return AS_EVENT_WRITE_ERROR;
        }
    } while (cmd->pos < cmd->len);
    
    return AS_EVENT_WRITE_COMPLETE;
}

bool swoole_as_event_read(as_event_command* cmd)
{
//    swWarn("swoole_as_event_read.");
    int fd = cmd->conn->fd;
    ssize_t bytes;
    
    do {
        bytes = read(fd, cmd->buf + cmd->pos, cmd->len - cmd->pos);
        
        if (bytes > 0) {
            cmd->pos += bytes;
            continue;
        }
        
        if (bytes < 0) {
            if (errno != EWOULDBLOCK) {
                as_error err;
                as_error_update(&err, AEROSPIKE_ERR_ASYNC_CONNECTION, "Socket %d read failed: %d", fd, errno);
                as_event_socket_error(cmd, &err);
                swWarn("Socket %d read failed: %d", fd, errno);
            }
            return false;
        }
        else {
            as_error err;
            as_error_update(&err, AEROSPIKE_ERR_ASYNC_CONNECTION, "Socket %d read closed by peer", fd);
            as_event_socket_error(cmd, &err);
            swWarn("Socket %d read closed by peer", fd);
            return false;
        }
    } while (cmd->pos < cmd->len);
    
    return true;
}

void swoole_as_event_command_peek_block(as_event_command* cmd)
{
    // Batch, scan, query may be waiting on end block.
    // Prepare for next message block.
    cmd->len = sizeof(as_proto);
    cmd->pos = 0;
    cmd->state = AS_ASYNC_STATE_READ_HEADER;

    if (! swoole_as_event_read(cmd)) {
        return;
    }
    
    as_proto* proto = (as_proto*)cmd->buf;
    as_proto_swap_from_be(proto);
    size_t size = proto->sz;
    
    cmd->len = (uint32_t)size;
    cmd->pos = 0;
    cmd->state = AS_ASYNC_STATE_READ_BODY;
    
    // Check for end block size.
    if (cmd->len == sizeof(as_msg)) {
        // Look like we received end block.  Read and parse to make sure.
        if (! swoole_as_event_read(cmd)) {
            return;
        }
        
        if (! cmd->parse_results(cmd)) {
            // We did not finish after all. Prepare to read next header.
            cmd->len = sizeof(as_proto);
            cmd->pos = 0;
            cmd->state = AS_ASYNC_STATE_READ_HEADER;
        }
    }
    else {
        // Received normal data block.  Stop reading for fairness reasons and wait
        // till next iteration.
        if (cmd->len > cmd->capacity) {
            if (cmd->free_buf) {
                cf_free(cmd->buf);
            }
            cmd->buf = cf_malloc(size);
            cmd->capacity = cmd->len;
            cmd->free_buf = true;
        }
    }
}

void swoole_as_event_watch_read(as_event_command* cmd)
{
    as_event_connection* conn = cmd->conn;
    swReactor* reactor = (swReactor*)(cmd->event_loop->loop);

    //为了避免swoole_epoll, del过程中对已删除的再次删除会打印错误日志，增加该判断条件
    if (!conn->del) {
        reactor->del(reactor, conn->fd);
    }

    if (reactor->add(reactor, conn->fd, PHP_SWOOLE_FD_AEROSPIKE | SW_EVENT_READ) < 0)
    {
        swWarn("swoole_as_event_watch_read:swoole_event_add failed.");
        return;
    }
    conn->del = false;
}

void swoole_as_event_watch_write(as_event_command* cmd)
{
    as_event_connection* conn = cmd->conn;
    swReactor* reactor = (swReactor*)(cmd->event_loop->loop);

    if (!conn->del) {
        reactor->del(reactor, conn->fd);
    }

    if (reactor->add(reactor, conn->fd, PHP_SWOOLE_FD_AEROSPIKE | SW_EVENT_WRITE) < 0)
    {
        swWarn("swoole_as_event_watch_write:swoole_event_add failed.");
        return;
    }
    conn->del = false;
}

void swoole_as_event_command_read_start(as_event_command* cmd)
{
    cmd->len = sizeof(as_proto);
    cmd->pos = 0;
    cmd->state = AS_ASYNC_STATE_READ_HEADER;
    
    swoole_as_event_watch_read(cmd);
    
    if (cmd->pipe_listener != NULL) {
        as_pipe_read_start(cmd);
    }
}

void swoole_as_event_command_write_start(as_event_command* cmd)
{
    cmd->state = AS_ASYNC_STATE_WRITE;
    
    int ret = swoole_as_event_write(cmd);
    
    if (ret == AS_EVENT_WRITE_COMPLETE) {
        // Done with write. Register for read.
        swoole_as_event_command_read_start(cmd);
        return;
    }
    
    if (ret == AS_EVENT_WRITE_INCOMPLETE) {
        // Got would-block. Register for write.
        swoole_as_event_watch_write(cmd);
    }
}

void swoole_as_event_parse_authentication(as_event_command* cmd)
{
    if (cmd->state == AS_ASYNC_STATE_AUTH_READ_HEADER) {
        // Read response length
        if (! swoole_as_event_read(cmd)) {
            return;
        }
        as_event_set_auth_parse_header(cmd);
        
        if (cmd->len > cmd->capacity) {
            as_error err;
            as_error_update(&err, AEROSPIKE_ERR_CLIENT, "Authenticate response size is corrupt: %u", cmd->auth_len);
            as_event_socket_error(cmd, &err);
            swWarn("Authenticate response size is corrupt: %u", cmd->auth_len);
            return;
        }
    }
    
    if (! swoole_as_event_read(cmd)) {
        return;
    }
    
    // Parse authentication response.
    cmd->len -= cmd->auth_len;
    uint8_t code = cmd->buf[cmd->len + AS_ASYNC_AUTH_RETURN_CODE];
    
    if (code) {
        // Can't authenticate socket, so must close it.
        as_error err;
        as_error_update(&err, code, "Authentication failed: %s", as_error_string(code));
        as_event_socket_error(cmd, &err);
        swWarn("Authentication failed: %s", as_error_string(code));
        return;
    }
    
    cmd->pos = 0;
    swoole_as_event_command_write_start(cmd);
}


void swoole_as_event_command_read(as_event_command* cmd)
{
    // Check for authenticate read-header or read-body.
    if (cmd->state & (AS_ASYNC_STATE_AUTH_READ_HEADER | AS_ASYNC_STATE_AUTH_READ_BODY)) {
        swoole_as_event_parse_authentication(cmd);
        return;
    }
    
    if (cmd->state == AS_ASYNC_STATE_READ_HEADER) {
        // Read response length
        if (! swoole_as_event_read(cmd)) {
            return;
        }
        
        as_proto* proto = (as_proto*)cmd->buf;
        as_proto_swap_from_be(proto);
        size_t size = proto->sz;
        
        cmd->len = (uint32_t)size;
        cmd->pos = 0;
        cmd->state = AS_ASYNC_STATE_READ_BODY;
        
        if (cmd->len > cmd->capacity) {
            if (cmd->free_buf) {
                cf_free(cmd->buf);
            }
            cmd->buf = cf_malloc(size);
            cmd->capacity = cmd->len;
            cmd->free_buf = true;
        }
    }
    // Read response body
    if (! swoole_as_event_read(cmd)) {
        return;
    }
    if (! cmd->parse_results(cmd)) {
        // Batch, scan, query is not finished.
        swoole_as_event_command_peek_block(cmd);
    }
}

void swoole_as_event_timeout(void* data)
{
    //printf("swoole_as_event_timeout.\n");
    as_event_timeout((as_event_command*)data);
}

int swoole_aerospike_onRead(swReactor *reactor, swEvent *event) {
    int fd = event->fd;
    swConnection* swConn = swReactor_get(reactor, fd);
    as_event_connection* conn = swConn->user_data;
    if (conn == NULL) {
        return 0;
    }
    as_event_command* cmd;
    
    if (conn->pipeline) {
        as_pipe_connection* pipe = (as_pipe_connection*)conn;
        
        if (pipe->writer && cf_ll_size(&pipe->readers) == 0) {
            // Authentication response will only have a writer.
            cmd = pipe->writer;
        }
        else {
            // Next response is at head of reader linked list.
            cf_ll_element* link = cf_ll_get_head(&pipe->readers);
            
            if (link) {
                cmd = as_pipe_link_to_command(link);
            }
            else {
                as_log_debug("Pipeline read event ignored");
                return 0;
            }
        }
    }
    else {
        cmd = ((as_async_connection*)conn)->cmd;
    }
    swoole_as_event_command_read(cmd);
    return 0;
}

int swoole_aerospike_onWrite(swReactor *reactor, swEvent *event) {
    int fd = event->fd;
    swConnection* swConn = swReactor_get(reactor, fd);
    as_event_connection* conn = swConn->user_data;
    if (conn == NULL) {
        return 0;
    }
    as_event_command* cmd = conn->pipeline ? ((as_pipe_connection*)conn)->writer : ((as_async_connection*)conn)->cmd;
    
    int ret = swoole_as_event_write(cmd);
    
    if (ret == AS_EVENT_WRITE_COMPLETE) {
        // Done with write. Register for read.
        if (cmd->state == AS_ASYNC_STATE_AUTH_WRITE) {
            as_event_set_auth_read_header(cmd);
            swoole_as_event_watch_read(cmd);
        }
        else {
            swoole_as_event_command_read_start(cmd);
        }
    }
    return 0;
}

int swoole_aerospike_onError(swReactor* reactor, swEvent *event) {
}

void swoole_as_event_watcher_init(as_event_command* cmd, int fd)
{
    if (cmd->cluster->user) {
        as_event_set_auth_write(cmd);
        cmd->state = AS_ASYNC_STATE_AUTH_WRITE;
    }
    else {
        cmd->state = AS_ASYNC_STATE_WRITE;
    }
    
    as_event_connection* conn = cmd->conn;
    conn->fd = fd;
    conn->del = false;
    swReactor* reactor = (swReactor*)cmd->event_loop->loop;
    reactor->setHandle(reactor, PHP_SWOOLE_FD_AEROSPIKE | SW_EVENT_READ, swoole_aerospike_onRead);
    reactor->setHandle(reactor, PHP_SWOOLE_FD_AEROSPIKE | SW_EVENT_WRITE, swoole_aerospike_onWrite);
    reactor->setHandle(reactor, PHP_SWOOLE_FD_AEROSPIKE | SW_EVENT_ERROR, swoole_aerospike_onError);

    if (reactor->add(reactor, fd, PHP_SWOOLE_FD_AEROSPIKE | SW_EVENT_WRITE) < 0)
    {
        swWarn("swoole_as_event_watcher_init:swoole_event_add failed.");
        return;
    }
    
    swConnection* swconn = swReactor_get(reactor, fd);
    if (swconn != NULL) {
        swconn->user_data = conn;
    }
    else {
        swWarn("swoole_as_event_watcher_init:swReactor_get failed.");
        return;
    }
}

void swoole_as_event_connect(as_event_command* cmd)
{
    int fd = as_event_create_socket(cmd);
    
    if (fd < 0) {
        swWarn("swoole_as_event_connect: create socket failed.");
        return;
    }
    
    // Try primary address.
    as_node* node = cmd->node;
    as_address* primary = as_vector_get(&node->addresses, node->address_index);
    
    // Attempt non-blocking connection.
    if (connect(fd, (struct sockaddr*)&primary->addr, sizeof(struct sockaddr)) == 0) {
        swoole_as_event_watcher_init(cmd, fd);
        return;
    }
    
    // Check if connection is in progress.
    if (errno == EINPROGRESS) {
        swoole_as_event_watcher_init(cmd, fd);
        return;
    }
    
    // Try other addresses.
    as_vector* addresses = &node->addresses;
    for (uint32_t i = 0; i < addresses->size; i++) {
        as_address* address = as_vector_get(addresses, i);
        
        // Address points into alias array, so pointer comparison is sufficient.
        if (address != primary) {
            if (connect(fd, (struct sockaddr*)&address->addr, sizeof(struct sockaddr)) == 0) {
                // Replace invalid primary address with valid alias.
                // Other threads may not see this change immediately.
                // It's just a hint, not a requirement to try this new address first.
                as_log_debug("Change node address %s %s:%d", node->name, address->name, (int)cf_swap_from_be16(address->addr.sin_port));
                swWarn("Change node address %s %s:%d", node->name, address->name, (int)cf_swap_from_be16(address->addr.sin_port));
                ck_pr_store_32(&node->address_index, i);
                swoole_as_event_watcher_init(cmd, fd);
                return;
            }
            
            // Check if connection is in progress.
            if (errno == EINPROGRESS) {
                // Replace invalid primary address with valid alias.
                // Other threads may not see this change immediately.
                // It's just a hint, not a requirement to try this new address first.
                as_log_debug("Change node address %s %s:%d", node->name, address->name, (int)cf_swap_from_be16(address->addr.sin_port));
                swWarn("Change node address %s %s:%d", node->name, address->name, (int)cf_swap_from_be16(address->addr.sin_port));
                ck_pr_store_32(&node->address_index, i);
                
                // Connection hasn't finished.
                swoole_as_event_watcher_init(cmd, fd);
                return;
            }
        }
    }
    
    // Failed to start a connection on any socket address.
    as_error err;
    as_error_update(&err, AEROSPIKE_ERR_ASYNC_CONNECTION, "Failed to connect: %s %s:%d",
                    node->name, primary->name, (int)cf_swap_from_be16(primary->addr.sin_port));
    as_event_connect_error(cmd, &err, fd);
}


/*********************************************************************
 call back
 *********************************************************************/
int swoole_as_event_validate_connection_callback(as_event_connection* conn) {
    return as_socket_validate(conn->fd);
}

void swoole_as_event_stop_timer_callback(as_event_command* cmd) {
    if (cmd->timeout_ms) {
        //ev_timer_stop(cmd->event_loop->loop, &cmd->timer);
        swTimer_node* node = (swTimer_node*)cmd->timer;
        if (node != NULL) {
            if (node->data) {
                sw_free(node->data);
                node->data = NULL;
            }
            swTimer_del(&SwooleG.timer, node);
        }
    }
}

void swoole_as_event_stop_watcher_callback(as_event_command* cmd, as_event_connection* conn) {
    swReactor* reactor = (swReactor*)cmd->event_loop->loop;
    if (!conn->del) {
        reactor->del(reactor, conn->fd);
        conn->del = true;
    }
}

void swoole_as_event_command_release_callback(as_event_command* cmd) {
    as_event_command_free(cmd);
}

bool swoole_as_event_create_loop_callback(as_event_loop* event_loop) {
    //swoole中不创建loop,而是直接使用main_reactor
    return false;
}

//TODO
void swoole_as_event_register_external_loop_callback(as_event_loop *event_loop) {
    // This method is only called when user sets an external event loop.
    as_queue_init(&event_loop->queue, sizeof(void*), AS_EVENT_QUEUE_INITIAL_CAPACITY);
}

//同一线程不会被执行到
bool swoole_as_event_send_callback(as_event_command* cmd) {
    
    // Notify other event loop thread that queue needs to be processed.
    printf("bbbbbbbbbbbbb\n");
    return false;
}

void swoole_as_event_command_begin_callback(as_event_command* cmd) {
    // Always initialize timer first when timeouts are specified.
    if (cmd->timeout_ms) {
        swReactor* reactor = (swReactor*)cmd->event_loop->loop;
        asTimeCallBack* cb_data = sw_malloc(sizeof(asTimeCallBack));
        cb_data->cb = swoole_as_event_timeout;
        cb_data->data = cmd;
        swTimer_node* node = swTimer_add(&SwooleG.timer, (double)cmd->timeout_ms, 0, cb_data);
        if (node != NULL) {
            node->php_used = 0;
            cmd->timer = node;
        } else {
            cmd->timer = NULL;
            sw_free(cb_data);
        }
    }
    
    as_connection_status status = cmd->pipe_listener != NULL ? as_pipe_get_connection(cmd) : as_event_get_connection(cmd);
    
    if (status == AS_CONNECTION_FROM_POOL) {
        swoole_as_event_command_write_start(cmd);
    }
    else if (status == AS_CONNECTION_NEW) {
        swoole_as_event_connect(cmd);
    }
}

void swoole_as_event_close_connection_callback(as_event_connection* conn) {
    //swWarn("swoole_as_event_close_connection_callback.");
    //swconnection->close 调用，释放相关对象？？
    if (conn) {
        close(conn->fd);
        cf_free(conn);
    }
}

void as_ev_close_connections(as_node* node, as_queue* conn_queue)
{
    as_event_connection* conn;
    // Queue connection commands to event loops.
    while (as_queue_pop(conn_queue, &conn)) {
        close(conn->fd);
        cf_free(conn);
        as_event_decr_connection(node->cluster, conn_queue);
        ck_pr_dec_32(&node->cluster->async_conn_pool);
    }
    as_queue_destroy(conn_queue);
}

void swoole_as_event_node_destroy_callback(as_node* node) {
    // Close connections.
    for (uint32_t i = 0; i < as_event_loop_size; i++) {
        as_ev_close_connections(node, &node->async_conn_qs[i]);
        as_ev_close_connections(node, &node->pipe_conn_qs[i]);
    }
    cf_free(node->async_conn_qs);
    cf_free(node->pipe_conn_qs);
}

bool swoole_as_event_send_close_loop_callback(as_event_loop* event_loop) {
    // swoole event loop 都在一个线程中
    // Send stop command through queue so it can be executed in event loop thread.
    printf("swoole_as_event_send_close_loop_callback 22222222\n");
    return false;
}

void swoole_as_event_close_loop_callback(as_event_loop* event_loop) {
    // Cleanup event loop resources.
    swReactor* reactor = (swReactor*)event_loop->loop;
    
    as_queue_destroy(&event_loop->queue);
    as_queue_destroy(&event_loop->pipe_cb_queue);
    pthread_mutex_unlock(&event_loop->lock);
    pthread_mutex_destroy(&event_loop->lock);
}

//设置aerospike的回调函数
void init_aerospike_callback(as_event_swoole_callback* as_event_callback) {
    /*
    if (as_event_callback == NULL) {
        as_event_callback = sw_malloc(sizeof(as_event_swoole_callback));
        if (as_event_callback == NULL)
        {
            swoole_error_log(SW_LOG_ERROR, SW_ERROR_MALLOC_FAIL, "malloc as_event_swoole_callback failed.");
            return;
        }
    }
    bzero(as_event_callback, sizeof(as_event_swoole_callback));
    */
    as_event_callback->validate_connection_callback = swoole_as_event_validate_connection_callback;
    as_event_callback->stop_timer_callback = swoole_as_event_stop_timer_callback;
    as_event_callback->stop_watcher_callback = swoole_as_event_stop_watcher_callback;
    as_event_callback->command_release_callback = swoole_as_event_command_release_callback;
    as_event_callback->create_loop_callback = swoole_as_event_create_loop_callback;
    as_event_callback->register_external_loop_callback = swoole_as_event_register_external_loop_callback;
    as_event_callback->send_callback = swoole_as_event_send_callback;
    as_event_callback->command_begin_callback = swoole_as_event_command_begin_callback;
    as_event_callback->close_connection_callback = swoole_as_event_close_connection_callback;
    as_event_callback->node_destroy_callback = swoole_as_event_node_destroy_callback;
    as_event_callback->send_close_loop_callback = swoole_as_event_send_close_loop_callback;
    as_event_callback->close_loop_callback = swoole_as_event_close_loop_callback;
    as_event_set_swoole_callback(as_event_callback);
}

void uninit_aerospike_callback(as_event_swoole_callback* as_event_callback) {
    //sw_free(as_event_callback);
    as_event_set_swoole_callback(NULL);
}


#endif
