#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_sub_device_logic.h"
#include "tc_iot_export.h"

tc_iot_shadow_local_data_subdev01 g_tc_iot_shadow_local_data_subdev01[TC_IOT_GW_MAX_SUB_DEVICE_COUNT] = {
    {0,1,2,""},
    {0,1,2,""},
    {0,1,2,""},
};

tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_subdev01[] = {
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(subdev01,booltest,    TC_IOT_SHADOW_TYPE_BOOL),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(subdev01,enumtest,    TC_IOT_SHADOW_TYPE_ENUM),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(subdev01,numtest,     TC_IOT_SHADOW_TYPE_NUMBER),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(subdev01,stringtest,  TC_IOT_SHADOW_TYPE_STRING),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(subdev01,Temperature,  TC_IOT_SHADOW_TYPE_NUMBER),
};

tc_iot_sub_device_info g_tc_iot_sub_devices[TC_IOT_GW_MAX_SUB_DEVICE_COUNT];
tc_iot_sub_device_table g_tc_iot_sub_device_table = {
    TC_IOT_GW_MAX_SUB_DEVICE_COUNT,
    0,
    &g_tc_iot_sub_devices[0],
};

