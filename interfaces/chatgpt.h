#ifndef CHATGPT_H
#define CHATGPT_H

#include <string>

namespace OHOS {
namespace Communication {
class ChatGPT {
public:
    static ChatGPT& GetInstance();
    std::string GenerateResponse(const std::string& input);
};
} // namespace Communication
} // namespace OHOS

#endif
