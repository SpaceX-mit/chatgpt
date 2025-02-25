#include <napi/native_api.h>
#include <napi/native_node_api.h>
#include "chatgpt.h"

// 定义异步工作数据结构
struct AsyncCallbackData {
    napi_env env;
    napi_ref streamCallbackRef;
    napi_ref completionCallbackRef;
    std::string chunk;
    std::string result;
    napi_value resourceName;
};

static void ExecuteAsyncWork(napi_env env, void* data) {
    // This function runs on the worker thread
    // Empty because our work is done in GenerateResponseStream
}

static void StreamCallbackComplete(napi_env env, napi_status status, void* data) {
    auto* callbackData = static_cast<AsyncCallbackData*>(data);
    
    napi_value callback, global, argv[1];
    napi_status result;
    
    result = napi_get_reference_value(env, callbackData->streamCallbackRef, &callback);
    if (result != napi_ok) return;
    
    result = napi_get_global(env, &global);
    if (result != napi_ok) return;
    
    napi_value undefined;
    result = napi_create_string_utf8(env, callbackData->chunk.c_str(), 
        callbackData->chunk.length(), &argv[0]);
    if (result != napi_ok) return;
    
    result = napi_get_undefined(env, &undefined);
    if (result != napi_ok) return;
    
    napi_call_function(env, undefined, callback, 1, argv, nullptr);
    delete callbackData;
}

static void CompletionCallbackComplete(napi_env env, napi_status status, void* data) {
    auto* callbackData = static_cast<AsyncCallbackData*>(data);
    
    napi_value callback;
    napi_get_reference_value(env, callbackData->completionCallbackRef, &callback);
    
    napi_value global;
    napi_get_global(env, &global);
    
    napi_value argv[1];
    napi_create_string_utf8(env, callbackData->result.c_str(), 
        callbackData->result.length(), &argv[0]);
    
    napi_call_function(env, global, callback, 1, argv, nullptr);
    
    // Cleanup
    napi_delete_reference(env, callbackData->streamCallbackRef);
    napi_delete_reference(env, callbackData->completionCallbackRef);
    delete callbackData;
}

napi_value GenerateResponse(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3], resource_name;
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    // Create persistent references
    napi_ref streamCallbackRef;
    napi_ref completionCallbackRef;
    
    // Create string for resource name
    napi_create_string_utf8(env, "StreamCallback", NAPI_AUTO_LENGTH, &resource_name);
    
    napi_create_reference(env, args[1], 1, &streamCallbackRef);
    napi_create_reference(env, args[2], 1, &completionCallbackRef);

    // Get input string
    char input[1024];
    size_t inputLen;
    napi_get_value_string_utf8(env, args[0], input, sizeof(input), &inputLen);

    // Create stream callback
    auto streamCallback = [env, streamCallbackRef](const std::string& chunk) {
        auto* data = new AsyncCallbackData{
            .env = env,
            .streamCallbackRef = streamCallbackRef,
            .chunk = chunk
        };

        napi_async_work work;
        napi_value resource_name;
        napi_create_string_utf8(env, "StreamCallback", NAPI_AUTO_LENGTH, &resource_name);

        napi_create_async_work(
            env,
            nullptr,
            resource_name,
            ExecuteAsyncWork,
            StreamCallbackComplete,
            data,
            &work);

        napi_queue_async_work(env, work);
    };

    // Create completion callback
    auto completionCallback = [env, streamCallbackRef, completionCallbackRef]
        (const std::string& result) {
        auto* data = new AsyncCallbackData{
            .env = env,
            .streamCallbackRef = streamCallbackRef,
            .completionCallbackRef = completionCallbackRef,
            .result = result
        };

        napi_async_work work;
        napi_value resource_name;
        napi_create_string_utf8(env, "CompletionCallback", NAPI_AUTO_LENGTH, &resource_name);

        napi_create_async_work(
            env,
            nullptr,
            resource_name,
            ExecuteAsyncWork,
            CompletionCallbackComplete,
            data,
            &work);

        napi_queue_async_work(env, work);
    };

    // Call the native method
    OHOS::Communication::ChatGPT::GetInstance().GenerateResponseStream(
        input, streamCallback, completionCallback);

    return nullptr;
}

// Module initialization
napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        { "generateResponse", nullptr, GenerateResponse, nullptr, nullptr, nullptr, 
            napi_default, nullptr }
    };
    napi_define_properties(env, exports, 1, desc);
    return exports;
}

NAPI_MODULE(chatgpt_napi, Init)
