#include <editor/physics/IPhysics2DAdapter.hpp>
#include <editor/physics/Box2DPhysicsAdapter.hpp>

namespace editor {
    IPhysics2DAdapter::~IPhysics2DAdapter() = default;

    IPhysics2DAdapter::Ptr getPhysics2DAdapter(PhysicsAdapterType type) {
        switch (type) {
            case PhysicsAdapterType::Box2D:
                return std::make_unique<Box2DPhysicsAdapter>();
        }
    }
}