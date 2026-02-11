/** @file   AnimationClipManager.h
 *  @date   2026/01/13
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Core/IResourceManager.h"
#include "Include/Framework/Graphics/AnimationImporter.h"
#include "Include/Framework/Graphics/AnimationData.h"

#include <memory>
#include <string>
#include <unordered_map>

//-----------------------------------------------------------------------------
// AnimationClipManager class
//-----------------------------------------------------------------------------

/** @class AnimationClipManager
 *  @brief アニメーションクリップを読み込み・保持・再利用するマネージャー
 */
class AnimationClipManager : public IResourceManager<Graphics::Import::AnimationClip>
{
public:
	/// @brief コンストラクタ
	AnimationClipManager();

	/// @brief デストラクタ
	~AnimationClipManager() override;

	/** @brief リソースを登録する（未登録ならロードして登録する）
	 *  @param _key リソースキー
	 *  @return 登録済み、またはロードに成功したクリップ。失敗時 nullptr
	 */
	Graphics::Import::AnimationClip* Register(const std::string& _key) override;

	/** @brief リソースの登録を解除する
	 *  @param _key リソースキー
	 */
	void Unregister(const std::string& _key) override;

	/** @brief キーに対応するリソースを取得する
	 *  @param _key リソースキー
	 *  @return 見つかった場合はポインタ、見つからなかった場合は nullptr
	 */
	Graphics::Import::AnimationClip* Get(const std::string& _key) override;

	/** @brief デフォルトのクリップを取得する
	 *  @return デフォルトクリップ。未設定なら nullptr
	 */
	Graphics::Import::AnimationClip* Default() const override;

	/** @brief クリップ読み込み情報を登録する（Register の前に呼ぶ想定）
	 *  @param _key リソースキー
	 *  @param _filename アニメーションファイルパス
	 */
	void AddClipInfo(const std::string& _key, const std::string& _filename);

	/// @brief 全クリップをクリアする
	void Clear();

	/** @brief クリップにイベントテーブルを構築する
	 *  @param _key クリップキー
	 *  @param _events イベント定義配列
	 */
	void BuildEventTable(Graphics::Import::AnimationClip& _clip, const std::vector<Graphics::Import::ClipEventDef>& _defs);

private:
	Graphics::Import::AnimationImporter importer;

	std::unordered_map<std::string, std::unique_ptr<Graphics::Import::AnimationClip>> clipMap;	///< クリップ本体
	std::unordered_map<std::string, std::string> clipInfoMap;									///< key -> filename
	std::unordered_map<std::string, std::vector<Graphics::Import::ClipEventDef>> eventDefMap;	///< key -> events

	Graphics::Import::AnimationClip* defaultClip = nullptr;                                     ///< デフォルト
};
