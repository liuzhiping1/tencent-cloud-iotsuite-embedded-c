#ifndef TC_IOT_SUB_DEVICE_H
#define TC_IOT_SUB_DEVICE_H


/*--- begin 子设备请求 method 字段取值----*/
/* 请求类 */
/**< 子设备上线
 *
 {
	"method": "device_online",
	"passthrough": {
		"sid": "c58a000e"
	},
	"timestamp": 12212121212,
	"sub_dev_state": [{
			"product": "iot-ammeter",
			"device_list": [{
					"dev_name": "ammeter_3",
					"sign": "4+idpxXyGvCPg4BcWZzCqaWFN1QQwvkrFtQT2KjHHvc="
				}
			]
		},
		{
			"product": "iot-locker",
			"device_list": [{
					"dev_name": "lock_3",
					"sign": "asdfasdfasdfasdfasdfasdfasdf"

				}
			]
		}
	]
}
 * */
#define TC_IOT_SUB_DEVICE_ONLINE           "device_online"

#define TC_IOT_SUB_DEVICE_OFFLINE          "device_offline"

#define TC_IOT_SUB_DEVICE_GROUP_UPDATE     "group_update"
#define TC_IOT_SUB_DEVICE_GROUP_GET        "group_get"
#define TC_IOT_SUB_DEVICE_GROUP_DELETE     "group_delete"

/*--- end 子设备请求 method 字段取值----*/

#define TC_IOT_GROUP_DOC_PRODUCT_DEPTH   2
#define TC_IOT_GROUP_DOC_DEVICE_DEPTH    4

typedef struct _tc_iot_sub_device_info {
    char product_id[TC_IOT_MAX_PRODUCT_ID_LEN]; /**< 设备 Product Id*/
    char device_name[TC_IOT_MAX_DEVICE_NAME_LEN];  /**< 设备 Device Name*/
    char device_secret[TC_IOT_MAX_SECRET_LEN];  /**< 设备签名秘钥*/
    char desired_bits[(TC_IOT_MAX_PROPERTY_COUNT/8)+1];
    char reported_bits[(TC_IOT_MAX_PROPERTY_COUNT/8)+1];
    int property_total;
    tc_iot_shadow_property_def * properties;
    void * p_data;
    unsigned int sequence;
} tc_iot_sub_device_info;


typedef struct _tc_io_sub_device_table{
    int total;
    int used;
    tc_iot_sub_device_info * items;
} tc_iot_sub_device_table;

typedef struct _tc_iot_sub_device_event_data {
    const char * product_id;
    const char * device_name;
    const char * name;
    const char * value;
}tc_iot_sub_device_event_data;

/**
 * @brief 子设备批量上线，调用方式样例：
 *
 tc_iot_sub_device_online(c,
 2, // 产品数，表示总共有2个产品有设备要上线。
 "iot-prod_01", // 第1个产品的 Product Id
    1, // 第1个产品下，总共有1个设备要上线
    "prodcut_001_device001","prodcut_001_device001_secret", // 第1个设备的名称和密钥
 "iot-prod_02", // 第2个产品的 Product Id
    2, // 第2个产品下，总共有2个设备要上线
    "prodcut_002_device001","prodcut_002_device001_secret", // 第1个设备的名称和密钥
    "prodcut_002_device002","prodcut_002_device002_secret", // 第2个设备的名称和密钥
 );
 *
 * @param c 网关对象
 * @param product_count 上线产品数
 * @param ... 变参格式：
 *            product_id_1, M(设备数), device_001, device_secret_001 ..., device_secret_M,
 *            product_id_2, N(设备数), device_001, device_secret_001 ..., device_secret_N
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_sub_device_online(tc_iot_shadow_client * c, int product_count, ...);


/**
 * @brief 子设备批量下线，调用方式样例：
 *
 tc_iot_sub_device_offline(c,
 2, // 产品数，表示总共有2个产品有设备要下线。
 "iot-prod_01", // 第1个产品的 Product Id
    1, // 第1个产品下，总共有1个设备要下线
    "prodcut_001_device001", // 第1个设备的名称
 "iot-prod_02", // 第2个产品的 Product Id
    2, // 第2个产品下，总共有2个设备要下线
    "prodcut_002_device001", // 第1个设备的名称
    "prodcut_002_device002", // 第2个设备的名称
 );
 *
 * @param c 网关对象
 * @param product_count 上线产品数
 * @param ... 变参格式：
 *            product_id_1, M(设备数), device_001, ..., device_M,
 *            product_id_2, N(设备数), device_001, ..., device_N
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_sub_device_offline(tc_iot_shadow_client * c, int product_count, ...);

void tc_iot_group_req_message_ack_callback(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);

void tc_iot_group_get_message_ack_callback(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);

void tc_iot_device_on_group_message_received(tc_iot_message_data* md);
int tc_iot_group_doc_parse(tc_iot_shadow_client * p_shadow_client, tc_iot_json_tokenizer * tokenizer);
int tc_iot_group_control_process(tc_iot_shadow_client * c, tc_iot_json_tokenizer * tokenizer, int product_index);

int tc_iot_sub_device_group_doc_init(tc_iot_shadow_client * c, char * buffer, int buffer_len, const char * method);
int tc_iot_sub_device_group_doc_add_product(char * buffer, int buffer_len, const char * product_id);
int tc_iot_sub_device_group_doc_add_device(char * buffer, int buffer_len, const char * device_name, unsigned int sequence);
int tc_iot_sub_device_group_doc_add_state_holder(char * buffer, int buffer_len, const char * state_holder);
int tc_iot_sub_device_group_doc_add_data(char * buffer, int buffer_len, int depth, const char * name,
                                         tc_iot_shadow_data_type_e type , const void * value);
int tc_iot_sub_device_group_doc_pub(tc_iot_shadow_client * c, char * buffer, int buffer_len,
                                     message_ack_handler callback,
                                    int timeout_ms, void * session_context);

// struct for member data begin
#define TC_IOT_LOCAL_STRUCT_NAME(name) tc_iot_shadow_local_data_ ## name
#define TC_IOT_GLOBAL_LOCAL_STRUCT_VAR_NAME(name) g_tc_iot_shadow_local_data_ ## name
// struct for member data end

// for member id begin
#define TC_IOT_LOCAL_ENUM_NAME(product) tc_iot_shadow_local_enum_ ## product
#define TC_IOT_LOCAL_MEMBER_ID(product, member) TC_IOT_PROP_ ## product ## _ ## member
#define TC_IOT_LOCAL_MEMBER_TOTAL(product) TC_IOT_PROP_ ## product ## TOTAL
#define TC_IOT_DECLARE_LOCAL_MEMBER_DEF(product,member,type) {      \
    #member, TC_IOT_PROP_ ## product ## _ ## member, type,offsetof(tc_iot_shadow_local_data_ ##product, member), \
        TC_IOT_MEMBER_SIZE(tc_iot_shadow_local_data_ ##product, member) \
}
// for member id end


int tc_iot_confirm_sub_device(tc_iot_shadow_client * c, tc_iot_sub_device_info * sub_devices, int sub_devices_count);
int tc_iot_report_sub_device(tc_iot_shadow_client * c, tc_iot_sub_device_info * sub_devices, int sub_devices_count);


#endif /* TC_IOT_SUB_DEVICE_H */
