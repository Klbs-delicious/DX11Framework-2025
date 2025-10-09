/**@file   GameObjectManager.h
 * @date   2025/09/17
 */
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"
#include"Include/Framework/Entities/GameObject.h"
#include"Include/Framework/Event/GameObjectEvent.h"

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
	/// @brief	コンストラクタ
	GameObjectManager();

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

	/// @brief オブジェクトの一括描画
	void DrawAll();

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
	GameObject* GetFindObjectByName(const std::string& _name);

	/**	@brief	ゲームオブジェクトをタグ検索で取得する
	 *	@param	const GameTags::Tag& _tag = GameTags::Tag::None	オブジェクトのタグ名
	 *	@return GameObject*										ゲームオブジェクト
	 */
	GameObject* GetFindObjectWithTag(const GameTags::Tag& _tag);

	/**	@brief	ゲームオブジェクトをタグ検索で取得する
	 *	@param	const GameTags::Tag& _tag = GameTags::Tag::None	オブジェクトのタグ名
	 *	@return std::vector<GameObject*>						当てはまるゲームオブジェクトのリスト
	 */
	std::vector<GameObject*> GetFindObjectsWithTag(const GameTags::Tag& _tag);

	/**	@brief 登録されたゲームオブジェクトを一括削除する
	 *	@details
	 *	-	OnDestroy() によって削除キューに追加されたオブジェクトを更新後に安全に破棄する
	 *  -	マップ・リスト・所有権をすべて解除しDispose() を呼び出してリソースを解放する
	 */
	void FlushDestroyQueue();

	/**@brief GameObjectからのイベント通知を受け取る
	 * @param GameObject* _obj			通知元のGameObjectインスタンス
	 * @param GameObjectEvent _event	発生したイベント種別
	 * @detail
	 *	-	GameObject が状態変化した際に呼び出される
	 *	-	実装側では event の種類に応じて処理を分岐させる
	 */
	void OnGameObjectEvent(GameObject* _obj, GameObjectEvent _event)override;

private:
	std::list<std::unique_ptr<GameObject>> gameObjects;		///< 生成されたゲームオブジェクト

	std::deque<GameObject*> pendingInit;	///< 初期化を行うオブジェクトのキュー
	std::vector<GameObject*> updateList;	///< 更新を行うオブジェクトの配列
	std::vector<GameObject*> drawList;		///< 描画を行うオブジェクトの配列
	std::deque<GameObject*> destroyQueue;	///< 遅延破棄対象キュー

	std::unordered_map<std::string, GameObject*> nameMap;	///< 名前検索用マップ
	std::unordered_map<GameTags::Tag, GameObject*> tagMap;	///< タグ検索用マップ
};