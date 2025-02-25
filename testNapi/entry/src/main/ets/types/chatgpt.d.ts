// entry/src/main/ets/types/chatgpt.d.ts
declare module '@ohos/chatgpt' {
  export interface ChatGPT {
    generateResponse(input: string): Promise<string>;
  }

  const chatgpt: ChatGPT;
  export default chatgpt;
}