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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <cmath>
#include <functional>

#include <DirectXMath.h>

using namespace DirectX;

//-----------------------------------------------------------------------------
// Internal helpers
//-----------------------------------------------------------------------------
namespace
{
}

//-----------------------------------------------------------------------------
// AnimationComponent
//-----------------------------------------------------------------------------
AnimationComponent::AnimationComponent(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive)
	, currentClip(nullptr)
	, meshComponent(nullptr)
	, modelData(nullptr)
	, isPlaying(false)
	, currentTime(0.0)
	, playbackSpeed(1.0f)
	, isSkeletonCached(false)
	, boneBuffer{}
	, boneCB(nullptr)
{
}

void AnimationComponent::Initialize()
{
	//----------------------------------------------
	// コンポーネントの取得
	//----------------------------------------------
	this->meshComponent = this->Owner()->GetComponent<MeshComponent>();

	//----------------------------------------------
	// 定数バッファの作成
	//----------------------------------------------
	auto& d3d = SystemLocator::Get<D3D11System>();
	this->boneCB = std::make_unique<DynamicConstantBuffer<BoneBuffer>>();
	this->boneCB->Create(d3d.GetDevice());
	this->boneBuffer.boneCount = 0;
	this->boneCB->Update(d3d.GetContext(), this->boneBuffer);

	this->isSkeletonCached = false;

	if (this->modelData)
	{
		// モデルデータが既にセットされている場合はスケルトンキャッシュを構築する
		this->RebuildSkeletonCache();
	}
}

void AnimationComponent::Dispose()
{
	this->boneCB.reset();
	this->currentClip = nullptr;
	this->meshComponent = nullptr;
	this->modelData = nullptr;
}

void AnimationComponent::BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const
{
	if (!this->boneCB) { return; }
	this->boneCB->BindVS(_context, _slot);
}

void AnimationComponent::SetAnimationClip(Graphics::Import::AnimationClip* _clip)
{
	this->currentClip = _clip;
}

const std::string& AnimationComponent::GetCurrentAnimationName() const
{
	static const std::string kEmpty;
	return this->currentClip ? this->currentClip->name : kEmpty;
}

void AnimationComponent::Play()
{
	this->isPlaying = true;
}

void AnimationComponent::Stop()
{
	this->isPlaying = false;
}

void AnimationComponent::SetModelData(Graphics::Import::ModelData* _modelData)
{
	this->modelData = _modelData;
	this->isSkeletonCached = false;

	if (this->boneCB && this->modelData)
	{
		// モデルデータが変わったのでスケルトンキャッシュを再構築する
		this->RebuildSkeletonCache();
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent::RebuildSkeletonCache
//-----------------------------------------------------------------------------
void AnimationComponent::RebuildSkeletonCache()
{
	if(!this->modelData)
	{
		std::cerr << "[AnimationComponent] モデルデータが設定されていないため、スケルトンキャッシュを構築できません。" << std::endl;
		return;
	}
	if (this->modelData->boneDictionary.size())
	{
		std::cerr << "[AnimationComponent] モデルデータにボーン情報が存在しないため、スケルトンキャッシュを構築できません。" << std::endl;
		return;
	}
	//----------------------------------------------
	// スケルトンキャッシュの構築
	//----------------------------------------------
}

void AnimationComponent::FixedUpdate(float _deltaTime)
{

}