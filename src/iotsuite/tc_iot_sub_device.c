#include "tc_iot_inc.h"

int tc_iot_calc_sub_device_online_sign(char* sign_out, int max_sign_len, const char* device_secret,
                                       const char* device_name, unsigned int nonce,
                                       const char* product_id, unsigned int timestamp) {
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];
    int ret;
    char * b64_buf = sign_out;

    IF_NULL_RETURN(sign_out, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);
    IF_EQUAL_RETURN(max_sign_len, 0, TC_IOT_INVALID_PARAMETER);

    ret = tc_iot_calc_sign(
        sha256_digest, sizeof(sha256_digest),
        device_secret,
        "deviceName=%s&nonce=%u&productId=%s&timestamp=%u",
        device_name, nonce,
        product_id, timestamp);

    ret = tc_iot_base64_encode((unsigned char *)sha256_digest, sizeof(sha256_digest), b64_buf,
                               max_sign_len);
    if (ret < max_sign_len && ret > 0) {
        b64_buf[ret] = '\0';
        tc_iot_mem_usage_log("b64_buf", max_sign_len, ret);
    } else {
        TC_IOT_LOG_ERROR("ret = %d", ret);
    }

    TC_IOT_LOG_TRACE("signature %s", b64_buf);
    return ret;
}

