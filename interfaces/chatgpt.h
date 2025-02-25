#ifndef FOUNDATION_COMMUNICATION_CHATGPT_INTERFACES_CHATGPT_H
#define FOUNDATION_COMMUNICATION_CHATGPT_INTERFACES_CHATGPT_H

#include <string>
#include <functional>

namespace OHOS {
namespace Communication {

class ChatGPT {
public:
    // Define callback types as public members
    using StreamCallback = std::function<void(const std::string&)>;
    using CompletionCallback = std::function<void(const std::string&)>;

    static ChatGPT& GetInstance();
    
    void GenerateResponseStream(
        const std::string& input,
        StreamCallback streamCallback,
        CompletionCallback completionCallback);

private:
    ChatGPT() = default;
    ~ChatGPT() = default;
    ChatGPT(const ChatGPT&) = delete;
    ChatGPT& operator=(const ChatGPT&) = delete;
};

} // namespace Communication
} // namespace OHOS

#endif
