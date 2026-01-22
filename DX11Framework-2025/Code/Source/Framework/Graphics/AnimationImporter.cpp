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
		return 30.0;
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

	//-------------------------------------------------
	// Assimp の aiAnimation から AnimationClip を構築する
	//-------------------------------------------------
	AnimationClip AnimationImporter::BuildClipFromAssimp(const aiAnimation* _anim) const
	{
		AnimationClip clip{};
		if (!_anim)
		{
			return clip;
		}

		// クリップ基本情報
		clip.name = (_anim->mName.length > 0) ? _anim->mName.C_Str() : "";
		clip.durationTicks = _anim->mDuration;

		// ticksPerSecond は 0 の場合があるため補正する
		clip.ticksPerSecond = FixTicksPerSecond(_anim->mTicksPerSecond);

		clip.tracks.clear();
		clip.tracks.reserve(_anim->mNumChannels);

		// チャンネル（ノードごとのトラック）を構築
		for (unsigned int c = 0; c < _anim->mNumChannels; c++)
		{
			const aiNodeAnim* ch = _anim->mChannels[c];
			if (!ch)
			{
				continue;
			}

			NodeTrack track{};
			track.nodeName = ch->mNodeName.C_Str();

			// 位置キーをコピー
			CopyPositionKeys(ch, track.positionKeys);

			// 回転キーをコピー
			CopyRotationKeys(ch, track.rotationKeys);

			// スケールキーをコピー
			CopyScaleKeys(ch, track.scaleKeys);

			// ノード名で辞書に登録（同名が来た場合は後勝ち）
			clip.tracks[track.nodeName] = std::move(track);
		}

		return clip;
	}

	AnimationImporter::AnimationImporter()
	{
	}

	AnimationImporter::~AnimationImporter()
	{
	}

	bool AnimationImporter::LoadSingleClip(const std::string& _filename, AnimationClip& _outClip) const
	{
		_outClip = AnimationClip{};
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			_filename,
			aiProcess_ConvertToLeftHanded |
			aiProcess_PopulateArmatureData
		);

		if (!scene)
		{
			// 読み込み失敗
			return false;
		}

		if (scene->mNumAnimations == 0 || !scene->mAnimations)
		{
			// アニメーションが存在しない
			return false;
		}

		const aiAnimation* anim = scene->mAnimations[0];
		if (!anim)
		{
			// アニメーションが存在しない
			return false;
		}

		// 1つ目のアニメーションを変換して返す
		_outClip = BuildClipFromAssimp(anim);
		return true;
	}

	bool AnimationImporter::LoadClips(const std::string& _filename, std::vector<AnimationClip>& _outClips) const
	{
		_outClips.clear();
		Assimp::Importer importer;

		// アニメーションは「ノード階層 + キー」が読めれば十分
		const aiScene* scene = importer.ReadFile(
			_filename,
			aiProcess_ConvertToLeftHanded |
			aiProcess_PopulateArmatureData
		);

		if (!scene)
		{
			// 読み込み失敗
			return false;
		}

		if (scene->mNumAnimations == 0 || !scene->mAnimations)
		{
			// アニメーションが存在しない
			return false;
		}

		// アニメーションをすべて変換して返す
		_outClips.reserve(scene->mNumAnimations);
		for (unsigned int i = 0; i < scene->mNumAnimations; i++)
		{
			const aiAnimation* anim = scene->mAnimations[i];
			if (!anim)
			{
				// アニメーションが存在しない
				continue;
			}

			// 変換して追加する
			_outClips.push_back(BuildClipFromAssimp(anim));
		}

		return !_outClips.empty();
	}

} // namespace Graphics::Import