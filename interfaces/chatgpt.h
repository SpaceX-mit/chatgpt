#ifndef FOUNDATION_COMMUNICATION_CHATGPT_INTERFACES_CHATGPT_H
#define FOUNDATION_COMMUNICATION_CHATGPT_INTERFACES_CHATGPT_H

#include <string>
#include <functional>
#include <mutex>

namespace OHOS {
namespace Communication {

class ChatGPT {
public:
    using StreamCallback = std::function<void(const std::string&)>;
    using CompletionCallback = std::function<void(const std::string&)>;

    static ChatGPT& GetInstance();
    
    void GenerateResponseStream(
        const std::string& input,
        StreamCallback streamCallback,
        CompletionCallback completionCallback);

    ~ChatGPT();

private:
    ChatGPT();
    ChatGPT(const ChatGPT&) = delete;
    ChatGPT& operator=(const ChatGPT&) = delete;
    
    bool InitializeCurl();
    void CleanupCurl();

    // Static member variables need to be declared in the class
    inline static std::once_flag initFlag;
    inline static bool isInitialized = false;
};

} // namespace Communication
} // namespace OHOS

#endif
