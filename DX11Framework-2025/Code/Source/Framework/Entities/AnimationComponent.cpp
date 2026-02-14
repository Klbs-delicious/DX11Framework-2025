/** @file   AnimationComponent.cpp
 *  @brief  アニメーション更新専用コンポーネント
 *  @date   2026/01/19
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Utils/CommonTypes.h"

#include "Include/Framework/Graphics/IAnimator.h"

#include <algorithm>
#include <cassert>

static_assert(ShaderCommon::MaxBones == 128, "HLSL BoneBuffer boneMatrices[128] と C++ 側 MaxBones が不一致です。");

//-----------------------------------------------------------------------------
// AnimationComponent Methods
//-----------------------------------------------------------------------------
AnimationComponent::AnimationComponent(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	boneCB(nullptr),
	meshComponent(nullptr),
	skeletonCache(nullptr),
	isSkeletonCached(false)
{
}

void AnimationComponent::Initialize()
{
	if (!this->boneCB)
	{
		auto& d3d = SystemLocator::Get<D3D11System>();
		this->boneCB = std::make_unique<DynamicConstantBuffer<BoneBuffer>>();
		this->boneCB->Create(d3d.GetDevice());
	}

	this->meshComponent = this->Owner()->GetComponent<MeshComponent>();
	if (!this->meshComponent)
	{
		this->meshComponent = this->Owner()->AddComponent<MeshComponent>();
	}

	this->boneBuffer.boneCount = 0;

	for (size_t i = 0; i < ShaderCommon::MaxBones; ++i)
	{
		this->boneBuffer.boneMatrices[i] = DX::Matrix4x4::Identity;
	}
}

void AnimationComponent::Dispose()
{
	this->animator.reset();
	this->meshComponent = nullptr;

	this->skeletonCache = nullptr;
	this->isSkeletonCached = false;

	this->boneCB.reset();
}

void AnimationComponent::SetSkeletonCache(const Graphics::Import::SkeletonCache* _cache)
{
	this->skeletonCache = _cache;

	if (!this->skeletonCache)
	{
		this->isSkeletonCached = false;
		return;
	}

	// 読み込み時の検証ログのみ許可
	if (Graphics::Debug::Config::IsImportDumpEnabled())
	{
		Graphics::Debug::Output::DumpSkeletonImportCheck(
			Graphics::Debug::Config::SkeletonImportDumpPath,
			*this->skeletonCache,
			5);

		Graphics::Debug::Output::DumpSkeletonOrderCheck(
			Graphics::Debug::Config::SkeletonOrderDumpPath,
			*this->skeletonCache);
	}

	this->isSkeletonCached = true;

	// スケルトンに合わせてPose側の配列サイズを整える
	// Animator未設定時の表示もここで安定させる
	this->currentPose.ResetForSkeleton(*this->skeletonCache);
}

void AnimationComponent::SetAnimator(std::unique_ptr<IAnimator> _animator)
{
	this->animator = std::move(_animator);

	// Animatorがある場合、Poseの更新は Update で行う
	// ここでは何もしない（SetSkeletonCache が先/後 どちらでも安全にしたい）
}

void AnimationComponent::Update(float _deltaTime)
{
	if (!this->isSkeletonCached)
	{
		OutputDebugStringA("[AnimComp] Update skip: isSkeletonCached=false\n");
		return;
	}
	if (!this->skeletonCache)
	{
		OutputDebugStringA("[AnimComp] Update skip: skeletonCache=null\n");
		return;
	}
	if (this->skeletonCache->nodes.empty())
	{
		OutputDebugStringA("[AnimComp] Update skip: nodes empty\n");
		return;
	}

	//{
	//	char buf[256];
	//	sprintf_s(buf,
	//		"[AnimComp] this=%p owner=%p dt=%.6f animator=%p nodes=%zu\n",
	//		this,
	//		this->Owner(),
	//		_deltaTime,
	//		this->animator.get(),
	//		this->skeletonCache->nodes.size());
	//	OutputDebugStringA(buf);
	//}

	//-----------------------------------------------------------------------------
	// LocalPose -> Pose（global/skin/cpuBoneMatrices）更新
	//-----------------------------------------------------------------------------
	if (this->animator)
	{
		this->animator->Update(_deltaTime);

		{
			char buf[256];
			sprintf_s(buf, "[AnimAfterUpdate] owner=%p this=%p animator=%p nrm=%.3f fin=%d\n",
				this->Owner(),
				this,
				this->animator.get(),
				this->animator->GetNormalizedTime(),
				this->animator->IsFinished() ? 1 : 0);
			OutputDebugStringA(buf);
		}

		this->currentPose.BuildFromLocalPose(*this->skeletonCache, this->animator->GetLocalPose());
	}
	else
	{
		this->currentPose.ResetForSkeleton(*this->skeletonCache);
	}
	//-----------------------------------------------------------------------------
	// BoneBuffer更新（Pose -> GPU用配列）
	//-----------------------------------------------------------------------------
	this->UpdateBoneBufferFromPose();

	//-----------------------------------------------------------------------------
	// 定数バッファ更新
	//-----------------------------------------------------------------------------
	if (this->boneCB)
	{
		auto& d3d = SystemLocator::Get<D3D11System>();
		this->boneCB->Update(d3d.GetContext(), this->boneBuffer);
	}
}

void AnimationComponent::Play()
{
	if (this->animator)
	{
		this->animator->Play();
	}
}

void AnimationComponent::Stop()
{
	if (this->animator)
	{
		this->animator->Stop();
	}
}

void AnimationComponent::Restart()
{
	if (this->animator)
	{
		this->animator->Restart();
	}
}

float AnimationComponent::GetNormalizedTime() const
{
	if (this->animator)
	{
		return this->animator->GetNormalizedTime();
	}
	return 0.0f;
}

Graphics::Import::AnimationClip* AnimationComponent::GetCurrentClip() const
{
	if (this->animator)
	{
		return this->animator->GetCurrentClip();
	}
	return nullptr;
}

void AnimationComponent::UpdateBoneBufferFromPose()
{
	if (!this->skeletonCache)
	{
		this->boneBuffer.boneCount = 0;
		return;
	}

	// アップロードするボーン数を決定する
	const uint32_t actualBoneCount = static_cast<uint32_t>(this->skeletonCache->boneIndexToNodeIndex.size());
	const uint32_t uploadBoneCount = std::min<uint32_t>(actualBoneCount, static_cast<uint32_t>(ShaderCommon::MaxBones));
	this->boneBuffer.boneCount = uploadBoneCount;

	// 初期化
	for (size_t i = 0; i < ShaderCommon::MaxBones; ++i)
	{
		this->boneBuffer.boneMatrices[i] = DX::Matrix4x4::Identity;
	}

	// ボーン行列を更新
	// CPUで転置してGPUへ送る（運用固定）
	for (uint32_t i = 0; i < uploadBoneCount; ++i)
	{
		this->boneBuffer.boneMatrices[i] =
			DX::TransposeMatrix(this->currentPose.cpuBoneMatrices[static_cast<size_t>(i)]);
	}
}

//-----------------------------------------------------------------------------
// Constant buffer bind
//-----------------------------------------------------------------------------
void AnimationComponent::BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const
{
	if (!_context) { return; }
	if (!this->boneCB) { return; }

	ID3D11Buffer* buffer = this->boneCB->GetBuffer();
	if (!buffer) { return; }

	{
		char buf[256];
		sprintf_s(buf, "[BindBoneCBVS] owner=%p this=%p animator=%p boneCB=%p slot=%u\n",
			this->Owner(), this, this->animator.get(), buffer, _slot);
		OutputDebugStringA(buf);
	}

	_context->VSSetConstantBuffers(_slot, 1, &buffer);
}