
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
