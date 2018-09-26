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
 * @brief _tc_iot_group_req_message_ack_callback shadow_update 上报消息回调
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void _tc_iot_group_req_message_ack_callback(tc_iot_command_ack_status_e ack_status,
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

int tc_iot_sub_device_group_req(tc_iot_gateway_dev * c,
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

        tc_iot_json_writer_object_begin(w, "{");
        tc_iot_json_writer_string(w , "product", product_id);
        tc_iot_json_writer_array_begin(w ,"device_list");
        for (j = 0; j < device_count; ++j) {
            device_name = va_arg(args, const char *);
            tc_iot_json_writer_object_begin(w, "{");
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

int tc_iot_sub_device_online(tc_iot_gateway_dev * c, int product_count, ...) {
    va_list args;
    int ret = 0;
    char buffer[2024]; // TODO calc length

    va_start(args, product_count);
    ret = tc_iot_sub_device_group_req( c,
                                       buffer, sizeof(buffer),
                                       TC_IOT_SUB_DEVICE_ONLINE, product_count, args,
                                       _tc_iot_group_req_message_ack_callback,
                                       c->mqtt_client.command_timeout_ms, NULL);
    va_end(args);
    if (ret > 0) {
        ret = TC_IOT_SUCCESS;
    }
    return ret;
}

int tc_iot_sub_device_offline(tc_iot_gateway_dev * c, int product_count, ...) {
    va_list args;
    int ret = 0;
    char buffer[2024]; // TODO calc length

    va_start(args, product_count);
    ret = tc_iot_sub_device_group_req( c,
                                       buffer, sizeof(buffer),
                                       TC_IOT_SUB_DEVICE_OFFLINE, product_count, args,
                                       _tc_iot_group_req_message_ack_callback,
                                       c->mqtt_client.command_timeout_ms, NULL);
    va_end(args);
    if (ret > 0) {
        ret = TC_IOT_SUCCESS;
    }
    return ret;
}

int tc_iot_sub_device_group_doc_init(tc_iot_gateway_dev * c, char * buffer, int buffer_len,
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
    int used_len = 0;
    int ret = 0;
    int buffer_left = 0;
    const char * backward_quotes = "]}";
    int backward = strlen(backward_quotes);

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    used_len = strlen(buffer);

    // check data;
    if (used_len < (backward*2) || buffer[0] != '{' || strcmp(buffer+used_len-backward, backward_quotes) != 0) {
        TC_IOT_LOG_ERROR("buffer not initialized correctly: %s", buffer);
        return TC_IOT_INVALID_PARAMETER;
    }

    // {..., "sub_dev":[... ]}
    // backward to here     ^
    used_len -= backward;
    buffer_left = buffer_len - used_len;
    if (buffer[used_len-1] != '{'  && buffer[used_len-1] != '[') {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,",{\"product\":\"%s\",\"device_list\":[]}%s", product_id, backward_quotes);
    } else {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,"{\"product\":\"%s\",\"device_list\":[]}%s", product_id, backward_quotes);
    }
    if (ret < buffer_left) {
        buffer[used_len+ret] = '\0';
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    return ret - backward;
}

int tc_iot_sub_device_group_doc_add_device(char * buffer, int buffer_len, const char * device_name) {
    int ret = 0;
    int used_len = 0;
    int buffer_left = 0;
    const char * backward_quotes = "]}]}";
    int backward = strlen(backward_quotes);

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    used_len = strlen(buffer);

    // check data;
    if (used_len < (backward*2) || buffer[0] != '{' || strcmp(buffer+used_len-backward, backward_quotes) != 0) {
        TC_IOT_LOG_ERROR("buffer not initialized correctly: %s", buffer);
        return TC_IOT_INVALID_PARAMETER;
    }

    // {..., "sub_dev":[... {"device_list":[ ... ]}]}
    // backward to here                          ^
    used_len -= backward;
    buffer_left = buffer_len - used_len;
    if (buffer[used_len-1] != '{'  && buffer[used_len-1] != '[') {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                  ",{\"dev_name\":\"%s\",\"state\":{}}%s", device_name,backward_quotes);
    } else {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                  "{\"dev_name\":\"%s\",\"state\":{}}%s", device_name,backward_quotes);
    }
    if (ret < buffer_left) {
        buffer[used_len+ret] = '\0';
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    return ret - 2;
}

int tc_iot_sub_device_group_doc_add_state_holder(char * buffer, int buffer_len, const char * state_holder) {
    int ret = 0;
    int used_len = 0;
    int buffer_left = 0;
    const char * backward_quotes = "}}]}]}";
    int backward = strlen(backward_quotes);

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    used_len = strlen(buffer);

    // check data;
    if (used_len < (backward*2) || buffer[0] != '{' || strcmp(buffer+used_len-backward, backward_quotes) != 0) {
        TC_IOT_LOG_ERROR("buffer not initialized correctly: %s", buffer);
        return TC_IOT_INVALID_PARAMETER;
    }

    // {..., "sub_dev":[... {"device_list":[ ... ]}]}
    // backward to here                          ^
    used_len -= backward;
    buffer_left = buffer_len - used_len;
    if (buffer[used_len-1] != '{'  && buffer[used_len-1] != '[') {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                  ",\"%s\":{}%s", state_holder, backward_quotes);
    } else {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                  "\"%s\":{}%s", state_holder, backward_quotes);
    }
    if (ret < buffer_left) {
        buffer[used_len+ret] = '\0';
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    return ret - 2;
}

