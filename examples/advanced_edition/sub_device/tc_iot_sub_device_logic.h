#ifndef TC_IOT_SUB_DEVICE_LOGIC_H
#define TC_IOT_SUB_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

#define TC_IOT_GW_MAX_SUB_DEVICE_COUNT  5

/* 数据模板本地存储结构定义 local data struct define */
typedef struct {
    tc_iot_shadow_bool booltest;
    tc_iot_shadow_enum enumtest;
    tc_iot_shadow_number numtest;
    char stringtest[64+1];
    tc_iot_shadow_number Temperature;
} tc_iot_shadow_local_data_subdev01;

typedef enum {
    TC_IOT_PROP_subdev01_booltest,
    TC_IOT_PROP_subdev01_enumtest,
    TC_IOT_PROP_subdev01_numtest,
    TC_IOT_PROP_subdev01_stringtest,
    TC_IOT_PROP_subdev01_Temperature,
    TC_IOT_PROP_TOTAL_subdev01,
} tc_iot_shadow_local_enum_subdev01;

extern tc_iot_shadow_local_data_subdev01 g_tc_iot_shadow_local_data_subdev01[TC_IOT_GW_MAX_SUB_DEVICE_COUNT];

tc_iot_sub_device_table g_tc_iot_sub_device_table;
tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_subdev01[TC_IOT_PROP_TOTAL_subdev01];

#endif /* TC_IOT_SUB_DEVICE_LOGIC_H */
