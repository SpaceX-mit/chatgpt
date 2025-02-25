#include "chatgpt.h"
#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "hilog/log.h"

using json = nlohmann::json;

// Add HiLog tag and domain
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, 0xD001650, "ChatGPT"
};

namespace {
// 全局变量，用于保存累计的响应内容
std::string g_result;

// 回调函数：每次接收到数据块时被调用
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    g_result.append(static_cast<char*>(contents), totalSize);
    OHOS::HiviewDFX::HiLog::Debug(LABEL, "Received data: %{public}s", 
        static_cast<char*>(contents));
    return totalSize;
}
}

namespace OHOS {
namespace Communication {
ChatGPT& ChatGPT::GetInstance() {
    static ChatGPT instance;
    HiviewDFX::HiLog::Info(LABEL, "ChatGPT instance created");
    return instance;
}

std::string ChatGPT::GenerateResponse(const std::string& input) {
    // 清空全局结果变量
    g_result.clear();
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
        HiviewDFX::HiLog::Error(LABEL, "Failed to initialize CURL");
        return "Error initializing CURL";
    }

    // 构造 JSON 请求体
    std::string jsonPayload = R"({
        "model": "deepseek-r1-1.5b",
        "prompt": ")" + input + R"(",
        "stream": true
    })";
    
    HiviewDFX::HiLog::Debug(LABEL, "Request payload: %{public}s", jsonPayload.c_str());

    // 设置 HTTP 头
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // 配置 CURL 选项
    curl_easy_setopt(curl, CURLOPT_URL, "http://10.0.91.97:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    HiviewDFX::HiLog::Info(LABEL, "Making request to Ollama API");
    CURLcode res = curl_easy_perform(curl);
    
    // 清理资源
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        HiviewDFX::HiLog::Error(LABEL, "CURL request failed: %{public}s", error.c_str());
        return "CURL error: " + error;
    }

    HiviewDFX::HiLog::Info(LABEL, "CURL request completed successfully");
    HiviewDFX::HiLog::Debug(LABEL, "Final response: %{public}s", g_result.c_str());
    
    try {
        // 处理流式响应，可能需要解析多个JSON对象
        std::string finalResponse;
        size_t pos = 0;
        while ((pos = g_result.find("}\n", pos)) != std::string::npos) {
            std::string jsonStr = g_result.substr(0, pos + 1);
            json responseJson = json::parse(jsonStr);
            if (responseJson.contains("response")) {
                finalResponse += responseJson["response"].get<std::string>();
            }
            pos += 2;
        }
        return finalResponse;
    } catch (const json::exception& e) {
        HiviewDFX::HiLog::Error(LABEL, "JSON parsing failed: %{public}s", e.what());
        return "JSON parsing error: " + std::string(e.what());
    }
}
} // namespace Communication
} // namespace OHOS
