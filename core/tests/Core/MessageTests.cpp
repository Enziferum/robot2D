#include <gtest/gtest.h>
#include <robot2D/Core/Message.hpp>
#include <robot2D/Core/MessageBus.hpp>

namespace {
    struct TestMessage {
        std::string a;
        int b;
    };

    enum TestMessageID {
        Test = robot2D::Message::Count
    };
}

class CoreMessageBusMock: public ::testing::Test {
public:
    void SetUp() override {
        auto* msg = m_messageBus.postMessage<TestMessage>(TestMessageID::Test);
        msg -> a = "42";
        msg -> b = 42;
    }
protected:
    robot2D::MessageBus m_messageBus;
};


TEST_F(CoreMessageBusMock, Core_Message_Test) {
    robot2D::Message message;
    m_messageBus.pollMessages(message);
    const auto& data = message.getData<TestMessage>();
    EXPECT_EQ(data.a, "42");
    EXPECT_EQ(data.b, 42);
}

TEST_F(CoreMessageBusMock, Core_MessageBadSize_Test) {
    robot2D::Message message;
    m_messageBus.pollMessages(message);
    const auto& data = message.getData<TestMessage>();
}