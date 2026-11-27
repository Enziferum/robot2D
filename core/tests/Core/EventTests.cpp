#include <gtest/gtest.h>
#include <robot2D/Core/EventQueue.hpp>
#include <robot2D/Core/EventBinder.hpp>
#include <robot2D/Core/Window.hpp>

TEST(CoreEvents, PreservesShortPressReleaseAndOwningPayloads) {
    robot2D::EventQueue queue;
    queue.push(robot2D::Event::KeyPressed{{robot2D::Key::A, 12, 0}, false});
    queue.push(robot2D::Event::KeyReleased{{robot2D::Key::A, 12, 0}});
    {
        robot2D::Event::FilesDropped drop{{"temporary/file.png"}};
        queue.push(drop);
        drop.paths.clear();
    }
    const auto events = queue.take();
    ASSERT_EQ(events.size(), 3);
    EXPECT_EQ(events[0].get<robot2D::Event::KeyPressed>().code, robot2D::Key::A);
    EXPECT_EQ(events[1].get<robot2D::Event::KeyReleased>().code, robot2D::Key::A);
    EXPECT_EQ(events[2].get<robot2D::Event::FilesDropped>().paths[0], "temporary/file.png");
    for(std::size_t index = 1; index < events.size(); ++index) {
        EXPECT_LT(events[index - 1].context().sequence, events[index].context().sequence);
        EXPECT_LE(events[index - 1].context().receivedAt, events[index].context().receivedAt);
    }
    EXPECT_TRUE(queue.take().empty());
}

TEST(CoreEvents, PollAndBatchShareOneFifo) {
    robot2D::EventQueue queue;
    queue.push(robot2D::Event::Closed{});
    queue.push(robot2D::Event::LostFocus{});
    robot2D::Event event;
    ASSERT_TRUE(queue.poll(event));
    EXPECT_EQ(event.type(), robot2D::Event::Type::Closed);
    const auto sequence = event.context().sequence;
    const auto batch = queue.take();
    ASSERT_EQ(batch.size(), 1);
    EXPECT_EQ(batch[0].type(), robot2D::Event::Type::LostFocus);
    EXPECT_GT(batch[0].context().sequence, sequence);
    EXPECT_FALSE(queue.poll(event));
}

TEST(CoreEvents, DistinguishesLogicalFramebufferAndScaleEvents) {
    const robot2D::Event event(robot2D::Event::FramebufferResized{{1600, 1200}});
    EXPECT_EQ(event.type(), robot2D::Event::Type::FramebufferResized);
    EXPECT_EQ(event.get<robot2D::Event::SizeEvent>().width, 1600);
    EXPECT_EQ(event.getIf<robot2D::Event::Resized>(), nullptr);
    EXPECT_THROW(event.get<robot2D::Event::ContentScaleChanged>(), std::bad_variant_access);
    auto stamped = robot2D::Event(robot2D::Event::ContentScaleChanged{1.5f, 2.f}).withFrame(42);
    EXPECT_EQ(stamped.context().frame, 42);
    EXPECT_FLOAT_EQ(stamped.get<robot2D::Event::ContentScaleChanged>().y, 2.f);
}

TEST(WindowMetrics, MinimizedAndZeroSurfacesAreNotDrawable) {
    robot2D::WindowMetrics metrics{{800, 600}, {1600, 1200}, {1.5f, 1.5f}, false};
    EXPECT_TRUE(metrics.drawable());
    metrics.minimized = true;
    EXPECT_FALSE(metrics.drawable());
    metrics.minimized = false;
    metrics.framebufferSize.x = 0;
    EXPECT_FALSE(metrics.drawable());
    metrics.framebufferSize.x = 1600;
    metrics.logicalSize.y = 0;
    EXPECT_FALSE(metrics.drawable());
}

TEST(CoreEventBinder, DispatchesConcretePayloadAndMetadataWithoutEditor) {
    robot2D::EventBinder binder;
    robot2D::Event event(robot2D::Event::KeyPressed{{robot2D::Key::A, 12, 0}, false},
        {7, 42, robot2D::EventTime{13}});
    EXPECT_EQ(binder.dispatch(event), robot2D::EventResult::Ignored);
    int received = 0;
    binder.bind<robot2D::Event::KeyPressed>([&](const robot2D::Event::KeyPressed& key,
        const robot2D::EventContext& context) {
        EXPECT_EQ(key.code, robot2D::Key::A);
        EXPECT_EQ(context.sequence, 7);
        EXPECT_EQ(context.frame, 42);
        ++received;
    });
    EXPECT_EQ(binder.dispatch(event), robot2D::EventResult::Handled);
    EXPECT_EQ(received, 1);
    binder.unbind<robot2D::Event::KeyPressed>();
    EXPECT_EQ(binder.dispatch(event), robot2D::EventResult::Ignored);
}

TEST(CoreEventBinder, RestoresDispatchStateAfterCallbackFailure) {
    robot2D::EventBinder binder;
    robot2D::Event event(robot2D::Event::Closed{});
    binder.bind<robot2D::Event::Closed>([&](const robot2D::Event::Closed&) {
        EXPECT_THROW(binder.unbind<robot2D::Event::Closed>(), std::logic_error);
        throw std::runtime_error("callback failure");
    });
    EXPECT_THROW(binder.dispatch(event), std::runtime_error);
    EXPECT_NO_THROW(binder.unbind<robot2D::Event::Closed>());
    binder.bind<robot2D::Event::Closed>([](const robot2D::Event::Closed&) {
        return robot2D::EventResult::Ignored;
    });
    EXPECT_EQ(binder.dispatch(event), robot2D::EventResult::Ignored);
}
