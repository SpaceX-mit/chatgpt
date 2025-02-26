import { ChatGPTService } from '../services/ChatGPTService'
import prompt from '@system.prompt'

@Entry
@Component
struct Index {
  @State userInput: string = ''
  @State response: string = ''
  @State isLoading: boolean = false
  @State statusMessage: string = ''
  private chatGPTService: ChatGPTService = ChatGPTService.getInstance()
  private controller: TextInputController = new TextInputController()

  aboutToAppear() {
    try {
      console.info('Initializing ChatGPT service...')
      this.chatGPTService = ChatGPTService.getInstance()
      console.info('ChatGPT service initialized successfully')
    } catch (error) {
      console.error('Failed to initialize ChatGPT service:', error)
      this.showError('Failed to initialize ChatGPT service')
    }
  }

  private showError(message: string) {
    this.statusMessage = message
    prompt.showToast({
      message: message,
      duration: 2000,
      bottom: 50
    })
  }

  build() {
    Column() {
      // Header
      Text('ChatGPT Demo')
        .fontSize(24)
        .fontWeight(FontWeight.Bold)
        .margin({ top: 20, bottom: 20 })

      // Input Area
      Column() {
        TextArea({
          placeholder: 'Enter your message here...',
          text: this.userInput,
          controller: this.controller
        })
          .height(120)
          .width('100%')
          .backgroundColor('#F5F5F5')
          .borderRadius(8)
          .onChange((value: string) => {
            this.userInput = value
          })

        Row() {
          Button('Clear')
            .width('30%')
            .height(40)  // Fixed height for both buttons
            .margin({ right: 10 })
            .borderRadius(8)  // Same border radius as the TextArea
            .backgroundColor('#999999')
            .onClick(() => {
              this.userInput = ''
              this.controller.caretPosition(-1)
            })

          Button({
            type: ButtonType.Normal,
            stateEffect: true
          }) {
            Row() {
              if (this.isLoading) {
                LoadingProgress()
                  .width(20)
                  .height(20)
                  .margin({ right: 10 })
              }
              Text(this.isLoading ? 'Generating...' : 'Send')
            }
          }
          .width('65%')
          .height(40)  // Same height as Clear button
          .borderRadius(8)  // Same border radius
          .enabled(!this.isLoading)
          .onClick(() => {
            if (!this.userInput.trim()) {
              this.showError('Please enter your message')
              return
            }

            this.isLoading = true
            this.response = ''

            try {
              this.chatGPTService.generateResponseStream(
                this.userInput,
                // Stream callback
                (chunk: string) => {
                  if (chunk) {
                    this.response += chunk
                  }
                },
                // Completion callback
                (result: string) => {
                  this.isLoading = false
                  if (result.startsWith('Error:')) {
                    this.showError(result)
                  } else {
                    console.info('Generation completed:', result)
                  }
                }
              )
            } catch (error) {
              this.isLoading = false
              console.error('Error generating response:', error)
              this.showError(`Error: ${error.message || 'Unknown error occurred'}`)
            }
          })
        }
        .width('100%')
        .margin({ top: 10, bottom: 20 })
        .justifyContent(FlexAlign.Center)  // Center the buttons horizontally
      }
      .width('90%')
      .padding(10)

      // Response Area
      if (this.response || this.isLoading) {
        Column() {
          Text('Response:')
            .fontSize(18)
            .fontWeight(FontWeight.Bold)
            .alignSelf(ItemAlign.Start)
            .margin({ bottom: 10 })

          Scroll() {
            Text(this.response || 'Waiting for response...')
              .fontSize(16)
              .backgroundColor('#F5F5F5')
              .padding(15)
              .borderRadius(8)
              .width('100%')
              .margin({ bottom: 20 })
          }
          .width('100%')
          .layoutWeight(1)  // Take remaining space
          .scrollBar(BarState.Auto)
          .edgeEffect(EdgeEffect.Spring)
        }
        .width('90%')
        .layoutWeight(1)  // Take remaining space in parent Column
        .animation({
          duration: 300,
          curve: Curve.EaseOut
        })
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#FFFFFF')
    .padding({ bottom: 20 })  // Add padding at bottom for better appearance
  }
}