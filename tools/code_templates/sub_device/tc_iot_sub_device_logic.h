#ifndef TC_IOT_SUB_DEVICE_LOGIC_H
#define TC_IOT_SUB_DEVICE_LOGIC_H

#include "tc_iot_inc.h"

#define TC_IOT_GW_MAX_SUB_DEVICE_COUNT  5

/* 数据模板本地存储结构定义 local data struct define */
/*${data_template.declare_subdev_local_data_struct()}*/

/* 数据模板字段 ID 宏定义*/
/*${data_template.declare_subdev_local_data_field_id()}*/

extern tc_iot_shadow_local_data_subdev01 g_tc_iot_shadow_local_data_subdev01[TC_IOT_GW_MAX_SUB_DEVICE_COUNT];
extern tc_iot_shadow_property_def g_tc_iot_shadow_property_defs_subdev01[TC_IOT_PROP_TOTAL_subdev01];

#endif /* TC_IOT_SUB_DEVICE_LOGIC_H */
