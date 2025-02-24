#include "napi/native_api.h"
#include "chatgpt.h"

// 定义 JS 方法名
static const std::string JS_METHOD_GENERATE_RESPONSE = "generateResponse";

// 封装 GenerateResponse 方法
napi_value GenerateResponse(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    // 从 JS 获取输入字符串
    char input[256];
    size_t strLen;
    napi_get_value_string_utf8(env, args[0], input, sizeof(input), &strLen);

    // 调用 C++ 方法
    std::string response = OHOS::Communication::ChatGPT::GetInstance().GenerateResponse(input);

    // 返回结果到 JS
    napi_value result;
    napi_create_string_utf8(env, response.c_str(), response.size(), &result);
    return result;
}

// 模块初始化函数
napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        { JS_METHOD_GENERATE_RESPONSE.c_str(), 0, GenerateResponse, 0, 0, 0, napi_default, 0 }
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

// 注册模块
NAPI_MODULE(chatgpt, Init)
