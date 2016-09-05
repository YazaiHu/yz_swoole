#aerospike_async接口说明
##API接口说明

###1.new

###2.init(user, password, ip_list)
	初始化
	@param user:用户名
	@param passwrod:密码
	@param ip_list:地址列表，格式为 "1.1.1.1:30;2.2.2.2:60"
	@return 正常返回true, 异常返回false
		
###3.connect()
	建立连接
	@return 正常返回true, 异常返回false

###4.is_connected()
	判断是否连接上
	@return 已连接返回true,未连接返回false
	
###5.close()
	断开连接
	@return 成功返回true

###6.uninit()
	清理内存资源
	@return 成功返回true
	
###7.put_simple_async(namespace, set_name, key_name, bin_name, bin_value, write_callback, policy)
	@param namespace:命名空间,必须为字符串,下同
	@param set_name:见参数说明
	@param key_name:见参数说明
	@param bin_name:见参数说明
	@param bin_value:格式必须为stirng|long|double中的一种，否则返回值为false
	@param write_callback:回调函数,详见参数说明
	@param policy:策略，格式为map,能识别的key为,
			"timeout": 该接口调用超时时间，默认为1秒，单位毫秒;
			"retry": 默认值为1
			"compression_threshold": 默认值为0
			"key": 可选值为AS_POLICY_KEY_SEND=1, AS_POLICY_KEY_DIGEST=0, 默认值为AS_POLICY_KEY_SEND
			"gen": 可选值为AS_POLICY_GEN_IGNORE＝0, AS_POLICY_GEN_EQ＝1, AS_POLICY_GEN_GT=2, 默认值为AS_POLICY_GEN_IGNORE
			"exists": 可选值为AS_POLICY_EXISTS_IGNORE＝0, AS_POLICY_EXISTS_CREATE=1, AS_POLICY_EXISTS_UPDATE=2, AS_POLICY_EXISTS_REPLACE=3, AS_POLICY_EXISTS_CREATE_OR_REPLACE=4, 默认值为AS_POLICY_EXISTS_IGNORE
			"commit_level": 可选值为AS_POLICY_COMMIT_LEVEL_ALL＝0, AS_POLICY_COMMIT_LEVEL_MASTER=1, 默认值为AS_POLICY_COMMIT_LEVEL_ALL
			"ttl":存储时间，默认值为0,单位秒,详见参数说明
			详见as库的policy_write
	@return 正常返回true,参数异常返回false

	
	
###8.put_list_async(namespace, set_name, key_name, bin_name, bin_value, write_callback, policy)
	@param namespace:
	@param set_name:
	@param key_name:
	@param bin_name:
	@param bin_value:必须为list型array,格式如: ["aaaa", 123, $objtest, [1,2,3], ["1"=>2]],否则返回false
	@param write_callback:回调函数,详见参数说明
	@param policy:策略，同put_simple_async
	@return 正常返回true,参数异常返回false

###9.put_map_async(namespace, set_name, key_name, bin_name, bin_value, write_callback, policy)
	@param namespace:
	@param set_name:
	@param key_name:
	@param bin_name:
	@param bin_value:array格式
	*	注意:如果为list型array,则会当成index为key的map存储!!!!!!!!!!
	@param write_callback:回调函数,详见参数说明
	@param policy:策略，同put_simple_async
	@return 正常返回true,参数异常返回false

###10.put_async(namespace, set_name, key_name, bin_record_list, write_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_record_list: bin_name与bin_value的集合，array格式，bin_value支持php_object,字符串,数字,list型与map型array, array: ["bin_name1"=>$json_object, "bin_name2"=>123, "bin_name3"=>"bin_value"] 
	*	注意:如果bin_name为数字,则会转成字符串存储于aerospike中,与其他强类型语言共用这一块数据需要注意!!!!!!!
	@param write_callback:回调函数,详见参数说明
	@param policy:策略，同put_simple_async
	@return 正常返回true,参数异常返回false

###11.get_async(namespace, set_name, key_name, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，格式为map, 能识别的key为
			"timeout": 该接口调用超时时间，默认为1秒，单位毫秒;
			"retry": 默认值为1
			"key":可选值为AS_POLICY_KEY_SEND=1, AS_POLICY_KEY_DIGEST=0, 默认值为AS_POLICY_KEY_SEND
			"replica":可选值为AS_POLICY_REPLICA_MASTER＝0，AS_POLICY_REPLICA_ANY＝1，默认值为AS_POLICY_REPLICA_MASTER
			"consistency_level":可选值为AS_POLICY_CONSISTENCY_LEVEL_ONE＝0，AS_POLICY_CONSISTENCY_LEVEL_ALL＝1，默认值为AS_POLICY_CONSISTENCY_LEVEL_ONE
			"deserialize":可选值为1或者0，默认值为1
			详见as库的policy_read
	@return 正常返回true,参数异常返回false	
	
###12.key_select_async(namespace, set_name, key_name, bin_name_list, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name_list: list型array,  
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同get_async
	@return 正常返回true,参数异常返回false	
	
###13.key_exists_async(namespace, set_name, key_name, record_callback, policy)	
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同get_async
	@return 正常返回true,参数异常返回false	
	
###14.key_remove_async(namespace, set_name, key_name, write_callback, policy)	
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param write_callback:回调函数,详见参数说明
	@param policy:策略，格式为map, 能识别的key为
			"timeout": 该接口调用超时时间，默认为1秒，单位毫秒;
			"retry": 默认值为1
			"key":可选值为AS_POLICY_KEY_SEND=1, AS_POLICY_KEY_DIGEST=0, 默认值为AS_POLICY_KEY_SEND
			"gen":可选值为AS_POLICY_GEN_IGNORE＝0, AS_POLICY_GEN_EQ＝1, AS_POLICY_GEN_GT=2, 默认值为AS_POLICY_GEN_IGNORE
			"generation":默认值为0
			"commit_level":可选值为AS_POLICY_COMMIT_LEVEL_ALL＝0, AS_POLICY_COMMIT_LEVEL_MASTER=1, 默认值为AS_POLICY_COMMIT_LEVEL_ALL
			详见as库policy_remove
	@return 正常返回true,参数异常返回false	
	