/**
 * @brief tc_iot_group_req_message_ack_callback shadow_update 上报消息回调
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void tc_iot_group_req_message_ack_callback(tc_iot_command_ack_status_e ack_status,
                                            tc_iot_message_data * md , void * session_context) {
    tc_iot_mqtt_message* message = NULL;

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            TC_IOT_LOG_ERROR("request timeout");
        } else {
            TC_IOT_LOG_ERROR("request return ack_status=%d", ack_status);
        }
        return;
    }

    message = md->message;
    TC_IOT_LOG_TRACE("[s->c] %s", (char*)message->payload);
}

/**
 * @brief shadow_get 上报消息回调
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void tc_iot_group_get_message_ack_callback(tc_iot_command_ack_status_e ack_status,
                                            tc_iot_message_data * md , void * session_context) {

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            TC_IOT_LOG_ERROR("request timeout");
        } else {
            TC_IOT_LOG_ERROR("request return ack_status=%d", ack_status);
        }
        return;
    }
    tc_iot_device_on_group_message_received(md);
}

int tc_iot_sub_device_group_req(tc_iot_shadow_client * c,
                                char * buffer, int buffer_len,
                                const char * method, int product_count, va_list args,
                                message_ack_handler callback, int timeout_ms, void * session_context) {
    char b64_buf[TC_IOT_BASE64_ENCODE_OUT_LEN(TC_IOT_SHA256_DIGEST_SIZE)];
    int ret;
    tc_iot_shadow_session * p_session;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;
    int i = 0;
    int j = 0;
    tc_iot_mqtt_message pubmsg;
    const char * pub_topic;
    unsigned int nonce = 0;
    unsigned int timestamp = 0;
    int device_count = 0;
    const char * product_id;
    const char * device_name;
    const char * device_secret;
    bool is_online = false;

    timestamp = tc_iot_hal_timestamp(NULL);

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);

    p_session = tc_iot_fetch_session(c);
    if (!p_session) {
        TC_IOT_LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    is_online = strcmp(method, TC_IOT_SUB_DEVICE_ONLINE) == 0;

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", method);

    tc_iot_json_writer_uint(w ,"timestamp", timestamp);
    if (is_online) {
        nonce = tc_iot_hal_random();
        tc_iot_json_writer_uint(w ,"nonce", nonce);
    }
    tc_iot_json_writer_object_begin(w ,"passthrough");
    tc_iot_json_writer_string(w ,"sid", p_session->sid);
    tc_iot_json_writer_object_end(w);

    tc_iot_json_writer_array_begin(w ,"sub_dev_state");

    for (i = 0; i < product_count; ++i) {
        product_id = va_arg(args, const char *);
        device_count = va_arg(args, int);

        tc_iot_json_writer_object_begin(w, NULL);
        tc_iot_json_writer_string(w , "product", product_id);
        tc_iot_json_writer_array_begin(w ,"device_list");
        for (j = 0; j < device_count; ++j) {
            device_name = va_arg(args, const char *);
            tc_iot_json_writer_object_begin(w, NULL);
            tc_iot_json_writer_string(w , "dev_name", device_name);
            if (is_online) {
                device_secret = va_arg(args, const char *);

                ret = tc_iot_calc_sub_device_online_sign(b64_buf, sizeof(b64_buf), device_secret,
                                                         device_name, nonce, product_id, timestamp);
                tc_iot_json_writer_string(w , "sign", b64_buf);
            }
            tc_iot_json_writer_object_end(w);
        }
        tc_iot_json_writer_array_end(w); // device_list
        tc_iot_json_writer_object_end(w);
    }

    tc_iot_json_writer_array_end(w); // sub_dev_state

    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_ERROR("ret=%d", ret);
        tc_iot_release_session(p_session);
        return ret;
    }

    tc_iot_hal_timer_init(&(p_session->timer));
    tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
    p_session->handler = callback;
    p_session->session_context = session_context;

    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    TC_IOT_LOG_TRACE("[c-s]: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    ret = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != ret) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", ret);
    }

    return w->pos;
}

int tc_iot_sub_device_online(tc_iot_shadow_client * c, int product_count, ...) {
    va_list args;
    int ret = 0;
    char buffer[2024]; // TODO calc length

    va_start(args, product_count);
    ret = tc_iot_sub_device_group_req( c,
                                       buffer, sizeof(buffer),
                                       TC_IOT_SUB_DEVICE_ONLINE, product_count, args,
                                       tc_iot_group_req_message_ack_callback,
                                       c->mqtt_client.command_timeout_ms, NULL);
    va_end(args);
    if (ret > 0) {
        ret = TC_IOT_SUCCESS;
    }
    return ret;
}

int tc_iot_sub_device_offline(tc_iot_shadow_client * c, int product_count, ...) {
    va_list args;
    int ret = 0;
    char buffer[2024]; // TODO calc length

    va_start(args, product_count);
    ret = tc_iot_sub_device_group_req( c,
                                       buffer, sizeof(buffer),
                                       TC_IOT_SUB_DEVICE_OFFLINE, product_count, args,
                                       tc_iot_group_req_message_ack_callback,
                                       c->mqtt_client.command_timeout_ms, NULL);
    va_end(args);
    if (ret > 0) {
        ret = TC_IOT_SUCCESS;
    }
    return ret;
}

int tc_iot_sub_device_group_doc_init(tc_iot_shadow_client * c, char * buffer, int buffer_len,
                                     const char * method,
                                     message_ack_handler callback,
                                     int timeout_ms, void * session_context) {
    int ret;
    tc_iot_shadow_session * p_session;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);

    p_session = tc_iot_fetch_session(c);
    if (!p_session) {
        TC_IOT_LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", method);

    tc_iot_json_writer_object_begin(w ,"passthrough");
    tc_iot_json_writer_string(w ,"sid", p_session->sid);
    tc_iot_json_writer_object_end(w);

    tc_iot_json_writer_array_begin(w ,"sub_dev");

    tc_iot_json_writer_array_end(w); // sub_dev

    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_ERROR("ret=%d", ret);
        tc_iot_release_session(p_session);
        return ret;
    }

    tc_iot_hal_timer_init(&(p_session->timer));
    tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
    p_session->handler = callback;
    p_session->session_context = session_context;

    return w->pos;
}

int tc_iot_sub_device_group_doc_add_product(char * buffer, int buffer_len, const char * product_id) {
    int ret = 0;
    int write_bytes = 0;

    ret = tc_iot_sub_device_group_doc_add_data(buffer, buffer_len, 2, NULL ,TC_IOT_SHADOW_TYPE_OBJECT, "");
    if (ret < 0) {
        TC_IOT_LOG_ERROR("product=%s, ret=%d", product_id, ret);
        return ret;
    }
    write_bytes += ret;

    ret = tc_iot_sub_device_group_doc_add_data(buffer, buffer_len, 0, "product", TC_IOT_SHADOW_TYPE_STRING,  product_id);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("product=%s, ret=%d", product_id, ret);
        return ret;
    }
    write_bytes += ret;
    ret = tc_iot_sub_device_group_doc_add_data(buffer, buffer_len , 0, "device_list",TC_IOT_SHADOW_TYPE_ARRAY, "");
    if (ret < 0) {
        TC_IOT_LOG_ERROR("product=%s, ret=%d", product_id, ret);
        return ret;
    }
    write_bytes += ret;
    return write_bytes;
}

int tc_iot_sub_device_group_doc_add_device(char * buffer, int buffer_len, const char * device_name, unsigned int sequence) {
    int ret = 0;
    int write_bytes = 0;

    ret = tc_iot_sub_device_group_doc_add_data(buffer, buffer_len, 0, NULL ,TC_IOT_SHADOW_TYPE_OBJECT, "");
    if (ret < 0) {
        TC_IOT_LOG_ERROR("device_name=%s, ret=%d",device_name, ret);
        return ret;
    }
    write_bytes += ret;

    ret = tc_iot_sub_device_group_doc_add_data(buffer, buffer_len, 0, "dev_name", TC_IOT_SHADOW_TYPE_STRING,  device_name);
    if (ret < 0) {
        TC_IOT_LOG_ERROR("device_name=%s, ret=%d",device_name, ret);
        return ret;
    }
    write_bytes += ret;

    if (sequence > 0) {
        ret = tc_iot_sub_device_group_doc_add_data(buffer, buffer_len, 0, "sequence", TC_IOT_SHADOW_TYPE_UINT,  &sequence);
        if (ret < 0) {
            TC_IOT_LOG_ERROR("device_name=%s, ret=%d",device_name, ret);
            return ret;
        }
        write_bytes += ret;
    }


    return write_bytes;
}

int tc_iot_sub_device_group_doc_add_state_holder(char * buffer, int buffer_len, const char * state_holder) {
    int ret = 0;
    int write_bytes = 0;

    ret = tc_iot_sub_device_group_doc_add_data(buffer, buffer_len, 6, state_holder ,TC_IOT_SHADOW_TYPE_OBJECT, "");
    if (ret < 0) {
        TC_IOT_LOG_ERROR("state_holder=%s, ret=%d",state_holder, ret);
        return ret;
    }
    write_bytes += ret;

    return write_bytes;
}

int tc_iot_sub_device_group_doc_add_data(char * buffer, int buffer_len, int depth, const char * name,
                                         tc_iot_shadow_data_type_e type , const void * value) {
    int ret = 0;
    int i = 0;
    int used_len = 0;
    char backward_quotes[16];
    char * pos = NULL;
    int bottom_depth = 0;
    tc_iot_json_writer writer, *w;
    w = &writer;

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    used_len = strlen(buffer);
    if (used_len > buffer_len) {
        TC_IOT_LOG_ERROR("used_len(%d) > buffer_len(%d)", used_len, buffer_len);
        return TC_IOT_INVALID_PARAMETER;
    }

    if (depth <= 0) {
        pos = buffer+used_len-1;
        for (i = 0; (i < sizeof(backward_quotes)) && (pos >= buffer); i++, pos--) {
            if (*pos != '}' && *pos != ']') {
                break;
            }
            bottom_depth++;
        }

        depth += bottom_depth;
        /* TC_IOT_LOG_TRACE("auto depth=%d, used_len=%d", depth, used_len); */
    }

    // check data;
    if (used_len < (depth*2)) {
        TC_IOT_LOG_ERROR("buffer invalid, used_len=%d, depth=%d: %s", used_len, depth, buffer);
        return TC_IOT_INVALID_PARAMETER;
    }

    if (depth >= sizeof(backward_quotes)) {
        TC_IOT_LOG_ERROR("depth too much, backwards not enough.: %s", buffer);
        return TC_IOT_INVALID_PARAMETER;
    }

    strncpy(backward_quotes, &buffer[used_len-depth], sizeof(backward_quotes));
    /* TC_IOT_LOG_TRACE("quotes=%s", backward_quotes); */

    for (i = 0; i < depth; ++i) {
        if (backward_quotes[i] != ']' && backward_quotes[i] != '}') {
            TC_IOT_LOG_ERROR("buffer end quote invalid: %s", buffer);
            return TC_IOT_INVALID_PARAMETER;
        }
    }

    // {..., "sub_dev":[... {"device_list":[ ... ]}]}
    // backward to here                          ^
    pos = &buffer[used_len-depth];

    tc_iot_json_writer_load(w, pos, buffer_len-used_len+depth);

    if (!value) {
        tc_iot_json_writer_null(w, name);
    } else {
        switch(type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            ret = tc_iot_json_writer_int(w, name, *(tc_iot_shadow_bool *)value);
            break;
        case TC_IOT_SHADOW_TYPE_NUMBER:
            ret = tc_iot_json_writer_decimal(w, name, *(tc_iot_shadow_number *)value);
            break;
        case TC_IOT_SHADOW_TYPE_ENUM:
            ret = tc_iot_json_writer_int(w, name, *(tc_iot_shadow_enum *)value);
            break;
        case TC_IOT_SHADOW_TYPE_INT:
            ret = tc_iot_json_writer_int(w, name, *(tc_iot_shadow_int *)value);
            break;
        case TC_IOT_SHADOW_TYPE_UINT:
            ret = tc_iot_json_writer_uint(w, name, *(tc_iot_shadow_uint *)value);
            break;
        case TC_IOT_SHADOW_TYPE_STRING:
            ret = tc_iot_json_writer_string(w, name, (tc_iot_shadow_string)value);
            break;
        case TC_IOT_SHADOW_TYPE_ARRAY:
            ret = tc_iot_json_writer_raw_data(w, name, "[]");
            break;
        case TC_IOT_SHADOW_TYPE_OBJECT:
            ret = tc_iot_json_writer_raw_data(w, name, "{}");
            break;
        default:
            TC_IOT_LOG_ERROR("type=%d invalid", type);
        }
    }

    ret = tc_iot_json_writer_raw_end_quote(w, backward_quotes);

    if (ret <= 0) {
        TC_IOT_LOG_ERROR("buffer overflow");
        return TC_IOT_BUFFER_OVERFLOW;
    }

    return w->pos-depth;
}


