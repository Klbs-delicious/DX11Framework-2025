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
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <unordered_set>

#include <DirectXMath.h>

using namespace DirectX;

//-----------------------------------------------------------------------------
// Internal helpers
//-----------------------------------------------------------------------------
namespace
{
	static DX::Matrix4x4 ToDxMatrix(const aiMatrix4x4& _m)
	{
		// Assimp matrices are converted to row-vector layout at import time.
		// Keep the values as-is to avoid double-transpose.
		const aiMatrix4x4& mt = _m;

		DX::Matrix4x4 out;
		out._11 = mt.a1; out._12 = mt.a2; out._13 = mt.a3; out._14 = mt.a4;
		out._21 = mt.b1; out._22 = mt.b2; out._23 = mt.b3; out._24 = mt.b4;
		out._31 = mt.c1; out._32 = mt.c2; out._33 = mt.c3; out._34 = mt.c4;
		out._41 = mt.d1; out._42 = mt.d2; out._43 = mt.d3; out._44 = mt.d4;
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
	: Component(_owner, _isActive)
	, currentClip(nullptr)
	, meshComponent(nullptr)
	, modelData(nullptr)
	, isPlaying(false)
	, currentTime(0.0)
	, playbackSpeed(1.0f)
	, isSkeletonCached(false)
	, transposeBoneMatricesOnUpload(true)
	, boneBuffer{}
	, boneCB(nullptr)
{
}

void AnimationComponent::Initialize()
{
	this->meshComponent = this->Owner()->GetComponent<MeshComponent>();

	auto& d3d = SystemLocator::Get<D3D11System>();
	this->boneCB = std::make_unique<DynamicConstantBuffer<BoneBuffer>>();
	this->boneCB->Create(d3d.GetDevice());

	// 初期状態はスキニング無効
	this->boneBuffer.boneCount = 0;
	this->boneCB->Update(d3d.GetContext(), this->boneBuffer);

	// スケルトン情報をキャッシュ
	this->isSkeletonCached = false;
	this->nodeNames.clear();
	this->nodeIndexMap.clear();
	this->parentIndex.clear();

	// vertex 側の boneIndex は `Bone::index` を参照するため、ここで
	// 「boneName -> Bone::index」と「nodeTreeIndex -> Bone::index」を作る
	this->boneIndexMap.clear();

	if (this->modelData)
	{
		std::function<void(const Utils::TreeNode<Graphics::Import::BoneNode>&, int)> build;
		build = [&](const Utils::TreeNode<Graphics::Import::BoneNode>& _node, int _parent)
			{
				const std::string& name = _node.nodedata.name;
				const int myIndex = static_cast<int>(this->nodeNames.size());
				this->nodeNames.emplace_back(name);
				this->nodeIndexMap[name] = myIndex;
				this->parentIndex.emplace_back(_parent);

				// boneDictionary に index がある場合のみ登録
				auto it = this->modelData->boneDictionary.find(name);
				if (it != this->modelData->boneDictionary.end() && it->second.index >= 0)
				{
					this->boneIndexMap.emplace_back(it->second.index);
				}
				else
				{
					this->boneIndexMap.emplace_back(-1);
				}

				for (const auto& c : _node.children)
				{
					build(*c, myIndex);
				}
			};

		build(this->modelData->boneTree, -1);

		const size_t boneCount = this->nodeNames.size();
		if (boneCount > 0 && boneCount <= BoneBuffer::MaxBones)
		{
			this->localBindPose.assign(boneCount, DX::Matrix4x4::Identity);
			this->offsetMatrices.assign(boneCount, DX::Matrix4x4::Identity);
			this->bindTrs.assign(boneCount, BindTRS{});

			this->localPose.assign(boneCount, DX::Matrix4x4::Identity);
			this->globalPose.assign(boneCount, DX::Matrix4x4::Identity);
			this->skinMatrices.assign(boneCount, DX::Matrix4x4::Identity);

			// bind/offset を辞書から埋める
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

			// root の globalBind 逆行列
			this->inverseRootBind = DX::Matrix4x4::Identity;
			{
				const std::string& rootName = this->nodeNames[0];
				auto it = this->modelData->boneDictionary.find(rootName);
				if (it != this->modelData->boneDictionary.end())
				{
					DX::Matrix4x4 root = ToDxMatrix(it->second.globalBind);
					root.Invert(this->inverseRootBind);
				}
			}

			// bind TRS (fallback) を作る
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

			this->isSkeletonCached = true;
		}
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
}

void AnimationComponent::FixedUpdate(float _deltaTime)
{
	if (!this->isPlaying) { return; }
	if (!this->currentClip) { return; }
	if (!this->modelData) { return; }
	if (!this->isSkeletonCached) { return; }
	if (!this->boneCB) { return; }

	// --------------------------------------------
	// Debug: Track name resolution check (once per clip)
	// --------------------------------------------
	{
		static const Graphics::Import::AnimationClip* s_lastClip = nullptr;
		if (s_lastClip != this->currentClip)
		{
			s_lastClip = this->currentClip;

			size_t matchCount = 0;
			size_t missingCount = 0;
			std::vector<std::string> missingNames;
			missingNames.reserve(16);

			for (const auto& [trackName, tr] : this->currentClip->tracks)
			{
				(void)tr;
				if (this->nodeIndexMap.find(trackName) != this->nodeIndexMap.end())
				{
					matchCount++;
				}
				else
				{
					missingCount++;
					if (missingNames.size() < 20)
					{
						missingNames.emplace_back(trackName);
					}
				}
			}

			std::cout << "[AnimationComponent][Debug] ClipTrackBindingCheck: clip='"
				<< this->currentClip->name
				<< "' tracks=" << this->currentClip->tracks.size()
				<< " nodeNames=" << this->nodeNames.size()
				<< " matched=" << matchCount
				<< " missing=" << missingCount
				<< "\n";

			if (!missingNames.empty())
			{
				std::cout << "[AnimationComponent][Debug] Missing track names (first " << missingNames.size() << "):";
				for (const auto& n : missingNames)
				{
					std::cout << " '" << n << "'";
				}
				std::cout << "\n";
			}
		}
	}

	// 時間を進める
	this->currentTime += static_cast<double>(_deltaTime) * static_cast<double>(this->playbackSpeed);

	const double tps = (this->currentClip->ticksPerSecond > 0.0) ? this->currentClip->ticksPerSecond : 30.0;
	const double durationTicks = this->currentClip->durationTicks;
	if (durationTicks <= 0.0) { return; }

	// ループ
	const double timeTicks = std::fmod(this->currentTime * tps, durationTicks);

	const size_t nodeCount = this->nodeNames.size();
	if (nodeCount == 0) { return; }
	if (this->boneIndexMap.size() != nodeCount) { return; }
	if (this->bindTrs.size() != nodeCount) { return; }
	if (this->localPose.size() != nodeCount) { return; }
	if (this->globalPose.size() != nodeCount) { return; }
	if (this->offsetMatrices.size() != nodeCount) { return; }
	if (this->skinMatrices.size() != nodeCount) { return; }

	// ローカル姿勢
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

		this->localPose[i] = ToDxMatrix(ComposeTRS(t, r, s));
	}

	// グローバル姿勢
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
			// row-vector: v * (parent * local) == (v * parent) * local
			this->globalPose[i] = ToDxMatrix(parentM * localM);
		}
	}

