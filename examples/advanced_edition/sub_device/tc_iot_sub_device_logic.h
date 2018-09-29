#ifndef TC_IOT_SUB_DEVICE_LOGIC_H
#define TC_IOT_SUB_DEVICE_LOGIC_H

#include "tc_iot_inc.h"


/* 数据模板本地存储结构定义 local data struct define */
TC_IOT_LOCAL_STRUCT_BEGIN(test)
TC_IOT_DECLARE_LOCAL_MEMBER_BOOL(booltest)
TC_IOT_DECLARE_LOCAL_MEMBER_ENUM(enumtest)
TC_IOT_DECLARE_LOCAL_MEMBER_NUMBER(numtest)
TC_IOT_DECLARE_LOCAL_MEMBER_STRING(stringtest,64)
TC_IOT_LOCAL_STRUCT_END(test)

TC_IOT_LOCAL_MEMBER_ID_BEGIN(test)
TC_IOT_DECLARE_LOCAL_MEMBER(test,booltest)
TC_IOT_DECLARE_LOCAL_MEMBER(test,enumtest)
TC_IOT_DECLARE_LOCAL_MEMBER(test,numtest)
TC_IOT_DECLARE_LOCAL_MEMBER(test,stringtest)
TC_IOT_LOCAL_MEMBER_ID_END(test)

#define TC_IOT_MAX_DEVICE_COUNT  3

extern TC_IOT_LOCAL_STRUCT_NAME(test) TC_IOT_GLOBAL_LOCAL_STRUCT_VAR_NAME(test)[TC_IOT_MAX_DEVICE_COUNT];

tc_iot_sub_device_info g_tc_iot_sub_devices[TC_IOT_MAX_DEVICE_COUNT];
tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_test[TC_IOT_LOCAL_MEMBER_TOTAL(test)];

#endif /* TC_IOT_SUB_DEVICE_LOGIC_H */
