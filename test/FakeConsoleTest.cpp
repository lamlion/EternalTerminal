#include "Headers.hpp"

#include "gtest/gtest.h"

#include "FakeConsole.hpp"

namespace et {
class FakeConsoleTest : public testing::Test {
 protected:
  void SetUp() override {
    socketHandler.reset(new PipeSocketHandler());
    fakeConsole.reset(new FakeConsole(socketHandler));
    fakeConsole->setup();
  }

  void TearDown() override {
    fakeConsole->teardown();
    fakeConsole.reset();
    socketHandler.reset();
  }

  shared_ptr<PipeSocketHandler> socketHandler;
  shared_ptr<FakeConsole> fakeConsole;
};

TEST_F(FakeConsoleTest, ReadWrite) {
  string s(64 * 1024, '\0');
  for (int a = 0; a < 64 * 1024 - 1; a++) {
    s[a] = rand() % 26 + 'A';
  }
  s[64 * 1024 - 1] = 0;

  EXPECT_FALSE(socketHandler->hasData(fakeConsole->getFd()));

  thread t([this, s]() { fakeConsole->simulateKeystrokes(s); });
  usleep(1000);

  EXPECT_TRUE(socketHandler->hasData(fakeConsole->getFd()));

  string s2(64 * 1024, '\0');
  socketHandler->readAll(fakeConsole->getFd(), &s2[0], s2.length(), false);

  t.join();

  EXPECT_EQ(s, s2);

  thread t2([this, s]() { fakeConsole->write(s); });

  string s3 = fakeConsole->getTerminalData(s.length());
  EXPECT_EQ(s, s3);

  t2.join();
}
}  // namespace et