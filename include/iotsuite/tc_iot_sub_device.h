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
#define TC_IOT_SUB_DEVICE_ONLINE       "device_online"

#define TC_IOT_SUB_DEVICE_OFFLINE       "device_offline"

/*--- end 子设备请求 method 字段取值----*/


typedef struct _tc_iot_sub_device_info {
    char product_id[TC_IOT_MAX_PRODUCT_ID_LEN]; /**< 设备 Product Id*/
    char device_name[TC_IOT_MAX_DEVICE_NAME_LEN];  /**< 设备 Device Name*/
    char device_secret[TC_IOT_MAX_SECRET_LEN];  /**< 设备签名秘钥*/
} tc_iot_sub_device_info;


typedef tc_iot_shadow_client tc_iot_gateway_dev;
typedef tc_iot_shadow_config tc_iot_gateway_config;

#define tc_iot_gateway_construct(gateway, config) tc_iot_shadow_construct(gateway,config)
#define tc_iot_gateway_destroy(gateway) tc_iot_shadow_destroy(gateway)
#define tc_iot_gateway_yield(gateway, timeout_ms) tc_iot_shadow_yield(gateway, timeout_ms)


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
int tc_iot_sub_device_online(tc_iot_gateway_dev * c, int product_count, ...);


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
int tc_iot_sub_device_offline(tc_iot_gateway_dev * c, int product_count, ...);

#endif /* TC_IOT_SUB_DEVICE_H */
