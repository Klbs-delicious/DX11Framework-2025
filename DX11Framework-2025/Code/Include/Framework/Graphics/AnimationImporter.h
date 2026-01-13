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

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	/** @struct AnimKeyVec3
	 *  @brief アニメーションのベクトル3キー
	 */
	struct AnimKeyVec3
	{
		double time = 0.0;
		aiVector3D value{};
	};

	/** @struct AnimKeyQuat
	 *  @brief アニメーションのクォータニオンキー
	 */
	struct AnimKeyQuat
	{
		double time = 0.0;
		aiQuaternion value{};
	};

	/** @struct NodeTrack
	 *  @brief ノードごとのアニメーショントラック
	 */
	struct NodeTrack
	{
		std::string nodeName{};
		std::vector<AnimKeyVec3> positionKeys{};
		std::vector<AnimKeyQuat> rotationKeys{};
		std::vector<AnimKeyVec3> scaleKeys{};
	};

	/** @struct AnimationClip
	 *  @brief アニメーションクリップ
	 */
	struct AnimationClip
	{
		std::string name{};
		double durationTicks = 0.0;
		double ticksPerSecond = 0.0;
		std::unordered_map<std::string, NodeTrack> tracks{};
	};

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
