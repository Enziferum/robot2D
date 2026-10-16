#include <gtest/gtest.h>
#include <robot2D/Core/MessageBus.hpp>
#include <cstdint>

namespace {
    struct alignas(32) Payload {
        inline static int alive = 0;
        std::string text = "constructed";
        Payload() {
            ++alive;
        }
        ~Payload() {
            --alive;
        }
    };

    struct LargePayload { char bytes[129]; };

    struct ThrowingPayload {
        ThrowingPayload() {
            throw std::runtime_error("constructor failed");
        }
    };
}

TEST(MessageBus, ConstructsAlignedPayloadAndKeepsItAfterPolling) {
    robot2D::MessageBus bus;
    const auto alive = Payload::alive;
    auto* payload = bus.postMessage<Payload>(42);
    ASSERT_NE(payload, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(payload) % alignof(Payload), 0u);
    EXPECT_EQ(payload -> text, "constructed");
    robot2D::Message message;
    ASSERT_TRUE(bus.pollMessages(message));
    EXPECT_EQ(&message.getData<Payload>(), payload);
    EXPECT_TRUE(bus.empty());
    EXPECT_EQ(Payload::alive, alive + 1);
    bus.clear();
    EXPECT_EQ(Payload::alive, alive);
    EXPECT_FALSE(bus.pollMessages(message));
}

TEST(MessageBus, ClearDropsQueuedMessagesAndAllowsReuse) {
    robot2D::MessageBus bus;
    for(int cycle = 0; cycle < 5; ++cycle) {
        for(int i = 0; i < 30; ++i)
            *bus.postMessage<int>(i) = i;
        EXPECT_EQ(bus.pendingMessages(), 30u);
        bus.clear();
        EXPECT_TRUE(bus.empty());
        bus.clear();
    }
    *bus.postMessage<int>(99) = 123;
    robot2D::Message message;
    ASSERT_TRUE(bus.pollMessages(message));
    EXPECT_EQ(message.id, 99);
    EXPECT_EQ(message.getData<int>(), 123);
}

TEST(MessageBus, DeliversFifoAndDestroysUnpolledPayloads) {
    const auto alive = Payload::alive;
    {
        robot2D::MessageBus bus;
        bus.postMessage<Payload>(10);
        bus.postMessage<Payload>(20);
        robot2D::Message message;
        ASSERT_TRUE(bus.pollMessages(message));
        EXPECT_EQ(message.id, 10);
        EXPECT_EQ(bus.pendingMessages(), 1u);
    }
    EXPECT_EQ(Payload::alive, alive);
}

TEST(MessageBus, RejectsInvalidSizesAndRollsBackThrowingConstruction) {
    robot2D::MessageBus bus;
    EXPECT_THROW(bus.postMessage(1, 0), std::length_error);
    EXPECT_THROW(bus.postMessage<LargePayload>(1), std::length_error);
    EXPECT_THROW(bus.postMessage<ThrowingPayload>(1), std::runtime_error);
    EXPECT_TRUE(bus.empty());
    EXPECT_NE(bus.postMessage<int>(2), nullptr);
}

TEST(MessageBus, ExhaustionDoesNotQueueFailedAllocationAndClearRestoresCapacity) {
    robot2D::MessageBus bus;
    bool exhausted = false;
    for(int i = 0; i < 10000; ++i) {
        const auto before = bus.pendingMessages();
        try {
            bus.postMessage(1, 128);
        }
        catch(const robot2D::BadMessageAllocation&) {
            EXPECT_EQ(bus.pendingMessages(), before);
            exhausted = true;
            break;
        }
    }
    EXPECT_TRUE(exhausted);
    bus.clear();
    EXPECT_NE(bus.postMessage(2, 128), nullptr);
}

TEST(MemoryManager, DeferredReleaseReclaimsWholeStack) {
    robot2D::mem::MemoryManager<robot2D::mem::StackAllocator> memory;
    auto* first = memory.allocate(13);
    auto* second = memory.allocate(27);
    auto* third = memory.allocate(5);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);
    memory.deallocate(first);
    memory.deallocate(second);
    memory.deallocate(third);
    auto* reused = memory.allocate(13);
    EXPECT_EQ(reused, first);
    memory.deallocate(reused);
    memory.deallocate(nullptr);
}

TEST(MessageBus, RawPayloadCanExceedTypedLimit) {
    robot2D::MessageBus bus;
    auto* data = static_cast<unsigned char*>(bus.postMessage(42, 512));
    ASSERT_NE(data, nullptr);
    data[511] = 123;
    robot2D::Message message;
    ASSERT_TRUE(bus.pollMessages(message));
    EXPECT_TRUE(message.needUnpack());
    EXPECT_EQ(data[511], 123);
    EXPECT_THROW(message.getData<int>(), std::invalid_argument);
}
