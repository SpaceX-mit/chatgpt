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
// Callback function to handle CURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    OHOS::HiviewDFX::HiLog::Debug(LABEL, "WriteCallback received %{public}zu bytes", totalSize);
    userp->append((char*)contents, totalSize);
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
    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL* curl = curl_easy_init();
    if (!curl) {
        HiviewDFX::HiLog::Error(LABEL, "Failed to initialize CURL");
        return "Error initializing CURL";
    }
    HiviewDFX::HiLog::Info(LABEL, "CURL initialized successfully");

    // Prepare the request payload
    json requestJson;
    requestJson["model"] = "deepseek-r1-1.5b";
    requestJson["prompt"] = input;
    requestJson["stream"] = true;  // 添加流式传输支持
    std::string jsonString = requestJson.dump();
    HiviewDFX::HiLog::Debug(LABEL, "Request payload prepared: %{public}s", jsonString.c_str());

    // Set up CURL request
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string response;

    //curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_URL, "http://10.0.91.97:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // 添加超时设置
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    HiviewDFX::HiLog::Info(LABEL, "Making request to Ollama API");
    CURLcode res = curl_easy_perform(curl);
    
    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();  // 添加全局清理

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        HiviewDFX::HiLog::Error(LABEL, "CURL request failed: %{public}s", error.c_str());
        return "CURL error: " + error;
    }
    HiviewDFX::HiLog::Info(LABEL, "CURL request completed successfully");

    try {
        json responseJson = json::parse(response);
        std::string result = responseJson["response"].get<std::string>();
        HiviewDFX::HiLog::Debug(LABEL, "Response parsed successfully: %{public}s", result.c_str());
        return result;
    } catch (const json::exception& e) {
        HiviewDFX::HiLog::Error(LABEL, "JSON parsing failed: %{public}s", e.what());
        return "JSON parsing error: " + std::string(e.what());
    }
}
} // namespace Communication
} // namespace OHOS
