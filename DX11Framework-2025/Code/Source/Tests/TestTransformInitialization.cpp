#include "Include/Framework/Entities/GameObject.h"

#include <cassert>
#include <cmath>
#include <string>

namespace
{
class DummyObserver final : public IGameObjectObserver
{
public:
    void OnGameObjectEvent(GameObject*, GameObjectEvent) override {}
};

constexpr float kEpsilon = 1.0e-5f;

void AssertNear(float lhs, float rhs)
{
    assert(std::fabs(lhs - rhs) <= kEpsilon);
}

struct TransformInitializationTestSuite
{
    TransformInitializationTestSuite()
    {
        DummyObserver observer;
        GameObject gameObject(observer, "TransformInitializationTest");
        Transform* transform = gameObject.transform;
        assert(transform != nullptr);

        const auto& worldMatrix = transform->GetWorldMatrix();
        AssertNear(worldMatrix._11, 1.0f);
        AssertNear(worldMatrix._22, 1.0f);
        AssertNear(worldMatrix._33, 1.0f);
        AssertNear(worldMatrix._41, 0.0f);
        AssertNear(worldMatrix._42, 0.0f);
        AssertNear(worldMatrix._43, 0.0f);

        const auto rotation = transform->GetWorldRotation();
        AssertNear(rotation.x, 0.0f);
        AssertNear(rotation.y, 0.0f);
        AssertNear(rotation.z, 0.0f);
        AssertNear(rotation.w, 1.0f);

        const auto forward = transform->Forward();
        AssertNear(forward.x, 0.0f);
        AssertNear(forward.y, 0.0f);
        AssertNear(forward.z, 1.0f);
    }
};

TransformInitializationTestSuite g_transformInitializationTests;

} // namespace
