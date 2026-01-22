/** @file	AnimationImporter.h
 *  @brief  Assimpを利用したアニメーションデータ読み込み
 *  @date	2026/01/13
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "Include/Framework/Graphics/AnimationData.h"

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
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

		/** @brief ファイルからアニメーションクリップを1つ読み込む
		 *  @param _filename ファイルパス（アニメ入りFBXなど）
		 *  @param _outClip 出力先
		 *  @return 成功時 true
		 */
		bool LoadSingleClip(const std::string& _filename, AnimationClip& _outClip) const;

		/** @brief ファイルからアニメーションクリップを読み込む
		 *  @param _filename ファイルパス（アニメ入りFBXなど）
		 *  @param _outClips 出力先（複数アニメ対応）
		 *  @return 成功時 true
		 */
		bool LoadClips(const std::string& _filename, std::vector<AnimationClip>& _outClips) const;

	private:

		/** @brief Assimp の aiAnimation から AnimationClip を構築する
		 *  @param _anim Assimp アニメーション
		 *  @return AnimationClip
		 */
		AnimationClip BuildClipFromAssimp(const aiAnimation* _anim) const;
	};
} // namespace Graphics::Import
