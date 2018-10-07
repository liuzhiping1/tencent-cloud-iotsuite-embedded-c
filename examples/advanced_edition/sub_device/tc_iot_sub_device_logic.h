#ifndef TC_IOT_SUB_DEVICE_LOGIC_H
#define TC_IOT_SUB_DEVICE_LOGIC_H

#include "tc_iot_inc.h"


/* 数据模板本地存储结构定义 local data struct define */
typedef struct {
    tc_iot_shadow_bool booltest;
    tc_iot_shadow_enum enumtest;
    tc_iot_shadow_number numtest;
    char stringtest[64+1];
} TC_IOT_LOCAL_STRUCT_NAME(test);

typedef enum {
    TC_IOT_LOCAL_MEMBER_ID(test,booltest),
    TC_IOT_LOCAL_MEMBER_ID(test,enumtest),
    TC_IOT_LOCAL_MEMBER_ID(test,numtest),
    TC_IOT_LOCAL_MEMBER_ID(test,stringtest),
    TC_IOT_LOCAL_MEMBER_TOTAL(test),
} TC_IOT_LOCAL_ENUM_NAME(test);

typedef struct {
    tc_iot_shadow_bool booltest;
    tc_iot_shadow_enum enumtest;
    tc_iot_shadow_number numtest;
} TC_IOT_LOCAL_STRUCT_NAME(more);

typedef enum {
    TC_IOT_LOCAL_MEMBER_ID(more,booltest),
    TC_IOT_LOCAL_MEMBER_ID(more,enumtest),
    TC_IOT_LOCAL_MEMBER_ID(more,numtest),
    TC_IOT_LOCAL_MEMBER_TOTAL(more),
} TC_IOT_LOCAL_ENUM_NAME(more);

#define TC_IOT_GW_MAX_PRODUCT_COUNT  2
#define TC_IOT_GW_MAX_SUB_DEVICE_COUNT  5

extern TC_IOT_LOCAL_STRUCT_NAME(test) TC_IOT_GLOBAL_LOCAL_STRUCT_VAR_NAME(test)[TC_IOT_GW_MAX_SUB_DEVICE_COUNT];

tc_iot_sub_device_info g_tc_iot_sub_devices[TC_IOT_GW_MAX_SUB_DEVICE_COUNT];
tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_test[TC_IOT_LOCAL_MEMBER_TOTAL(test)];

tc_iot_sub_device_info * tc_iot_sub_device_info_find(const char * product_id,
                                                     const char * device_name);
tc_iot_sub_device_info * tc_iot_sub_device_info_set_reported_bits(const char * product_id,
                                                                  const char * device_name,
                                                                  const char * field_name);
tc_iot_sub_device_info * tc_iot_sub_device_info_set_desired_bits(const char * product_id,
                                                                 const char * device_name,
                                                                 const char * field_name);
#endif /* TC_IOT_SUB_DEVICE_LOGIC_H */
