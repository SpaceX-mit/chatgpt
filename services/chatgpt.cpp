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

ChatGPT::ChatGPT() {
    HiviewDFX::HiLog::Info(LABEL, "ChatGPT constructor called");
    std::call_once(initFlag, [this]() {
        HiviewDFX::HiLog::Info(LABEL, "Initializing CURL (call_once)");
        if (InitializeCurl()) {
            isInitialized = true;
            HiviewDFX::HiLog::Info(LABEL, "CURL globally initialized successfully");
        } else {
            HiviewDFX::HiLog::Error(LABEL, "CURL global initialization failed");
        }
    });
    HiviewDFX::HiLog::Info(LABEL, "ChatGPT constructor finished, isInitialized: %{public}d", isInitialized);
}

ChatGPT::~ChatGPT() {
    if (isInitialized) {
        CleanupCurl();
    }
}

bool ChatGPT::InitializeCurl() {
    HiviewDFX::HiLog::Info(LABEL, "InitializeCurl called");
    CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        HiviewDFX::HiLog::Error(LABEL, "curl_global_init failed with code: %{public}d, error: %{public}s", 
            code, curl_easy_strerror(code));
        return false;
    }
    HiviewDFX::HiLog::Info(LABEL, "curl_global_init succeeded");
    return true;
}

void ChatGPT::CleanupCurl() {
    HiviewDFX::HiLog::Info(LABEL, "Cleaning up CURL global state");
    curl_global_cleanup(); 
    isInitialized = false;
    HiviewDFX::HiLog::Info(LABEL, "CURL global state cleaned up successfully");
}

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
    
    if (!isInitialized) {
        completionCallback("CURL not initialized");
        return;
    }

    auto* context = new CallbackContext{
        .streamCallback = std::move(streamCallback),
        .completionCallback = std::move(completionCallback)
    };
    
    std::thread([this, input, context]() {
        CURL* curl = curl_easy_init();
        
        if (!curl) {
            HiviewDFX::HiLog::Error(LABEL, "Failed to initialize CURL");
            context->completionCallback("Error initializing CURL");
            delete context;
            return;
        }

        // Set up CURL headers with error checking
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!headers) {
            HiviewDFX::HiLog::Error(LABEL, "Failed to create headers");
            curl_easy_cleanup(curl);
            context->completionCallback("Error creating headers");
            delete context;
            return;
        }
        
        // Prepare request payload
        json requestJson = {
            {"model", "deepseek-r1-1.5b"},
            {"prompt", input},
            {"stream", true}
        };
        std::string jsonString = requestJson.dump();
        HiviewDFX::HiLog::Info(LABEL, "Request payload: %{public}s", jsonString.c_str());

        // Configure CURL options with improved settings
        const char* url = "http://10.0.91.97:11434/api/generate";
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, context);
        
        // 增加连接延迟容忍
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);  // 连接超时改为5秒
        //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);        // 总超时改为60秒
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);   // 启用 TCP keepalive
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);  // keepalive idle 时间
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);  // keepalive 间隔
        
        // For streaming applications, consider using these settings:
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);         // No timeout for transfer
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L); // Alternative: Timeout if speed is too low
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L); // for 30 seconds (1 byte/sec minimum)
        
        // 调试选项
        #ifdef DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        #endif

        HiviewDFX::HiLog::Info(LABEL, "Making request to Ollama API at %{public}s", url);
        
        // 增加重试逻辑
        int maxRetries = 3;
        int retryCount = 0;
        CURLcode res;
        
        do {
            if (retryCount > 0) {
                HiviewDFX::HiLog::Info(LABEL, "Retry attempt %{public}d after 2 second delay", retryCount);
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            
            res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                break;
            }
            
            HiviewDFX::HiLog::Error(LABEL, "CURL attempt %{public}d failed: %{public}s", 
                retryCount + 1, curl_easy_strerror(res));
                
            retryCount++;
        } while (retryCount < maxRetries && 
                (res == CURLE_COULDNT_CONNECT || res == CURLE_OPERATION_TIMEDOUT));
        
        HiviewDFX::HiLog::Info(LABEL, "CURL request completed after %{public}d attempts", retryCount + 1);
        
        // Cleanup CURL resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::string error = curl_easy_strerror(res);
            HiviewDFX::HiLog::Error(LABEL, "CURL request failed after %{public}d attempts: %{public}s", 
                retryCount + 1, error.c_str());
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
