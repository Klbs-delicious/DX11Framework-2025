/** @file   TimeScaleGroup.h
 *  @brief  複数オブジェクトをまとめて時間スケール制御するためのコンポーネント
 *  @date   2025/11/16
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/TimeScaleComponent.h"

#include <string>
#include <vector>
#include <unordered_map>

 /// @struct 複数オブジェクトをまとめて時間スケール制御するための情報構造体
struct ScaleGroupInfo
{
	std::string groupName;						///< グループ識別用の名前
	float timeScale;							///< グループ全体に適用する時間倍率
	std::vector<TimeScaleComponent*> members;	///< グループに所属するTimeScaleComponentのリスト
};

 /** @class TimeScaleGroup
  *  @brief 複数オブジェクトをまとめて時間スケール制御するためのグループコンポーネント
  *  @details - Componentを継承し、同一グループ名を持つTimeScaleComponentのスケールを一括管理する
  */
class TimeScaleGroup : public Component
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	TimeScaleGroup(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~TimeScaleGroup() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief グループにメンバーを追加する（必要であればグループを作る）
	 *  @param _groupName グループ名
	 *  @param _timeScaleComponent 追加するコンポーネント
	 *  @return メンバーの追加に成功したら true
	 */
	bool AddGroup(const std::string& _groupName, TimeScaleComponent* _timeScaleComponent);

	/** @brief グループを新規作成する
	 *  @param _groupName グループ名
	 *  @return 追加に成功したら true。すでに存在する場合は false。
	 */
	bool AddGroup(const std::string& _groupName);
	
	/** @brief 時間スケールを設定する
	 *  @param const std::string& std::string& _name グループ名
	 */
	void SetGroupScale(const std::string& _name, float _scale);

	/**@brief 時間スケールを取得する
	 * @param const std::string& _groupName グループ名
	 * @return float グループの時間スケール値
	 */
	[[nodiscard]] float GetGroupScale(const std::string& _groupName) const;

	/// @brief 全グループ情報をクリアする
	void ClearGroups();

	/** @brief 指定したグループ情報を削除する
	 *  @param const std::string& _groupName グループ名
	 */
	void RemoveGroup(const std::string& _groupName);
private:
	std::unordered_map<std::string, ScaleGroupInfo> scaleGroups;	///< グループ名をキーとしたスケールグループ情報のマップ
};
