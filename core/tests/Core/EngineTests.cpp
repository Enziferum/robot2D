#include <gtest/gtest.h>
#include <robot2D/Engine.hpp>
#include <limits>
#include <stdexcept>

namespace {
    class ProbeApplication final : public robot2D::Application {
    public:
        explicit ProbeApplication(int& destroyed) : m_destroyed(destroyed) {
        }
        ~ProbeApplication() override {
            ++m_destroyed;
        }
        void setup() override {
            ADD_FAILURE() << "Invalid configuration reached setup";
        }
        void render() override {
            ADD_FAILURE() << "Invalid configuration reached rendering";
        }
    private:
        int& m_destroyed;
    };
}

TEST(Engine, RejectsNullApplicationWithoutInitializingDesktop) {
    robot2D::Engine engine;
    EXPECT_THROW(engine.run(nullptr, robot2D::EngineConfiguration{}), std::invalid_argument);
    EXPECT_EQ(robot2D::runEngine(nullptr), -1);
}

TEST(Engine, RejectsNonPositiveFrameRateAndReleasesApplication) {
    int destroyed = 0;
    robot2D::Engine engine;
    for(int rate : {0, -1, std::numeric_limits<int>::min()}) {
        robot2D::EngineConfiguration config({64, 64}, "invalid", rate);
        EXPECT_THROW(engine.run(std::make_unique<ProbeApplication>(destroyed), std::move(config)),
            std::invalid_argument);
    }
    EXPECT_EQ(destroyed, 3);
}

TEST(Engine, RejectsInvalidWindowSizeBeforeInitializingDesktop) {
    int destroyed = 0;
    robot2D::Engine engine;
    for(const auto size : {robot2D::vec2u{0, 1}, robot2D::vec2u{1, 0},
                           robot2D::vec2u{std::numeric_limits<unsigned>::max(), 1}}) {
        robot2D::EngineConfiguration config(size, "invalid");
        EXPECT_THROW(engine.run(std::make_unique<ProbeApplication>(destroyed), std::move(config)),
            std::invalid_argument);
    }
    EXPECT_EQ(destroyed, 3);
}
