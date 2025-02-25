#include "chatgpt.h"
#include <iostream>
#include <thread>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "hilog/log.h"

using json = nlohmann::json;

// Add HiLog tag and domain
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, 0xD001650, "ChatGPT"
};

namespace {
// Callback context structure
struct CallbackContext {
    std::string accumulatedResponse;
    OHOS::Communication::ChatGPT::StreamCallback streamCallback;
    OHOS::Communication::ChatGPT::CompletionCallback completionCallback;
    size_t lastProcessedPos = 0;
};

// Process single JSON response
void ProcessJsonResponse(const std::string& jsonStr, CallbackContext* context) {
    try {
        json response = json::parse(jsonStr);
        if (response.contains("response")) {
            std::string content = response["response"].get<std::string>();
            if (!content.empty()) {
                context->streamCallback(content);
            }
        }
    } catch (const json::exception& e) {
        OHOS::HiviewDFX::HiLog::Error(LABEL, "JSON parsing failed: %{public}s", e.what());
    }
}

// CURL write callback function
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    auto* context = static_cast<CallbackContext*>(userp);
    
    // Append new data
    context->accumulatedResponse.append(static_cast<char*>(contents), totalSize);
    
    // Find complete JSON objects
    size_t pos = context->lastProcessedPos;
    while ((pos = context->accumulatedResponse.find("}\n", pos)) != std::string::npos) {
        std::string jsonStr = context->accumulatedResponse.substr(
            context->lastProcessedPos, 
            pos - context->lastProcessedPos + 1
        );
        ProcessJsonResponse(jsonStr, context);
        pos += 2; // Skip "}\n"
        context->lastProcessedPos = pos;
    }
    
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

void ChatGPT::GenerateResponseStream(
    const std::string& input,
    StreamCallback streamCallback,
    CompletionCallback completionCallback) {
    
    HiviewDFX::HiLog::Info(LABEL, "Generating streaming response for input: %{public}s", input.c_str());
    
    // Create context object on heap
    auto* context = new CallbackContext{
        .streamCallback = std::move(streamCallback),
        .completionCallback = std::move(completionCallback)
    };
    
    // Use thread for async execution
    std::thread([this, input, context]() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        CURL* curl = curl_easy_init();
        
        if (!curl) {
            HiviewDFX::HiLog::Error(LABEL, "Failed to initialize CURL");
            context->completionCallback("Error initializing CURL");
            delete context;
            return;
        }

        // Set up CURL headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        // Prepare request payload
        json requestJson = {
            {"model", "deepseek-r1-1.5b"},
            {"prompt", input},
            {"stream", true}
        };
        std::string jsonString = requestJson.dump();

        // Configure CURL options
        curl_easy_setopt(curl, CURLOPT_URL, "http://10.0.90.97:11434/api/generate");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, context);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        HiviewDFX::HiLog::Info(LABEL, "Making request to Ollama API");
        // Perform request
        CURLcode res = curl_easy_perform(curl);
        
        HiviewDFX::HiLog::Info(LABEL, "Making request to Ollama API end");
        // Cleanup CURL resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (res != CURLE_OK) {
            std::string error = curl_easy_strerror(res);
            HiviewDFX::HiLog::Error(LABEL, "CURL request failed: %{public}s", error.c_str());
            context->completionCallback("CURL error: " + error);
        } else {
            HiviewDFX::HiLog::Info(LABEL, "Request completed successfully");
            context->completionCallback("Generation completed successfully");
        }
        
        delete context;
    }).detach();
}

} // namespace Communication
} // namespace OHOS
