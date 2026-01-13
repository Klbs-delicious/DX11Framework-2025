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

		/** @brief アニメーションを読み込み AnimationClip に変換
		 *  @param _filename アニメーションファイルパス
		 *  @param _outClip 出力先アニメーションクリップ
		 *  @return 成功時 true
		 */
		bool Load(const std::string& _filename, AnimationClip& _outClip);

	private:
		/** @brief アニメーションクリップを読み込み
		 *  @param _scene Assimpシーン
		 *  @param _outClip 出力先アニメーションクリップ
		 */
		void ReadClip(const aiScene* _scene, AnimationClip& _outClip) const;
	};
} // namespace Graphics::Import
