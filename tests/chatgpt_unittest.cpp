// foundation/communication/chatgpt/tests/chatgpt_unittest.cpp

#include <gtest/gtest.h>
#include "chatgpt.h"  // 假设这是你的核心头文件

// 测试用例 1: 验证 Add 函数的基本功能
TEST(ChatGPTTest, AddFunctionTest) {
  ChatGPT chat;
  EXPECT_EQ(chat.Add(2, 3), 5);
  EXPECT_EQ(chat.Add(-1, 1), 0);
}

// 测试用例 2: 验证边界条件
TEST(ChatGPTTest, AddEdgeCases) {
  ChatGPT chat;
  EXPECT_EQ(chat.Add(0, 0), 0);
  EXPECT_EQ(chat.Add(INT_MAX, 1), INT_MIN);  // 假设处理溢出逻辑
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
