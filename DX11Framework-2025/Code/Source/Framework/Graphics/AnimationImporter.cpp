/** @file	AnimationImporter.cpp
 *  @brief  Assimpを利用したアニメーションデータ読み込み
 *  @date	2026/01/13
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/AnimationImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <iostream>

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	/** @brief 位置キーをコピー
	 *  @param _src 
	 *  @param _dst 
	 */
	void CopyPositionKeys(const aiNodeAnim* _src, std::vector<Graphics::Import::AnimKeyVec3>& _dst)
	{
		_dst.clear();
		if (!_src || _src->mNumPositionKeys == 0) { return; }

		_dst.reserve(_src->mNumPositionKeys);
		for (unsigned int i = 0; i < _src->mNumPositionKeys; i++)
		{
			Graphics::Import::AnimKeyVec3 key{};
			key.time = _src->mPositionKeys[i].mTime;
			key.value = _src->mPositionKeys[i].mValue;
			_dst.emplace_back(key);
		}
	}

	/** @brief 回転キーをコピー
	 *  @param _src 
	 *  @param _dst 
	 */
	void CopyRotationKeys(const aiNodeAnim* _src, std::vector<Graphics::Import::AnimKeyQuat>& _dst)
	{
		_dst.clear();
		if (!_src || _src->mNumRotationKeys == 0) { return; }

		_dst.reserve(_src->mNumRotationKeys);
		for (unsigned int i = 0; i < _src->mNumRotationKeys; i++)
		{
			Graphics::Import::AnimKeyQuat key{};
			key.time = _src->mRotationKeys[i].mTime;
			key.value = _src->mRotationKeys[i].mValue;
			_dst.emplace_back(key);
		}
	}

	/** @brief スケールキーをコピー
	 *  @param _src 
	 *  @param _dst 
	 */
	void CopyScaleKeys(const aiNodeAnim* _src, std::vector<Graphics::Import::AnimKeyVec3>& _dst)
	{
		_dst.clear();
		if (!_src || _src->mNumScalingKeys == 0) { return; }

		_dst.reserve(_src->mNumScalingKeys);
		for (unsigned int i = 0; i < _src->mNumScalingKeys; i++)
		{
			Graphics::Import::AnimKeyVec3 key{};
			key.time = _src->mScalingKeys[i].mTime;
			key.value = _src->mScalingKeys[i].mValue;
			_dst.emplace_back(key);
		}
	}

	/** @brief TicksPerSecond の補正
	 *  @param _tps 
	 *  @return 
	 */
	double FixTicksPerSecond(double _tps)
	{
		if (_tps > 0.0) { return _tps; }

		// Assimp では 0 が入ることがあるため、現実的な既定値を入れる
		return 25.0;
	}
}

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	//-----------------------------------------------------------------------------
	// AnimationImporter class
	//-----------------------------------------------------------------------------

	AnimationImporter::AnimationImporter()
	{
	}

	AnimationImporter::~AnimationImporter()
	{
	}

	bool AnimationImporter::Load(const std::string& _filename, AnimationClip& _outClip)
	{
		_outClip = AnimationClip{};

		Assimp::Importer importer;

		// 読み込みオプション設定
		const unsigned int flags =
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_ConvertToLeftHanded |
			aiProcess_PopulateArmatureData;

		const aiScene* scene = importer.ReadFile(_filename, flags);
		if (!scene)
		{
			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return false;
		}

		if (scene->mNumAnimations == 0 || !scene->mAnimations[0])
		{
			std::cerr << "[Error] AnimationImporter::Load: No animations in file: " << _filename << std::endl;
			return false;
		}

		ReadClip(scene, _outClip);

		// 名前が空の場合は最低限の名前を入れる
		if (_outClip.name.empty())
		{
			_outClip.name = "Anim0";
		}

		return true;
	}

	void AnimationImporter::ReadClip(const aiScene* _scene, AnimationClip& _outClip) const
	{
		if (!_scene || _scene->mNumAnimations == 0 || !_scene->mAnimations[0]) { return; }

		// アニメーション本体を取得
		const aiAnimation* anim = _scene->mAnimations[0];

		_outClip.name = (anim->mName.length > 0) ? anim->mName.C_Str() : "";
		_outClip.durationTicks = anim->mDuration;
		_outClip.ticksPerSecond = FixTicksPerSecond(anim->mTicksPerSecond);
		_outClip.tracks.clear();

		// チャンネル（ノードトラック）を詰める
		for (unsigned int c = 0; c < anim->mNumChannels; c++)
		{
			const aiNodeAnim* channel = anim->mChannels[c];
			if (!channel) { continue; }

			const std::string nodeName = (channel->mNodeName.length > 0) ? channel->mNodeName.C_Str() : "";
			if (nodeName.empty()) { continue; }

			NodeTrack track{};
			track.nodeName = nodeName;

			CopyPositionKeys(channel, track.positionKeys);
			CopyRotationKeys(channel, track.rotationKeys);
			CopyScaleKeys(channel, track.scaleKeys);

			_outClip.tracks.emplace(nodeName, std::move(track));
		}
	}
} // namespace Graphics::Import
