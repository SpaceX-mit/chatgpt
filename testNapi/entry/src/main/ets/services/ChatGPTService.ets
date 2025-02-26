//import chatGpt from '@ohos.chatgpt'

export class ChatGPTService {
  private static instance: ChatGPTService;
  private nativeChatGPT: any;

  private constructor() {
    try {
      this.nativeChatGPT = globalThis.requireNapi('chatgpt_napi');
      console.info('ChatGPT native module loaded:', this.nativeChatGPT);
    } catch (error) {
      console.error('Failed to load ChatGPT native module:', error);
      throw error;
    }
  }

  public static getInstance(): ChatGPTService {
    if (!ChatGPTService.instance) {
      ChatGPTService.instance = new ChatGPTService();
    }
    return ChatGPTService.instance;
  }

  public async generateResponse(input: string): Promise<string> {
    return new Promise((resolve, reject) => {
      try {
        if (!this.nativeChatGPT) {
          throw new Error('Native ChatGPT module not initialized');
        }
        console.info('Generating response for input:', input);
        const response = this.nativeChatGPT.generateResponse(input);
        console.info('Response generated:', response);
        resolve(response);
      } catch (error) {
        console.error('Error in generateResponse:', error);
        reject(error);
      }
    });
  }

  public generateResponseStream(
    input: string,
    streamCallback: (chunk: string) => void,
    completionCallback: (result: string) => void
  ): void {
    try {
      this.nativeChatGPT.generateResponse(input, streamCallback, completionCallback)
    } catch (error) {
      console.error('ChatGPT service error:', error);
      completionCallback(`Error: ${error.message}`);
    }
  }

  private checkNativeModule() {
    if (!this.nativeChatGPT) {
      console.error('Native ChatGPT module not initialized');
      throw new Error('Native module not initialized');
    }
  }
}