###15.incr_async(namespace, set_name, key_name, bin_name, value, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param value: 想要增加的值，支持long, double两种类型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，格式为map, 能识别的key为
			"timeout": 该接口调用超时时间，默认为1秒，单位毫秒;
			"retry": 默认值为1
			"key":可选值为AS_POLICY_KEY_SEND=1, AS_POLICY_KEY_DIGEST=0, 默认值为AS_POLICY_KEY_SEND
			"gen":可选值为AS_POLICY_GEN_IGNORE＝0, AS_POLICY_GEN_EQ＝1, AS_POLICY_GEN_GT=2, 默认值为AS_POLICY_GEN_IGNORE
			"replica":可选值为AS_POLICY_REPLICA_MASTER＝0，AS_POLICY_REPLICA_ANY＝1，默认值为AS_POLICY_REPLICA_MASTER
			"consistency_level":可选值为AS_POLICY_CONSISTENCY_LEVEL_ONE＝0，AS_POLICY_CONSISTENCY_LEVEL_ALL＝1，默认值为AS_POLICY_CONSISTENCY_LEVEL_ONE
			"commit_level": 可选值为AS_POLICY_COMMIT_LEVEL_ALL＝0, AS_POLICY_COMMIT_LEVEL_MASTER=1, 默认值为AS_POLICY_COMMIT_LEVEL_ALL
			"deserialize":可选值为1或者0，默认值为1
			详见as库policy_operate
	@return 正常返回true,参数异常返回false	
	
###16.prepend_strp_async(namespace, set_name, key_name, bin_name, value, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param value: 想要增加的值,必须为字符串
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###17.append_strp_async(namespace, set_name, key_name, bin_name, value, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param value: 想要增加的值,必须为字符串
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###18.list_append_async(namespace, set_name, key_name, bin_name, value, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param value: 支持null,long,double,string, array, object类型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###19.list_insert_async(namespace, set_name, key_name, bin_name, index, value, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:插入的位置，必须为long型
	@param value:支持null,long,double,string, array, object类型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###20.list_pop_async(namespace, set_name, key_name, bin_name, index, record_callback, policy)
	获得指定位置的值，并且在数据库中去除该值
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:索引，必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###21.list_pop_range_async(namespace, set_name, key_name, bin_name, index, count, record_callback, policy)
	从指定index连续获取数目为count的值，并且在数据库中去除该系列值
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:索引，必须为long型
	@param count:数量，必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###22.list_pop_range_from_async(namespace, set_name, key_name, bin_name, index, record_callback, policy)
	获取从指定index至末尾的值，并且在数据库中去除该系列值
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:索引，必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###23.list_remove_async(namespace, set_name, key_name, bin_name, index, record_callback, policy)
	在数据库中去除索引为index的值
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:索引，必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###24.list_remove_range_async(namespace, set_name, key_name, bin_name, index, count, record_callback, policy)
	在数据库中去除索引为index的值
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:索引，必须为long型
	@param count:数量，必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###25.list_remove_range_from_async(namespace, set_name, key_name, bin_name, index, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:索引，必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###26.list_clear_async(namespace, set_name, key_name, bin_name, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###27.list_set_async(namespace, set_name, key_name, bin_name, index, value, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name:
	@param index:必须为long型
	@param value:类型支持null,long,double,string,array,object
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###28.list_trim_async(namespace, set_name, key_name, bin_name, index, count, record_callback, policy)
	保留从index开始数量为count的值，其余从数据库删除
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:必须为long型
	@param count:必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###29.list_get_async(namespace, set_name, key_name, bin_name, index, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###30.list_get_range_async(namespace, set_name, key_name, bin_name, index, count, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:必须为long型
	@param count:必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###30.list_get_range_from_async(namespace, set_name, key_name, bin_name, index, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param index:必须为long型
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
###30.list_size_async(namespace, set_name, key_name, bin_name, record_callback, policy)
	@param namespace:命名空间
	@param set_name:表名
	@param key_name:
	@param bin_name: 
	@param record_callback:回调函数,详见参数说明
	@param policy:策略，同incr_async
	@return 正常返回true,参数异常返回false	
	
##参数说明
###1.TTL
	*   The time-to-live (expiration) of the record in seconds.
    *   There are two special values that can be set in the record TTL:
    *   (*) ZERO (defined as AS_RECORD_DEFAULT_TTL), which means that the
    *      record will adopt the default TTL value from the namespace.
    *   (*) 0xFFFFFFFF (also, -1 in a signed 32 bit int)
    *      (defined as AS_RECORD_NO_EXPIRE_TTL), which means that the record
    *      will get an internal "void_time" of zero, and thus will never expire.
    *
    *   Note that the TTL value will be employed ONLY on write/update calls.
    
###2.write_callback(err)
	写操作的回调函数
	@param err: 正常返回"AEROSPIKE_OK", 异常则返回对应的字符串
	
###3.record_callback(err, rec)
	读操作的回调函数
	@param err: 正常返回"AEROSPIKE_OK", 异常则返回对应的字符串
	@param rec: 格式为map,key为bin_name, value为bin_value
	
###4.set_name
	表名,必须为字符串
	
###5.key_name
	支持long与字符串,不能为二进制

###6.bin_name
	必须为字符串
	
##example
	详见yz-swoole/examples/youzan/KVStore.php