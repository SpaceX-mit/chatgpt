
import { ChatGPTService } from '../services/ChatGPTService';

/*
@Entry
@Component
struct Index {
  @State message: string = 'Hello World'
  @State response: string = ''

  build() {
    Row() {
      Column() {
        TextInput({ placeholder: '请输入问题' })
          .onChange((value: string) => {
            this.message = value
          })
          .width('80%')
          .margin(10)

        Button('发送')
          .onClick(() => {
            try {
              // 调用NAPI接口
              this.response = ChatGPTService.generateResponse(this.message)
            } catch (error) {
              console.error('Error generating response:', error)
              this.response = 'Error: ' + error.message
            }
          })
          .width('40%')
          .margin(10)

        Text(this.response)
          .fontSize(16)
          .width('80%')
          .margin(10)
      }
      .width('100%')
    }
    .height('100%')
  }
}*/


@Entry
@Component
struct Index {
  @State message: string = 'Type your message...';
  @State response: string = '';
  @State isLoading: boolean = false;
  private chatGPTService: ChatGPTService = ChatGPTService.getInstance();
  @State userInput: string = '';

  aboutToAppear() {
    try {
      console.info('Initializing ChatGPT service...');
      this.chatGPTService = ChatGPTService.getInstance();
      console.info('ChatGPT service initialized successfully');
    } catch (error) {
      console.error('Failed to initialize ChatGPT service:', error);
      this.message = 'Failed to initialize ChatGPT service';
    }
  }

  build() {
    Column() {
      TextArea({ placeholder: 'Enter your message here...', text: this.userInput })
        .height(100)
        .width('90%')
        .margin(10)
        .onChange((value: string) => {
          this.userInput = value;
        })

      Button('Send Message')
        .width('90%')
        .margin(10)
        .enabled(!this.isLoading)
        .onClick(async () => {
          if (!this.userInput.trim()) {
            return;
          }

          try {
            this.isLoading = true;
            this.message = 'Generating response...';

            const response = await this.chatGPTService.generateResponse(this.userInput);
            this.response = response;
            this.message = 'Response received';
          } catch (error) {
            console.error('Error:', error);
            this.message = `Error: ${error.message}`;
          } finally {
            this.isLoading = false;
          }
        })

      if (this.isLoading) {
        LoadingProgress()
          .width(50)
          .height(50)
          .margin(10)
      }

      Text(this.message)
        .fontSize(16)
        .margin(10)
        .fontColor(this.message.includes('Error') ? '#FF0000' : '#000000')

      if (this.response) {
        Text('Response:')
          .fontSize(18)
          .fontWeight(FontWeight.Bold)
          .margin({ top: 20, bottom: 10 })

        Text(this.response)
          .fontSize(16)
          .margin(10)
          .padding(10)
          .backgroundColor('#F5F5F5')
          .width('90%')
      }
    }
    .width('100%')
    .height('100%')
    .padding(20)
  }
}