int tc_iot_sub_device_group_doc_add_data(char * buffer, int buffer_len, const char * name,
                                         tc_iot_shadow_data_type_e type , const void * value) {
    int ret = 0;
    int used_len = 0;
    int buffer_left = 0;
    const char * backward_quotes = "}}}]}]}";
    int backward = strlen(backward_quotes);

    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(name, TC_IOT_NULL_POINTER);
    used_len = strlen(buffer);

    // check data;
    if (used_len < (backward*2) || buffer[0] != '{' || strcmp(buffer+used_len-backward, backward_quotes) != 0) {
        TC_IOT_LOG_ERROR("buffer not initialized correctly: %s", buffer);
        return TC_IOT_INVALID_PARAMETER;
    }

    // {..., "sub_dev":[... {"device_list":[ ... ]}]}
    // backward to here                          ^
    used_len -= backward;
    buffer_left = buffer_len - used_len;
    if (buffer[used_len-1] != '{'  && buffer[used_len-1] != '[') {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left, ",");
        used_len++;
        buffer_left--;
    }

    if (!value) {
        ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left, "\"%s\":null%s", name, backward_quotes);
    } else {
        switch(type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                      "\"%s\":%d%s", name, *(tc_iot_shadow_bool *)value,  backward_quotes);
            break;
        case TC_IOT_SHADOW_TYPE_NUMBER:
            ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                      "\"%s\":%f%s", name, *(tc_iot_shadow_number *)value,  backward_quotes);
            break;
        case TC_IOT_SHADOW_TYPE_ENUM:
            ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                      "\"%s\":%d%s", name, *(tc_iot_shadow_enum *)value,  backward_quotes);
            break;
        case TC_IOT_SHADOW_TYPE_INT:
            ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                      "\"%s\":%d%s", name, *(tc_iot_shadow_int *)value,  backward_quotes);
            break;
        case TC_IOT_SHADOW_TYPE_STRING:
            ret = tc_iot_hal_snprintf(buffer+used_len, buffer_left,
                                      "\"%s\":\"%s\"%s", name, (tc_iot_shadow_string)value,  backward_quotes);
            break;
        default:
            TC_IOT_LOG_ERROR("type=%d invalid", type);
        }
    }

    if (ret < buffer_left) {
        buffer[used_len+ret] = '\0';
    } else {
        return TC_IOT_BUFFER_OVERFLOW;
    }
    return ret - 2;
}
