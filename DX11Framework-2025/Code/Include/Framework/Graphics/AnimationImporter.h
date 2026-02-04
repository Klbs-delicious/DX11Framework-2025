/** @file	AnimationImporter.h
 *  @brief  Assimpを利用したアニメーションデータ読み込み
 *  @date	2026/01/13
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/AnimationData.h"

#include <assimp/scene.h>

#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	struct SkeletonCache;

	/** @class AnimationImporter
	 *  @brief アニメーションデータをAssimpで読み込み AnimationClip に変換するクラス
	 */
	class AnimationImporter
	{
	public:
		/// @brief コンストラクタ
		AnimationImporter();

		/// @brief デストラクタ
		~AnimationImporter();

		/** @brief 先頭のアニメーションクリップを読み込む
		 *  @param _filename ファイル名
		 *  @param _outClip 出力先
		 *  @return 成功した場合 true
		 */
		bool LoadSingleClip(const std::string& _filename, AnimationClip& _outClip) const;

		/** @brief 全アニメーションクリップを読み込む
		 *  @param _filename ファイル名
		 *  @param _outClips 出力先
		 *  @return 成功した場合 true
		 */
		bool LoadClips(const std::string& _filename, std::vector<AnimationClip>& _outClips) const;

	private:
		/** @brief Assimp の aiAnimation から AnimationClip を構築する
		 *  @param _anim Assimp のアニメーションデータ
		 *  @return 構築したアニメーションクリップ
		 */
		AnimationClip BuildClipFromAssimp(const aiAnimation* _anim) const;
	};
} // namespace Graphics::Import