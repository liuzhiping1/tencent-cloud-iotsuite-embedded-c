#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_sub_device_logic.h"
#include "tc_iot_export.h"

tc_iot_shadow_local_data g_tc_iot_device_local_data_test[] = {
    {0,1,2,""},
    {0,1,2,""},
    {0,1,2,""},
};

tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_test[] = {
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(test,booltest,    TC_IOT_SHADOW_TYPE_BOOL),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(test,enumtest,    TC_IOT_SHADOW_TYPE_ENUM),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(test,numtest,     TC_IOT_SHADOW_TYPE_NUMBER),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(test,stringtest,  TC_IOT_SHADOW_TYPE_STRING),
};

tc_iot_shadow_local_data g_tc_iot_device_local_data_more[] = {
    {0,1,2,""},
    {0,1,2,""},
};

tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_more[] = {
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(more,booltest,    TC_IOT_SHADOW_TYPE_BOOL),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(more,enumtest,    TC_IOT_SHADOW_TYPE_ENUM),
    TC_IOT_DECLARE_LOCAL_MEMBER_DEF(more,numtest,     TC_IOT_SHADOW_TYPE_NUMBER),
};

tc_iot_sub_device_info g_tc_iot_sub_devices[] = {
    {"iot-nsg5vbok","hxb_ammeter_1","device_sec_0test", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_test),&g_tc_iot_shadow_property_defs_test[0], &g_tc_iot_device_local_data_test[0]},
    {"iot-nsg5vbok","hxb_ammeter_2","device_sec_002", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_test),&g_tc_iot_shadow_property_defs_test[0], &g_tc_iot_device_local_data_test[1]},
    {"iot-nsg5vbok","hxb_ammeter_3","device_sec_003", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_test),&g_tc_iot_shadow_property_defs_test[0], &g_tc_iot_device_local_data_test[2]},
    {"iot-nsg5vbok","hxb_ammeter_4","device_sec_004", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_more),&g_tc_iot_shadow_property_defs_more[0], &g_tc_iot_device_local_data_more[0]},
    {"iot-nsg5vbok","hxb_ammeter_5","device_sec_005", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_more),&g_tc_iot_shadow_property_defs_more[0], &g_tc_iot_device_local_data_more[1]},
};
