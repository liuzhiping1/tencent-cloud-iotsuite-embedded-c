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
} TC_IOT_LOCAL_STRUCT_NAME(subdev01);

typedef enum {
    TC_IOT_LOCAL_MEMBER_ID(subdev01,booltest),
    TC_IOT_LOCAL_MEMBER_ID(subdev01,enumtest),
    TC_IOT_LOCAL_MEMBER_ID(subdev01,numtest),
    TC_IOT_LOCAL_MEMBER_ID(subdev01,stringtest),
    TC_IOT_LOCAL_MEMBER_ID(subdev01,Temperature),
    TC_IOT_LOCAL_MEMBER_TOTAL(subdev01),
} TC_IOT_LOCAL_ENUM_NAME(subdev01);

extern TC_IOT_LOCAL_STRUCT_NAME(subdev01) TC_IOT_GLOBAL_LOCAL_STRUCT_VAR_NAME(subdev01)[TC_IOT_GW_MAX_SUB_DEVICE_COUNT];

/* typedef struct { */
/*     tc_iot_shadow_bool booltest; */
/*     tc_iot_shadow_enum enumtest; */
/*     tc_iot_shadow_number numtest; */
/* } TC_IOT_LOCAL_STRUCT_NAME(more); */

/* typedef enum { */
/*     TC_IOT_LOCAL_MEMBER_ID(more,booltest), */
/*     TC_IOT_LOCAL_MEMBER_ID(more,enumtest), */
/*     TC_IOT_LOCAL_MEMBER_ID(more,numtest), */
/*     TC_IOT_LOCAL_MEMBER_TOTAL(more), */
/* } TC_IOT_LOCAL_ENUM_NAME(more); */

/* extern TC_IOT_LOCAL_STRUCT_NAME(more) TC_IOT_GLOBAL_LOCAL_STRUCT_VAR_NAME(more)[TC_IOT_GW_MAX_SUB_DEVICE_COUNT]; */

/* #define TC_IOT_GW_MAX_PRODUCT_COUNT  2 */


/* tc_iot_sub_device_info g_tc_iot_sub_devices[TC_IOT_GW_MAX_SUB_DEVICE_COUNT]; */
tc_iot_sub_device_table g_tc_iot_sub_device_table;
tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_subdev01[TC_IOT_LOCAL_MEMBER_TOTAL(subdev01)];

#endif /* TC_IOT_SUB_DEVICE_LOGIC_H */
