/**@file   GameObjectManager.h
 * @date   2025/09/17
 */
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"
#include"Include/Framework/Event/GameObjectEvent.h"

#include"Include/Framework/Entities/GameObject.h"
#include"Include/Framework/Entities/Component.h"
#include"Include/Framework/Entities/PhaseInterfaces.h"
#include"Include/Framework/Entities/Rigidbody3D.h"
#include"Include/Framework/Entities/Transform.h"

#include<memory>
#include<list>
#include<unordered_map>
#include<string>
#include <deque>

/**	@class	GameObjectManager
 *	@brief	ゲームオブジェクトの生成、更新、取得などを管理する
 *	@details	
 *  - このクラスはコピー、代入を禁止している
 *  - このクラスはGameObjectの通知を受け取ってその状態にあわせて処理を行う
 */
class GameObjectManager :private NonCopyable, public IGameObjectObserver
{
public:
	/**	@brief コンストラクタ
	 *	@param const EngineServices* _services
	 */
	GameObjectManager(const EngineServices* _services);

	/// @brief	デストラクタ
	~GameObjectManager();

	/// @brief 解放処理
	void Dispose();

	/// @brief 未初期化オブジェクトを初期化する
	void FlushInitialize();

	/**	@brief オブジェクトの一括更新
	 *	@param		float _deltaTime	デルタタイム
	 */
	void UpdateAll(float _deltaTime);

	/**	@brief オブジェクトの一括固定更新
	 *	@param		float _deltaTime	デルタタイム
	 */
	void FixedUpdateAll(float _deltaTime);

	/// @brief 全Transformのワールド行列を更新する
	void UpdateAllTransforms();

	/// @brief 一括描画
	void RenderAll();

	/**	@brief	ゲームオブジェクトの作成
	 *	@param	const std::string& _name						オブジェクトの名前
	 *	@param	const GameTags::Tag& _tag = GameTags::Tag::None	オブジェクトのタグ名
	 *	@param	const bool _isActive = true						オブジェクトの有効状態
	 *	@return GameObject*										生成したゲームオブジェクト
	 */
	GameObject* Instantiate(const std::string& _name, const GameTags::Tag& _tag = GameTags::Tag::None, const bool _isActive = true);

	/**	@brief	ゲームオブジェクトを名前検索で取得する
	 *	@param	const std::string& _name	オブジェクトの名前
	 *	@return GameObject*					ゲームオブジェクト
	 */
	[[nodiscard]] GameObject* GetFindObjectByName(const std::string& _name);

	/**	@brief	ゲームオブジェクトをタグ検索で取得する
	 *	@param	const GameTags::Tag& _tag = GameTags::Tag::None	オブジェクトのタグ名
	 *	@return std::vector<GameObject*>						当てはまるゲームオブジェクトのリスト
	 */
	[[nodiscard]] std::vector<GameObject*> GetFindObjectsWithTag(const GameTags::Tag& _tag);

	/**	@brief 登録されたゲームオブジェクトを一括削除する
	 *	@details
	 *	-	OnDestroy() によって削除キューに追加されたオブジェクトを更新後に安全に破棄する
	 *  -	マップ・リスト・所有権をすべて解除しDispose() を呼び出してリソースを解放する
	 */
	void FlushDestroyQueue();

	/**@brief GameObjectからのイベント通知を受け取る（コンテキスト版）
	 * @param GameObjectEventContext _eventContext	イベントコンテキスト情報
	 * @detail
	 *	-	GameObject が状態変化した際に呼び出される
	 *	-	実装側では eventContext.eventType の種類に応じて処理を分岐させる
	 */
	void OnGameObjectEvent(const GameObjectEventContext _eventContext) override;

	/** @brief 要素を重複なく追加する
	 *  @tparam T 追加する要素のポインタ型
	 *  @param _v 追加先のベクター
	 *  @param _p 追加するポインタ
	 */
	template<typename T>
	inline void PushUnique(std::vector<T*>& _v, T* _p)
	{
		if (std::find(_v.begin(), _v.end(), _p) == _v.end())
		{
			_v.push_back(_p);
		}
	}

	/** @brief 要素を 1 つ確実に削除する
	 *  @tparam T 削除する要素のポインタ型
	 *  @param _v 削除対象のベクター
	 *  @param _p 削除したいポインタ
	 */
	template<typename T>
	inline void EraseOne(std::vector<T*>& _v, T* _p)
	{
		_v.erase(std::remove(_v.begin(), _v.end(), _p), _v.end());
	}

private:
	void RegisterComponentToPhases(Component* _component);
	void UnregisterComponentFromPhases(Component* _component);

private:
	const EngineServices* services;		///< リソース関連の参照

	// オブジェクト関連
	std::list<std::unique_ptr<GameObject>> gameObjects;		///< 生成されたゲームオブジェクト
	std::deque<GameObject*> destroyQueue;					///< 遅延破棄対象キュー

	// 外部コンポーネント管理用配列（オブジェクトIDと紐づけ）
	std::deque<Component*> pendingInits;			///< 初期化を行うコンポーネントのキュー
	std::vector<IUpdatable*> updates;				///< 更新を持つコンポーネントの配列
	std::vector <IFixedUpdatable*> fixedUpdates;	///< 固定更新を持つオブジェクトの配列

	// 内部コンポーネント管理用配列
	std::vector<IDrawable*> renderes;							///< 描画を持つコンポーネントの配列
	std::vector<Framework::Physics::Rigidbody3D*> rigidbodies;	///< 物理コンポーネントの配列
	std::vector<Transform*> transforms;							///< Transformコンポーネントの配列

	// 検索用マップ
	std::unordered_map<std::string, GameObject*> nameMap;				///< 名前検索用マップ
	std::unordered_map<GameTags::Tag, std::vector<GameObject*>> tagMap;	///< タグ検索用マップ

};