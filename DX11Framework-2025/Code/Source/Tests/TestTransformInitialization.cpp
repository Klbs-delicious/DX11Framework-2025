#ifdef _DEBUG
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

        // --- Local系の初期値チェック ---
        const auto& localPos = transform->GetLocalPosition();
        AssertNear(localPos.x, 0.0f);
        AssertNear(localPos.y, 0.0f);
        AssertNear(localPos.z, 0.0f);

        const auto& localRot = transform->GetLocalRotation();
        AssertNear(localRot.x, 0.0f);
        AssertNear(localRot.y, 0.0f);
        AssertNear(localRot.z, 0.0f);
        AssertNear(localRot.w, 1.0f);

        const auto& localScale = transform->GetLocalScale();
        AssertNear(localScale.x, 1.0f);
        AssertNear(localScale.y, 1.0f);
        AssertNear(localScale.z, 1.0f);

        // --- World系の確認（行列とforwardベクトル） ---
        const auto& worldMatrix = transform->GetWorldMatrix();
        AssertNear(worldMatrix._11, 1.0f);
        AssertNear(worldMatrix._22, 1.0f);
        AssertNear(worldMatrix._33, 1.0f);
        AssertNear(worldMatrix._41, 0.0f);
        AssertNear(worldMatrix._42, 0.0f);
        AssertNear(worldMatrix._43, 0.0f);

        const auto forward = transform->Forward();
        AssertNear(forward.x, 0.0f);
        AssertNear(forward.y, 0.0f);
        AssertNear(forward.z, 1.0f);
    }
};

TransformInitializationTestSuite g_transformInitializationTests;

} // namespace
#endif // DEBUG