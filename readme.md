
# ChatGPT API 使用示例

这个项目展示了如何使用 ChatGPT API 进行对话交互。
克隆这个 GitHub 项目到本地。以下是具体步骤：

1. 打开命令行终端，进入你想要存放项目的目录。
2. 运行以下命令克隆仓库：
```bash
git clone https://github.com/SpaceX-mit/chatgpt.git
 ```

3. 进入项目目录：
```bash
cd chatgpt
 ```

完成这些步骤后，项目就会被克隆到你的本地机器上。你可以在 chatgpt 目录中找到所有项目文件。

如果你遇到克隆失败的问题，可能是因为：

1. 没有安装 Git
2. 网络连接问题
3. 仓库访问权限问题
如果你还没有安装 Git，需要先从 Git 官网 下载并安装。

## 功能特点

- 支持与 ChatGPT 进行自然语言对话
- 可以保存对话历史
- 支持多轮对话上下文
- 可配置不同的模型参数

## 安装


分析这个项目的napi到C++代码的流程
分析这个项目从前端到 C++ 的完整调用链路：

1. 前端层 (ArkTS) ：
- `Index.ets` 中：
  - 用户界面通过 ChatGPTService.getInstance() 获取服务实例
  - 调用 generateResponse 方法发送用户输入
2. 服务层 (ArkTS) ：
- `ChatGPTService.ets` 中：
  - 使用单例模式管理服务实例
  - 通过 requireNapi('chatgpt_napi') 加载原生模块
  - 封装 NAPI 方法调用，提供 Promise 接口
3. NAPI 层 ：
- `chatgpt_napi.cpp` 中：
  - 通过 NAPI_MODULE 注册模块
  - Init 函数注册 generateResponse 方法
  - GenerateResponse 函数处理参数转换和回调设置
  - 使用 napi_async_work 处理异步操作
4. C++ 实现层 ：
- `chatgpt.h` 定义了核心接口
- `chatgpt.cpp` 实现具体功能：
  - 单例模式管理 ChatGPT 实例
  - CURL 处理 HTTP 请求
  - 流式响应处理
调用流程：

1. 用户在 UI 输入文本 → 触发 onClick 事件
2. ChatGPTService 调用 NAPI 模块的 generateResponse
3. NAPI 层转换参数，创建异步工作
4. C++ 层执行 HTTP 请求，通过回调返回结果
5. 结果通过回调链传回前端显示
关键技术点：

1. 异步处理：使用 napi_async_work 避免阻塞主线程
2. 内存管理：正确处理回调上下文的生命周期
3. 错误处理：各层都有完整的错误处理机制
4. 类型转换：JS/TS 与 C++ 类型的安全转换
这个实现是完整和合理的，符合 NAPI 的最佳实践。


相许设计：
让我分析这个项目的整体架构和调用流程：

1. **整体架构**：
项目采用分层架构设计：
- 表现层（UI）：<mcfile name="Index.ets" path="d:\2025\work\chatgpt\chatgpt\testNapi\entry\src\main\ets\pages\Index.ets"></mcfile>
- 服务层：<mcfile name="ChatGPTService.ets" path="d:\2025\work\chatgpt\chatgpt\testNapi\entry\src\main\ets\services\ChatGPTService.ets"></mcfile>
- NAPI层：<mcfile name="chatgpt_napi.cpp" path="d:\2025\work\chatgpt\chatgpt\napi\chatgpt_napi.cpp"></mcfile>
- C++核心层：<mcfile name="chatgpt.h" path="d:\2025\work\chatgpt\chatgpt\interfaces\chatgpt.h"></mcfile> 和 <mcfile name="chatgpt.cpp" path="d:\2025\work\chatgpt\chatgpt\services\chatgpt.cpp"></mcfile>

2. **调用流程**：

```plaintext
UI层(Index.ets) 
    ↓ 调用
服务层(ChatGPTService.ets)
    ↓ NAPI调用
NAPI层(chatgpt_napi.cpp)
    ↓ C++调用
C++核心层(chatgpt.cpp)
    ↓ HTTP请求
Ollama API服务
```

3. **具体流程分析**：

a) **UI 层触发**：
```typescript
// Index.ets 中的点击事件
this.chatGPTService.generateResponseStream(
    this.userInput,
    (chunk: string) => { this.response += chunk },
    (result: string) => { this.isLoading = false }
)
```

b) **服务层处理**：
```typescript
// ChatGPTService.ets
public generateResponseStream(input: string, streamCallback, completionCallback): void {
    this.nativeChatGPT.generateResponse(input, streamCallback, completionCallback)
}
```

c) **NAPI 层转换**：
```cpp
// chatgpt_napi.cpp
napi_value GenerateResponse(napi_env env, napi_callback_info info) {
    // 参数转换
    // 创建异步工作
    OHOS::Communication::ChatGPT::GetInstance().GenerateResponseStream(
        input, streamCallback, completionCallback);
}
```

d) **C++ 核心实现**：
```cpp
// chatgpt.cpp
void ChatGPT::GenerateResponseStream(
    const std::string& input,
    StreamCallback streamCallback,
    CompletionCallback completionCallback) {
    // 执行 HTTP 请求
    // 处理流式响应
}
```

4. **关键特性**：

- **单例模式**：
  - ChatGPTService 使用单例
  - C++ ChatGPT 类使用单例