/**
 * @brief 数据回调，处理 group_get 获取最新状态，或
 * 者影子服务推送的最新控制指令数据。
 *
 * @param md 影子数据消息
 */
void tc_iot_device_on_group_message_received(tc_iot_message_data* md) {
    tc_iot_json_tokenizer t, *tokenizer;
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[TC_IOT_MAX_FIELD_LEN];
    int field_index = 0;
    int ret = 0;
    tc_iot_mqtt_message* message = md->message;
    tc_iot_shadow_client * c = TC_IOT_CONTAINER_OF(md->mqtt_client, tc_iot_shadow_client, mqtt_client);

    tokenizer = &t;

    TC_IOT_LOG_TRACE("[s->c] %s", (char*)message->payload);
    if (md->error_code != TC_IOT_SUCCESS) {
        ret = tc_iot_shadow_event_notify(c, TC_IOT_MQTT_EVENT_ERROR_NOTIFY, md, NULL);
        return;
    }

    /* 有效性检查 */
    ret = tc_iot_json_tokenizer_load( tokenizer, message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        return ;
    }

    tc_iot_mem_usage_log("json_token[TC_IOT_MAX_JSON_TOKEN_COUNT]", sizeof(json_token), sizeof(json_token[0])*ret);

    field_index = tc_iot_json_tokenizer_find_child(tokenizer, 0, "method", field_buf, sizeof(field_buf));
    if (field_index <= 0 ) {
        TC_IOT_LOG_ERROR("field method not found in JSON: %s", (char*)message->payload);
        return ;
    }

    if (strncmp("group_control", field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("Control data receved.");
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPLY, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("Reply pack recevied.");
    }

    tc_iot_group_doc_parse(c, tokenizer);
}

int tc_iot_group_doc_parse(tc_iot_shadow_client * c, tc_iot_json_tokenizer * tokenizer) {

    const char * subdev_start = NULL;
    int subdev_len = 0;
    jsmntok_t * subdev_node = NULL;
    int subdev_index = 0;
    int product_index = 0;

    int i = 0;

    /* 检查 desired 字段是否存在 */
    subdev_index = tc_iot_json_tokenizer_find_child(tokenizer, 0, "sub_dev", NULL, 0);
    if (subdev_index <= 0 ) {
        TC_IOT_LOG_ERROR("subdev not found");
    } else {
        subdev_node = tc_iot_json_tokenizer_get_token(tokenizer,subdev_index);
        subdev_start = tc_iot_json_tokenizer_get_str_start(tokenizer,subdev_index);
        subdev_len = tc_iot_json_tokenizer_get_str_len(tokenizer,subdev_index);
        TC_IOT_LOG_TRACE("sub_dev found, size=%d,data:%s", subdev_node->size, tc_iot_log_summary_string(subdev_start, subdev_len));
        for (i = 0; i < subdev_node->size; i++) {
            product_index = tc_iot_json_tokenizer_nth_child(tokenizer, subdev_index, i);
            if (product_index <= 0) {
                TC_IOT_LOG_ERROR("find %dth child failed, product_index=%d", i, product_index);
                return TC_IOT_FAILURE;
            }
            tc_iot_group_control_process(c, tokenizer, product_index);
        }
    }

    return TC_IOT_SUCCESS;
}

int tc_iot_group_control_process(tc_iot_shadow_client * c, tc_iot_json_tokenizer * tokenizer, int product_index) {
    char buffer[1024]; /* TODO: MACRO */
    int buffer_len = sizeof(buffer);
    int ret = 0;
    int j = 0;
    int k = 0;
    const char * product_start = NULL;
    int product_len = 0;
    jsmntok_t * product_node = NULL;

    const char * device_list_start = NULL;
    int device_list_len = 0;
    jsmntok_t * device_list_node = NULL;
    int device_list_index = 0;

    const char * device_start = NULL;
    int device_len = 0;
    jsmntok_t * device_node = NULL;
    int device_index = 0;

    const char * desired_start = NULL;
    int desired_len = 0;
    jsmntok_t * desired_node = NULL;
    int desired_index = 0;

    int data_point_index = 0;

    char product_id[TC_IOT_MAX_PRODUCT_ID_LEN]; /**< 设备 Product Id*/
    char device_name[TC_IOT_MAX_DEVICE_NAME_LEN];  /**< 设备 Device Name*/
    char data_point_name[65]; // /* TODO: MACRO */
    tc_iot_sub_device_event_data event_data;


    event_data.product_id = product_id;
    event_data.device_name = device_name;
    event_data.name = NULL;
    event_data.value = NULL;

    product_node = tc_iot_json_tokenizer_get_token(tokenizer,product_index);
    product_start = tc_iot_json_tokenizer_get_str_start(tokenizer,product_index);
    product_len = tc_iot_json_tokenizer_get_str_len(tokenizer,product_index);
    TC_IOT_LOG_TRACE("product found, size=%d,data:%s", product_node->size, tc_iot_log_summary_string(product_start, product_len));

    ret  = tc_iot_json_tokenizer_find_child(tokenizer, product_index, "product", product_id, sizeof(product_id));
    if (ret <= 0) {
        return TC_IOT_FAILURE;
    }
    TC_IOT_LOG_TRACE("product_id=%s", product_id);

    device_list_index  = tc_iot_json_tokenizer_find_child(tokenizer, product_index, "device_list", NULL, 0);
    if (device_list_index <= product_index) {
        return TC_IOT_FAILURE;
    }

    device_list_node = tc_iot_json_tokenizer_get_token(tokenizer,device_list_index);
    device_list_start = tc_iot_json_tokenizer_get_str_start(tokenizer,device_list_index);
    device_list_len = tc_iot_json_tokenizer_get_str_len(tokenizer,device_list_index);
    TC_IOT_LOG_TRACE("device_list found, size=%d,data:%s", device_list_node->size, tc_iot_log_summary_string(device_list_start, device_list_len));

    for (j = 0; j < device_list_node->size; j++) {

        device_index = tc_iot_json_tokenizer_nth_child(tokenizer, device_list_index, j);
        if (device_index <= 0) {
            TC_IOT_LOG_ERROR("find %dth child failed, device_index=%d", j, device_index);
            return TC_IOT_FAILURE;
        }
        device_node = tc_iot_json_tokenizer_get_token(tokenizer,device_index);
        device_start = tc_iot_json_tokenizer_get_str_start(tokenizer,device_index);
        device_len = tc_iot_json_tokenizer_get_str_len(tokenizer,device_index);
        TC_IOT_LOG_TRACE("device found, size=%d, data:%s", device_node->size, tc_iot_log_summary_string(device_start, device_len));

        ret  = tc_iot_json_tokenizer_find_child(tokenizer, device_index, "dev_name", device_name, sizeof(device_name));
        if (ret <= 0) {
            TC_IOT_LOG_ERROR("find child [dev_name] failed, device_index=%d", device_index);
            return TC_IOT_FAILURE;
        }
        TC_IOT_LOG_TRACE("device_name=%s", device_name);

        ret  = tc_iot_json_tokenizer_find_child(tokenizer, device_index, "sequence", buffer, buffer_len);
        if (ret <= 0) {
            TC_IOT_LOG_ERROR("find child [sequence] failed, device_index=%d", device_index);
            return TC_IOT_FAILURE;
        }
        event_data.name = "sequence";
        event_data.value = buffer;
        ret =  tc_iot_shadow_event_notify(c, TC_IOT_SUB_DEV_SEQUENCE_RECEIVED, &event_data, NULL);

        desired_index  = tc_iot_json_tokenizer_find_child(tokenizer,
                                                          tc_iot_json_tokenizer_find_child(tokenizer, device_index, "state", NULL, 0),
                                                          "desired", NULL, 0);
        if (desired_index <= 0) {
            TC_IOT_LOG_ERROR("find child [state.desired] failed, device_index=%d", device_index);
            return TC_IOT_FAILURE;
        }

        desired_node = tc_iot_json_tokenizer_get_token(tokenizer,desired_index);
        desired_start = tc_iot_json_tokenizer_get_str_start(tokenizer,desired_index);
        desired_len = tc_iot_json_tokenizer_get_str_len(tokenizer,desired_index);
        TC_IOT_LOG_TRACE("desired found, size=%d,data:%s", desired_node->size, tc_iot_log_summary_string(desired_start, desired_len));

        event_data.name = data_point_name;
        event_data.value = buffer;
        for (k = 0; k < desired_node->size; k++) {

            data_point_index = tc_iot_json_tokenizer_nth_child_value(data_point_name, sizeof(data_point_name),
                                                                     tokenizer, desired_index, k);
            if (data_point_index <= 0) {
                TC_IOT_LOG_ERROR("find %dth child failed, data_point_index=%d", k, data_point_index);
                return TC_IOT_FAILURE;
            }

            ret = tc_iot_json_tokenizer_nth_child_value(buffer, buffer_len,
                                                        tokenizer, data_point_index, 0);
            if (ret <= 0) {
                TC_IOT_LOG_ERROR("find 0th child failed, ret=%d", ret);
                return TC_IOT_FAILURE;
            }

            ret =  tc_iot_shadow_event_notify(c, TC_IOT_SUB_DEV_SERVER_CONTROL, &event_data, NULL);
        }
        event_data.name = NULL;
        event_data.value = NULL;
        ret =  tc_iot_shadow_event_notify(c, TC_IOT_SUB_DEV_SERVER_CONTROL_FINISHED, &event_data, NULL);
    }

    return TC_IOT_SUCCESS;
}
