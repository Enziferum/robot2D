#include <gtest/gtest.h>
#include <robot2D/Util/ResourceHandler.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace {
    struct Resource {
        inline static int constructions = 0;
        inline static int loads = 0;
        inline static int alive = 0;
        std::string value;

        Resource() {
            ++constructions; ++alive;
        }
        Resource(const Resource&) = delete;
        Resource& operator=(const Resource&) = delete;
        Resource(Resource&& other) noexcept : value(std::move(other.value)) {
            ++alive;
        }
        Resource& operator=(Resource&&) = delete;
        ~Resource() {
            --alive;
        }

        bool loadFromFile(std::string path) {
            ++loads;
            value = std::move(path);
            return value != "missing";
        }
    };

    struct NonDefaultResource {
        explicit NonDefaultResource(int number) : value(number) {
        }
        NonDefaultResource(NonDefaultResource&&) = default;
        NonDefaultResource& operator=(NonDefaultResource&&) = delete;
        int value;
    };

    struct ImmovableResource {
        ImmovableResource() = default;
        ImmovableResource(const ImmovableResource&) = delete;
        ImmovableResource(ImmovableResource&&) = delete;
    };
}

TEST(ResourceHandler, MissingLookupDoesNotInsert) {
    robot2D::ResourceHandler<Resource> resources;
    EXPECT_EQ(resources.find("missing"), nullptr);
    EXPECT_THROW(resources.get("missing"), std::out_of_range);
    EXPECT_THROW(resources["missing"], std::out_of_range);
    EXPECT_FALSE(resources.has("missing"));
    EXPECT_TRUE(resources.empty());
    EXPECT_EQ(resources.size(), 0u);
    const auto& readOnly = resources;
    EXPECT_EQ(readOnly.find("missing"), nullptr);
    EXPECT_THROW(readOnly["missing"], std::out_of_range);
    EXPECT_THROW(readOnly.get("missing"), std::out_of_range);
}

TEST(ResourceHandler, DuplicateLoadDoesNotConstructOrLoad) {
    robot2D::ResourceHandler<Resource> resources;
    ASSERT_TRUE(resources.loadFromFile("id", "first"));
    const auto constructions = Resource::constructions;
    const auto loads = Resource::loads;
    EXPECT_FALSE(resources.loadFromFile("id", "second"));
    EXPECT_EQ(Resource::constructions, constructions);
    EXPECT_EQ(Resource::loads, loads);
    EXPECT_EQ(resources.get("id").value, "first");
    EXPECT_EQ(resources.size(), 1u);
}

TEST(ResourceHandler, FailedLoadAllowsRetryAndReleasesTemporary) {
    robot2D::ResourceHandler<Resource> resources;
    const auto alive = Resource::alive;
    EXPECT_FALSE(resources.loadFromFile("id", "missing"));
    EXPECT_TRUE(resources.empty());
    EXPECT_EQ(Resource::alive, alive);
    EXPECT_TRUE(resources.loadFromFile("id", "valid"));
    EXPECT_EQ(resources.get("id").value, "valid");
}

TEST(ResourceHandler, MoveConstructionDoesNotRequireAssignmentOrDefaultConstructor) {
    robot2D::ResourceHandler<NonDefaultResource> resources;
    EXPECT_TRUE(resources.add("id", NonDefaultResource{42}));
    EXPECT_EQ(resources.get("id").value, 42);
}

TEST(ResourceHandler, DuplicateAddLeavesSourceUntouched) {
    robot2D::ResourceHandler<Resource> resources;
    ASSERT_NE(resources.add("id"), nullptr);
    Resource source;
    source.value = "keep";
    const auto constructions = Resource::constructions;
    EXPECT_EQ(resources.add("id"), nullptr);
    EXPECT_EQ(Resource::constructions, constructions);
    EXPECT_FALSE(resources.add("id", std::move(source)));
    // Duplicate rejection guarantees that the rvalue reference is not moved from.
    EXPECT_EQ(source.value, "keep"); // NOLINT(bugprone-use-after-move)
}

TEST(ResourceHandler, DefaultInsertionDoesNotRequireMovability) {
    robot2D::ResourceHandler<ImmovableResource> resources;
    EXPECT_NE(resources.add("id"), nullptr);
}

TEST(ResourceHandler, InsertionPreservesReferencesAndConstAccess) {
    robot2D::ResourceHandler<Resource> resources;
    auto* original = resources.add("first");
    ASSERT_NE(original, nullptr);
    original -> value = "stable";
    for(int i = 0; i < 100; ++i)
        ASSERT_NE(resources.add(std::to_string(i)), nullptr);

    const auto& readOnly = resources;
    static_assert(std::is_same_v<decltype(readOnly.find("first")), const Resource*>);
    EXPECT_EQ(readOnly.find("first"), original);
    EXPECT_EQ(&readOnly["first"], original);
    EXPECT_EQ(&readOnly.get("first"), original);
    EXPECT_EQ(readOnly.get("first").value, "stable");
}

TEST(ResourceHandler, DestructionReleasesOwnedResources) {
    const auto alive = Resource::alive;
    {
        robot2D::ResourceHandler<Resource> resources;
        ASSERT_NE(resources.add("first"), nullptr);
        ASSERT_NE(resources.add("second"), nullptr);
        EXPECT_EQ(Resource::alive, alive + 2);
    }
    EXPECT_EQ(Resource::alive, alive);
}
