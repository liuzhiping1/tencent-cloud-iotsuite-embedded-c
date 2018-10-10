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

tc_iot_sub_device_info g_tc_iot_sub_devices[] = 
{
    {"iot-nsg5vbok","hxb_ammeter_1","device_sec_0test", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_test),&g_tc_iot_shadow_property_defs_test[0], &g_tc_iot_device_local_data_test[0], 0},
    {"iot-nsg5vbok","hxb_ammeter_2","device_sec_002", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_test),&g_tc_iot_shadow_property_defs_test[0], &g_tc_iot_device_local_data_test[1], 0},
    {"iot-nsg5vbok","hxb_ammeter_3","device_sec_003", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_test),&g_tc_iot_shadow_property_defs_test[0], &g_tc_iot_device_local_data_test[2], 0},
    {"iot-nsg5vbok","hxb_ammeter_4","device_sec_004", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_more),&g_tc_iot_shadow_property_defs_more[0], &g_tc_iot_device_local_data_more[0], 0},
    {"iot-nsg5vbok","hxb_ammeter_5","device_sec_005", {0}, {0}, TC_IOT_ARRAY_LENGTH(g_tc_iot_shadow_property_defs_more),&g_tc_iot_shadow_property_defs_more[0], &g_tc_iot_device_local_data_more[1], 0},
};
tc_iot_sub_device_table g_tc_iot_sub_device_table = {
    TC_IOT_GW_MAX_PRODUCT_COUNT,
    0,
    &g_tc_iot_sub_devices[0],
};

tc_iot_sub_device_info * tc_iot_sub_device_info_register(tc_iot_sub_device_table * t,
                                                         const char * product_id,
                                                         const char * device_name,
                                                         const char * secret,
                                                         int property_total,
                                                         tc_iot_shadow_property_def * properties,
                                                         void * p_data)
{
    int i = 0;

    if (!t) {
        TC_IOT_LOG_ERROR("t is null");
        return NULL;
    }

    if (t->total <= t->used) {
        TC_IOT_LOG_ERROR("sub device table overflow, total=%d, used=%d",  t->total, t->used);
        return NULL;
    }
    return NULL;
}

tc_iot_sub_device_info * tc_iot_sub_device_info_find( const char * product_id, const char * device_name)
{
    int i = 0;
    tc_iot_sub_device_info * current = NULL;
    tc_iot_sub_device_table * t = &g_tc_iot_sub_device_table;

    for (i = 0; i < t->used; ++i) {
        current = &t->items[i];
        if (strcmp(product_id, current->product_id) == 0
            && strcmp(device_name, current->device_name) == 0) {
            return current;
        }
    }

    TC_IOT_LOG_ERROR("device not found: product_id=%s,device_name=%s",
                     product_id, device_name);
    return NULL;
}

tc_iot_sub_device_info * tc_iot_sub_device_info_set_reported_bits(const char * product_id,
                                                                  const char * device_name,
                                                                  const char * field_name)
{
    int i = 0;
    int property_total;
    tc_iot_shadow_property_def * properties;
    tc_iot_sub_device_info * current = tc_iot_sub_device_info_find(product_id, device_name);

    if (!current) {
        return NULL;
    }

    property_total = current->property_total;
    properties = current->properties;
    for (i = 0; i < property_total; ++i, ++properties) {
        if (strcmp(properties->name, field_name) == 0) {
            TC_IOT_BIT_SET(current->reported_bits, properties->id);
            return current;
        }
    }

    TC_IOT_LOG_ERROR("device property not found: product_id=%s,device_name=%s, field_name=%s",
                     product_id, device_name, field_name);
    return NULL;
}

tc_iot_sub_device_info * tc_iot_sub_device_info_set_desired_bits(const char * product_id,
                                                                 const char * device_name,
                                                                 const char * field_name)
{
    int i = 0;
    int property_total;
    tc_iot_shadow_property_def * properties;
    tc_iot_sub_device_info * current = tc_iot_sub_device_info_find(product_id, device_name);

    if (!current) {
        return NULL;
    }

    property_total = current->property_total;
    properties = current->properties;
    for (i = 0; i < property_total; ++i, ++properties) {
        if (strcmp(properties->name, field_name) == 0) {
            TC_IOT_BIT_SET(current->desired_bits, properties->id);
            return current;
        }
    }

    TC_IOT_LOG_ERROR("device property not found: product_id=%s,device_name=%s, field_name=%s",
                     product_id, device_name, field_name);
    return NULL;
}
