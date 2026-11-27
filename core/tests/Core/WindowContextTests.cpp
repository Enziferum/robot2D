#include <gtest/gtest.h>
#include <robot2D/Core/WindowContext.hpp>

TEST(WindowContext, PreservesExplicitVersion) {
    using Api = robot2D::WindowContext::RenderApi;
    for(const auto api : {Api::OpenGL4_1, Api::OpenGL4_3, Api::OpenGL4_5}) {
        const robot2D::WindowContext context(false, false, api);
        EXPECT_EQ(context.renderApi, api);
    }
}

TEST(WindowContext, PlatformDefault) {
#if defined(ROBOT2D_MACOS)
    EXPECT_EQ(robot2D::WindowContext::Default.renderApi, robot2D::WindowContext::RenderApi::OpenGL4_1);
#else
    EXPECT_EQ(robot2D::WindowContext::Default.renderApi, robot2D::WindowContext::RenderApi::OpenGL4_3);
#endif
}