	// 最終スキン行列（nodeTreeIndex 優先で保持）
	for (size_t i = 0; i < nodeCount; i++)
	{
		const XMMATRIX g = ToXmMatrix(this->globalPose[i]);
		const XMMATRIX off = ToXmMatrix(this->offsetMatrices[i]);

		// Standard skinning: bring vertex from mesh space to bone local (inverse bind), then to animated global.
		// With row-vector shader (mul(v, M)), the multiplication order is reversed.
		// v' = v * (off * g)
		const XMMATRIX skin = off * g;
		this->skinMatrices[i] = ToDxMatrix(skin);
	}

	// GPUへ送る: vertex の boneIndex (= Bone::index) に合わせて詰める
	BoneBuffer upload{};
	for (uint32_t i = 0; i < BoneBuffer::MaxBones; ++i)
	{
		upload.boneMatrices[i] = DX::Matrix4x4::Identity;
	}

	uint32_t maxBoneIndexPlus1 = 0;
	for (size_t nodeIdx = 0; nodeIdx < nodeCount && nodeIdx < BoneBuffer::MaxBones; ++nodeIdx)
	{
		const int boneIdx = (nodeIdx < this->boneIndexMap.size()) ? this->boneIndexMap[nodeIdx] : -1;
		if (boneIdx < 0) { continue; }
		if (boneIdx >= static_cast<int>(BoneBuffer::MaxBones)) { continue; }

		upload.boneMatrices[static_cast<uint32_t>(boneIdx)] = this->skinMatrices[nodeIdx];
		maxBoneIndexPlus1 = std::max(maxBoneIndexPlus1, static_cast<uint32_t>(boneIdx) + 1u);
	}

	upload.boneCount = maxBoneIndexPlus1;
	if (upload.boneCount == 0 && nodeCount > 0)
	{
		// 0 だと VS で disableSkin 判定に入りやすいので最低1
		upload.boneCount = 1;
	}

	if (this->transposeBoneMatricesOnUpload)
	{
		for (uint32_t i = 0; i < upload.boneCount; ++i)
		{
			upload.boneMatrices[i] = upload.boneMatrices[i].Transpose();
		}
	}

	auto& d3d = SystemLocator::Get<D3D11System>();
	this->boneCB->Update(d3d.GetContext(), upload);
}
