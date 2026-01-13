/** @file   AnimationComponent.cpp
 *  @brief  アニメーション更新専用コンポーネント
 *  @date   2026/01/13
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Graphics/ModelManager.h"

#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/SystemLocator.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>

#include <DirectXMath.h>

using namespace DirectX;

//-----------------------------------------------------------------------------
// Internal helpers
//-----------------------------------------------------------------------------
namespace
{
	static DX::Matrix4x4 ToDxMatrix(const aiMatrix4x4& _m)
	{
		DX::Matrix4x4 out;
		out._11 = _m.a1; out._12 = _m.a2; out._13 = _m.a3; out._14 = _m.a4;
		out._21 = _m.b1; out._22 = _m.b2; out._23 = _m.b3; out._24 = _m.b4;
		out._31 = _m.c1; out._32 = _m.c2; out._33 = _m.c3; out._34 = _m.c4;
		out._41 = _m.d1; out._42 = _m.d2; out._43 = _m.d3; out._44 = _m.d4;
		return out;
	}

	static DX::Matrix4x4 ToDxMatrix(const XMMATRIX& _m)
	{
		XMFLOAT4X4 f;
		XMStoreFloat4x4(&f, _m);

		DX::Matrix4x4 out;
		out._11 = f._11; out._12 = f._12; out._13 = f._13; out._14 = f._14;
		out._21 = f._21; out._22 = f._22; out._23 = f._23; out._24 = f._24;
		out._31 = f._31; out._32 = f._32; out._33 = f._33; out._34 = f._34;
		out._41 = f._41; out._42 = f._42; out._43 = f._43; out._44 = f._44;
		return out;
	}

	static XMMATRIX ToXmMatrix(const DX::Matrix4x4& _m)
	{
		XMFLOAT4X4 f;
		f._11 = _m._11; f._12 = _m._12; f._13 = _m._13; f._14 = _m._14;
		f._21 = _m._21; f._22 = _m._22; f._23 = _m._23; f._24 = _m._24;
		f._31 = _m._31; f._32 = _m._32; f._33 = _m._33; f._34 = _m._34;
		f._41 = _m._41; f._42 = _m._42; f._43 = _m._43; f._44 = _m._44;
		return XMLoadFloat4x4(&f);
	}

	static float Lerp(float _a, float _b, float _t)
	{
		return _a + (_b - _a) * _t;
	}

	static aiVector3D Lerp(const aiVector3D& _a, const aiVector3D& _b, float _t)
	{
		return aiVector3D(
			Lerp(_a.x, _b.x, _t),
			Lerp(_a.y, _b.y, _t),
			Lerp(_a.z, _b.z, _t)
		);
	}

	static aiQuaternion Slerp(const aiQuaternion& _a, const aiQuaternion& _b, float _t)
	{
		aiQuaternion out;
		aiQuaternion::Interpolate(out, _a, _b, _t);
		out.Normalize();
		return out;
	}

	static size_t FindKeyIndexVec3(const std::vector<Graphics::Import::AnimKeyVec3>& _keys, double _timeTicks)
	{
		if (_keys.size() <= 1) { return 0; }

		for (size_t i = 0; i + 1 < _keys.size(); i++)
		{
			if (_timeTicks < _keys[i + 1].time)
			{
				return i;
			}
		}
		return _keys.size() - 2;
	}

	static size_t FindKeyIndexQuat(const std::vector<Graphics::Import::AnimKeyQuat>& _keys, double _timeTicks)
	{
		if (_keys.size() <= 1) { return 0; }

		for (size_t i = 0; i + 1 < _keys.size(); i++)
		{
			if (_timeTicks < _keys[i + 1].time)
			{
				return i;
			}
		}
		return _keys.size() - 2;
	}

	static aiVector3D SampleVec3(const std::vector<Graphics::Import::AnimKeyVec3>& _keys, double _timeTicks, const aiVector3D& _fallback)
	{
		if (_keys.empty()) { return _fallback; }
		if (_keys.size() == 1) { return _keys[0].value; }

		const size_t idx = FindKeyIndexVec3(_keys, _timeTicks);
		const auto& k0 = _keys[idx];
		const auto& k1 = _keys[idx + 1];

		const double dt = (k1.time - k0.time);
		float t = 0.0f;
		if (dt > 0.0)
		{
			t = static_cast<float>((_timeTicks - k0.time) / dt);
			t = std::clamp(t, 0.0f, 1.0f);
		}
		return Lerp(k0.value, k1.value, t);
	}

	static aiQuaternion SampleQuat(const std::vector<Graphics::Import::AnimKeyQuat>& _keys, double _timeTicks, const aiQuaternion& _fallback)
	{
		if (_keys.empty()) { return _fallback; }
		if (_keys.size() == 1) { return _keys[0].value; }

		const size_t idx = FindKeyIndexQuat(_keys, _timeTicks);
		const auto& k0 = _keys[idx];
		const auto& k1 = _keys[idx + 1];

		const double dt = (k1.time - k0.time);
		float t = 0.0f;
		if (dt > 0.0)
		{
			t = static_cast<float>((_timeTicks - k0.time) / dt);
			t = std::clamp(t, 0.0f, 1.0f);
		}
		return Slerp(k0.value, k1.value, t);
	}

	static XMMATRIX ComposeTRS(const aiVector3D& _t, const aiQuaternion& _r, const aiVector3D& _s)
	{
		const XMVECTOR tv = XMVectorSet(_t.x, _t.y, _t.z, 1.0f);
		const XMVECTOR sv = XMVectorSet(_s.x, _s.y, _s.z, 0.0f);

		const XMVECTOR qv = XMVectorSet(_r.x, _r.y, _r.z, _r.w);

		const XMMATRIX T = XMMatrixTranslationFromVector(tv);
		const XMMATRIX R = XMMatrixRotationQuaternion(qv);
		const XMMATRIX S = XMMatrixScalingFromVector(sv);

		return S * R * T;
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent
//-----------------------------------------------------------------------------

AnimationComponent::AnimationComponent(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	currentClip(nullptr),
	meshComponent(nullptr),
	modelData(nullptr),
	isPlaying(false),
	currentTime(0.0),
	playbackSpeed(1.0f),
	skinningBuffer(nullptr)
{
}

void AnimationComponent::Initialize()
{
	this->meshComponent = this->Owner()->GetComponent<MeshComponent>();
	if (!this->meshComponent)
	{
		std::cerr << "[Error] AnimationComponent::Initialize: MeshComponent not found in GameObject: "
			<< this->Owner()->GetName() << std::endl;
		return;
	}

	if (!this->modelData)
	{
		auto services = this->Owner()->Services();
		if (!services || !services->models)
		{
			std::cerr << "[Error] AnimationComponent::Initialize: ModelManager not available." << std::endl;
			return;
		}

		auto defaultModelEntry = services->models->Default();
		if (!defaultModelEntry)
		{
			std::cerr << "[Error] AnimationComponent::Initialize: Default ModelEntry not found." << std::endl;
			return;
		}

		this->modelData = defaultModelEntry->GetModelData();
		if (!this->modelData)
		{
			std::cerr << "[Error] AnimationComponent::Initialize: Default ModelData not found." << std::endl;
			return;
		}
	}

	this->isSkeletonCached = false;

	this->nodeNames.clear();
	this->nodeIndexMap.clear();
	this->parentIndex.clear();

	std::function<void(const Utils::TreeNode<Graphics::Import::BoneNode>&, int)> build;
	build = [&](const Utils::TreeNode<Graphics::Import::BoneNode>& _node, int _parent)
		{
			const std::string& name = _node.nodedata.name;

			const int myIndex = static_cast<int>(this->nodeNames.size());
			this->nodeNames.emplace_back(name);
			this->nodeIndexMap[name] = myIndex;
			this->parentIndex.emplace_back(_parent);

			for (const auto& c : _node.children)
			{
				build(*c, myIndex);
			}
		};

	build(this->modelData->boneTree, -1);

	const size_t boneCount = this->nodeNames.size();
	if (boneCount == 0)
	{
		std::cerr << "[Error] AnimationComponent::Initialize: boneTree is empty." << std::endl;
		return;
	}

	this->localBindPose.assign(boneCount, DX::Matrix4x4::Identity);
	this->offsetMatrices.assign(boneCount, DX::Matrix4x4::Identity);

	this->localPose.assign(boneCount, DX::Matrix4x4::Identity);
	this->globalPose.assign(boneCount, DX::Matrix4x4::Identity);
	this->skinMatrices.assign(boneCount, DX::Matrix4x4::Identity);

	for (size_t i = 0; i < boneCount; i++)
	{
		const std::string& name = this->nodeNames[i];

		auto it = this->modelData->boneDictionary.find(name);
		if (it != this->modelData->boneDictionary.end())
		{
			this->localBindPose[i] = ToDxMatrix(it->second.localBind);
			this->offsetMatrices[i] = ToDxMatrix(it->second.offsetMatrix);
		}
	}

	{
		const std::string& rootName = this->nodeNames[0];

		auto it = this->modelData->boneDictionary.find(rootName);
		if (it != this->modelData->boneDictionary.end())
		{
			DX::Matrix4x4 root = ToDxMatrix(it->second.globalBind);
			root.Invert(this->inverseRootBind);
		}
		else
		{
			this->inverseRootBind = DX::Matrix4x4::Identity;
		}
	}

	this->bindTrs.assign(boneCount, BindTRS{});

	for (size_t i = 0; i < boneCount; i++)
	{
		const XMMATRIX m = ToXmMatrix(this->localBindPose[i]);

		XMVECTOR s, r, t;
		if (XMMatrixDecompose(&s, &r, &t, m))
		{
			XMFLOAT3 sf; XMStoreFloat3(&sf, s);
			XMFLOAT4 rf; XMStoreFloat4(&rf, r);
			XMFLOAT3 tf; XMStoreFloat3(&tf, t);

			this->bindTrs[i].t = aiVector3D(tf.x, tf.y, tf.z);
			this->bindTrs[i].s = aiVector3D(sf.x, sf.y, sf.z);

			this->bindTrs[i].r = aiQuaternion(rf.w, rf.x, rf.y, rf.z);
		}
		else
		{
			this->bindTrs[i].t = aiVector3D(0.0f, 0.0f, 0.0f);
			this->bindTrs[i].s = aiVector3D(1.0f, 1.0f, 1.0f);
			this->bindTrs[i].r = aiQuaternion(1.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	// スキニング用定数バッファを作成する（失敗チェック付き）
	this->skinningBuffer = std::make_unique<DynamicConstantBuffer<BoneBuffer>>();

	ID3D11Device* device = SystemLocator::Get<D3D11System>().GetDevice();
	if (!device)
	{
		std::cerr << "[Error] AnimationComponent::Initialize: device is null." << std::endl;
		this->skinningBuffer.reset();
		return;
	}

	if (!this->skinningBuffer->Create(device))
	{
		std::cerr << "[Error] AnimationComponent::Initialize: skinningBuffer Create failed." << std::endl;
		this->skinningBuffer.reset();
		return;
	}

	this->isSkeletonCached = true;

	this->isPlaying = false;
	this->currentTime = 0.0;
	this->playbackSpeed = 1.0f;
}

void AnimationComponent::Dispose()
{
	this->currentClip = nullptr;
	this->meshComponent = nullptr;
	this->modelData = nullptr;

	this->isPlaying = false;
	this->currentTime = 0.0;
	this->playbackSpeed = 1.0f;

	this->isSkeletonCached = false;

	this->nodeNames.clear();
	this->nodeIndexMap.clear();
	this->parentIndex.clear();

	this->bindTrs.clear();

	this->localPose.clear();
	this->globalPose.clear();
	this->skinMatrices.clear();

	this->localBindPose.clear();
	this->offsetMatrices.clear();

	this->inverseRootBind = DX::Matrix4x4::Identity;

	this->skinningBuffer.reset();
}

void AnimationComponent::SetAnimationClip(Graphics::Import::AnimationClip* _clip)
{
	this->currentClip = _clip;
	this->currentTime = 0.0;
}

void AnimationComponent::Play()
{
	if (!this->currentClip) { return; }
	if (!this->modelData) { return; }
	if (!this->isSkeletonCached) { return; }

	this->isPlaying = true;
}

void AnimationComponent::Stop()
{
	this->isPlaying = false;
}

const std::string& AnimationComponent::GetCurrentAnimationName() const
{
	static const std::string empty{};
	return this->currentClip ? this->currentClip->name : empty;
}

void AnimationComponent::FixedUpdate(float _deltaTime)
{
	if (!this->isPlaying) { return; }
	if (!this->currentClip) { return; }
	if (!this->modelData) { return; }
	if (!this->isSkeletonCached) { return; }

	const double dt = static_cast<double>(_deltaTime) * static_cast<double>(this->playbackSpeed);
	this->currentTime += dt;

	const double tps = (this->currentClip->ticksPerSecond > 0.0) ? this->currentClip->ticksPerSecond : 30.0;
	const double durationTicks = this->currentClip->durationTicks;
	if (durationTicks <= 0.0) { return; }

	const double timeTicks = std::fmod(this->currentTime * tps, durationTicks);

	const size_t nodeCount = this->nodeNames.size();
	if (nodeCount == 0) { return; }

	for (size_t i = 0; i < nodeCount; i++)
	{
		const std::string& nodeName = this->nodeNames[i];

		const aiVector3D fallbackT = this->bindTrs[i].t;
		const aiVector3D fallbackS = this->bindTrs[i].s;
		const aiQuaternion fallbackR = this->bindTrs[i].r;

		aiVector3D t = fallbackT;
		aiQuaternion r = fallbackR;
		aiVector3D s = fallbackS;

		auto trackIt = this->currentClip->tracks.find(nodeName);
		if (trackIt != this->currentClip->tracks.end())
		{
			const Graphics::Import::NodeTrack& tr = trackIt->second;

			t = SampleVec3(tr.positionKeys, timeTicks, fallbackT);
			r = SampleQuat(tr.rotationKeys, timeTicks, fallbackR);
			s = SampleVec3(tr.scaleKeys, timeTicks, fallbackS);
		}

		const XMMATRIX local = ComposeTRS(t, r, s);
		this->localPose[i] = ToDxMatrix(local);
	}

	for (size_t i = 0; i < nodeCount; i++)
	{
		const int p = this->parentIndex[i];
		const XMMATRIX localM = ToXmMatrix(this->localPose[i]);

		if (p < 0)
		{
			this->globalPose[i] = this->localPose[i];
		}
		else
		{
			const XMMATRIX parentM = ToXmMatrix(this->globalPose[static_cast<size_t>(p)]);
			this->globalPose[i] = ToDxMatrix(parentM * localM);
		}
	}

	for (size_t i = 0; i < nodeCount; i++)
	{
		const XMMATRIX g = ToXmMatrix(this->globalPose[i]);
		const XMMATRIX off = ToXmMatrix(this->offsetMatrices[i]);
		const XMMATRIX rootInv = ToXmMatrix(this->inverseRootBind);

		const XMMATRIX skin = rootInv * g * off;
		this->skinMatrices[i] = ToDxMatrix(skin);
	}

	// --- GPU へ送る ---
	if (!this->skinningBuffer)
	{
		std::cerr << "[Error] AnimationComponent::FixedUpdate: skinningBuffer is null." << std::endl;
		return;
	}

	ID3D11DeviceContext* context = SystemLocator::Get<D3D11System>().GetContext();
	if (!context)
	{
		std::cerr << "[Error] AnimationComponent::FixedUpdate: context is null." << std::endl;
		return;
	}

	const size_t skinCount = std::min(this->skinMatrices.size(), MaxBones);
	this->boneBuffer.boneCount = static_cast<UINT>(skinCount);

	for (size_t i = 0; i < skinCount; i++)
	{
		this->boneBuffer.skin[i] = this->skinMatrices[i];
	}
	for (size_t i = skinCount; i < MaxBones; i++)
	{
		this->boneBuffer.skin[i] = DX::Matrix4x4::Identity;
	}

	this->skinningBuffer->Update(context, this->boneBuffer);
}

void AnimationComponent::BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const
{
	if (!_context) { return; }
	if (!this->skinningBuffer) { return; }

	this->skinningBuffer->BindVS(_context, _slot);
}