- **异步处理**：
  - NAPI 层使用 `napi_async_work`
  - C++ 层使用 `std::thread`
  - UI 层使用回调处理响应

- **流式处理**：
  - 支持数据流式返回
  - 实时更新 UI 显示

5. **错误处理**：
- 每一层都有完整的错误处理机制
- 错误信息可以传递到 UI 层显示

6. **资源管理**：
- CURL 资源的初始化和清理
- NAPI 引用的管理
- 内存的正确释放


详细分析 <mcfile name="chatgpt_napi.cpp" path="d:\2025\work\chatgpt\chatgpt\napi\chatgpt_napi.cpp"></mcfile> 的代码流程：

1. **模块初始化流程**：
```cpp
NAPI_MODULE(chatgpt_napi, Init)  // 注册模块
↓
Init(napi_env env, napi_value exports)  // 初始化函数
↓
napi_define_properties  // 注册 generateResponse 方法
```

2. **生成响应流程** <mcsymbol name="GenerateResponse" filename="chatgpt_napi.cpp" path="d:\2025\work\chatgpt\chatgpt\napi\chatgpt_napi.cpp" startline="71" type="function">：
```cpp
GenerateResponse
↓
1. 获取参数 (napi_get_cb_info)
2. 创建回调引用 (napi_create_reference)
3. 获取输入字符串 (napi_get_value_string_utf8)
4. 创建流回调和完成回调
5. 调用 C++ 接口
```

3. **流式回调处理流程**：
```cpp
streamCallback lambda
↓
创建 AsyncCallbackData
↓
创建异步工作 (napi_create_async_work)
↓
提交异步工作 (napi_queue_async_work)
↓
StreamCallbackComplete
↓
调用 JavaScript 回调函数
```

4. **完成回调处理流程**：
```cpp
completionCallback lambda
↓
创建 AsyncCallbackData
↓
创建异步工作
↓
提交异步工作
↓
CompletionCallbackComplete
↓
调用 JavaScript 回调函数
↓
清理资源
```

5. **关键数据结构**：
```cpp
struct AsyncCallbackData {
    napi_env env;                    // NAPI 环境
    napi_ref streamCallbackRef;      // 流回调引用
    napi_ref completionCallbackRef;  // 完成回调引用
    std::string chunk;               // 数据块
    std::string result;              // 结果
    napi_value resourceName;         // 资源名
};
```

6. **异步工作处理**：
- `ExecuteAsyncWork`：工作线程执行函数（本例中为空）
- `StreamCallbackComplete`：流回调完成处理
- `CompletionCallbackComplete`：完成回调处理

7. **资源管理**：
- 创建持久引用：`napi_create_reference`
- 删除引用：`napi_delete_reference`
- 释放内存：`delete callbackData`

8. **错误处理**：
- 检查 NAPI 函数返回值
- 失败时及时返回
- 确保资源正确清理

这个实现的特点：
1. 异步处理避免阻塞
2. 完善的内存管理
3. 良好的错误处理
4. 支持流式数据
5. 清晰的模块化设计


分析 <mcfile name="chatgpt.cpp" path="d:\2025\work\chatgpt\chatgpt\services\chatgpt.cpp"></mcfile> 的代码流程：

1. **初始化流程**：
```cpp
ChatGPT::ChatGPT()
↓
std::call_once(initFlag, [this]() {
    InitializeCurl()  // CURL 全局初始化
})
```

2. **回调上下文结构**：
```cpp
struct CallbackContext {
    std::string accumulatedResponse;      // 累积的响应数据
    StreamCallback streamCallback;         // 流式回调
    CompletionCallback completionCallback; // 完成回调
    size_t lastProcessedPos = 0;          // 上次处理位置
}
```

3. **主要处理流程** <mcsymbol name="GenerateResponseStream" filename="chatgpt.cpp" path="d:\2025\work\chatgpt\chatgpt\services\chatgpt.cpp" startline="108" type="function">：
```cpp
GenerateResponseStream
↓
创建 CallbackContext
↓
启动新线程
↓
初始化 CURL
↓
设置请求参数
↓
执行请求（带重试机制）
↓
清理资源
```

4. **数据处理流程**：
```cpp
WriteCallback（接收数据）
↓
累积响应数据
↓
查找完整 JSON 对象
↓
ProcessJsonResponse（解析 JSON）
↓
调用流式回调
```

5. **错误处理机制**：
- CURL 初始化检查
- 请求头创建检查
- 请求执行状态检查
- JSON 解析异常处理
- 重试机制（最多3次）

6. **关键优化特性**：
- TCP keepalive 支持
- 连接超时设置
- 低速传输保护
- 流式传输优化
- 调试日志支持

7. **资源管理**：
```cpp
资源分配
↓
使用资源
↓
清理顺序：
1. CURL 请求清理
2. 请求头清理
3. 上下文清理
```

8. **日志记录**：
- 使用 HiLog 记录关键操作
- 记录错误信息
- 记录请求状态
- 记录重试信息

主要特点：
1. 单例模式确保全局唯一实例
2. 异步处理避免阻塞
3. 完善的错误处理
4. 优化的网络设置
5. 资源安全管理
6. 详细的日志记录

这个实现提供了可靠的流式数据处理和错误恢复机制，适合长连接场景下的数据传输。