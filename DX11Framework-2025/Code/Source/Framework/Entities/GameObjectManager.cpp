/** @file   GameObjectManager.cpp
*   @date   2025/09/17
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/GameObjectManager.h"

#include <algorithm>
//-----------------------------------------------------------------------------
// GameObjectManager Class
//-----------------------------------------------------------------------------

/**	@brief コンストラクタ
 *	@param const EngineServices* _services
 */
GameObjectManager::GameObjectManager(const EngineServices* _services) :
	gameObjects(), services(_services),
	pendingInit(),updateList(),drawList(), destroyQueue(),
	nameMap(),tagMap()
{}

/// @brief デストラクタ
GameObjectManager::~GameObjectManager()
{
	this->Dispose();
}
#include<iostream>

/// @brief 解放処理
void GameObjectManager::Dispose()
{
	// すでに破棄済みなら何もしない
	if (this->gameObjects.empty()) return;
	std::cout << "Size: " << this->gameObjects.size() << std::endl;

	// オブジェクトの解放
	for (auto& object : this->gameObjects)
	{
		if (object) 
		{
			object->Dispose();
			object.reset();
		}
	}

	// 配列の解放
	this->gameObjects.clear();
	this->pendingInit.clear();
	this->updateList.clear();
	this->drawList.clear();
	this->nameMap.clear();
	this->tagMap.clear();
}

/// @brief 未初期化オブジェクトを初期化する
void GameObjectManager::FlushInitialize()
{
	while (!this->pendingInit.empty())
	{
		GameObject* obj = this->pendingInit.front();
		this->pendingInit.pop_front();

		if (!obj) continue;

		// 初期化処理
		obj->Initialize();
	}
}

/** @brief オブジェクトの一括更新
 *  @param float _deltaTime デルタタイム
 */
void GameObjectManager::UpdateAll(float _deltaTime)
{
	for (auto& updatableObj : this->updateList)
	{
		if (updatableObj)
		{
			updatableObj->Update(_deltaTime);
		}
	}

	// 削除申請のあるオブジェクトを消す
	this->FlushDestroyQueue();
}

/// @brief オブジェクトの一括描画
void GameObjectManager::DrawAll() 
{
	for (auto& drawableObj : this->drawList)
	{
		if (drawableObj)
		{
			drawableObj->Draw();
		}
	}
}

/** @brief ゲームオブジェクトの作成
 *  @param const std::string& _name オブジェクトの名前
 *  @param const GameTags::Tag& _tag = GameTags::Tag::None オブジェクトのタグ名
 *  @param const bool _isActive = true オブジェクトの有効状態
 *  @return GameObject* 生成したゲームオブジェクト
 */
GameObject* GameObjectManager::Instantiate(const std::string& _name, const GameTags::Tag& _tag, const bool _isActive) 
{
	// GameObject を生成
	auto newObject = std::make_unique<GameObject>(*this, _name, _tag, _isActive);
	GameObject* rawPtr = newObject.get();

	// [TODO]GameObjectクラス内でSetUp()を作成しコンストラクタ内でSetUp()を定義する
	//rawPtr->SetUp();

	// リソース関連の参照を設定
	rawPtr->SetServices(this->services);

	// 初期化待ちキューに登録（初期化は次のループで呼ぶ）
	this->pendingInit.push_back(rawPtr);
	
	// 名前・タグマップに登録
	this->nameMap[_name] = rawPtr;
	this->tagMap[_tag] = rawPtr;

	// 管理リストに追加して所有権を保持
	this->gameObjects.push_back(std::move(newObject));

	return rawPtr;
}

/** @brief ゲームオブジェクトを名前検索で取得する
 *  @param const std::string& _name オブジェクトの名前
 *  @return GameObject* ゲームオブジェクト なければnullptr
 */
GameObject* GameObjectManager::GetFindObjectByName(const std::string& _name) 
{
	auto it = this->nameMap.find(_name);
	if (it != this->nameMap.end()) {
		return it->second;
	}
	return nullptr;
}

/** @brief ゲームオブジェクトをタグ検索で取得する
 *  @param const GameTags::Tag& _tag = GameTags::Tag::None オブジェクトのタグ名
 *  @return GameObject* ゲームオブジェクト なければnullptr
 */
GameObject* GameObjectManager::GetFindObjectWithTag(const GameTags::Tag& _tag)
{
	auto it = this->tagMap.find(_tag);
	if (it != this->tagMap.end()) {
		return it->second;
	}
	return nullptr;
}

/** @brief ゲームオブジェクトをタグ検索で取得する
 *  @param const GameTags::Tag& _tag = GameTags::Tag::None オブジェクトのタグ名
 *  @return std::vector<GameObject*> 当てはまるゲームオブジェクトのリスト
 */
std::vector<GameObject*> GameObjectManager::GetFindObjectsWithTag(const GameTags::Tag& _tag)
{
	std::vector<GameObject*> result;

	// 当てはまるオブジェクトがあれば格納していく
	for (const auto& obj : this->gameObjects)
	{
		if (obj && obj->GetTag() == _tag)
		{
			result.push_back(obj.get());
		}
	}

	return result;
}

/**	@brief 登録されたゲームオブジェクトを一括削除する
 *	@details 
 *	-	OnDestroy() によって削除キューに追加されたオブジェクトを更新後に安全に破棄する
 *  -	マップ・リスト・所有権をすべて解除しDispose() を呼び出してリソースを解放する
 */
void GameObjectManager::FlushDestroyQueue()
{
	while (!this->destroyQueue.empty())
	{
		// 削除するオブジェクトを取得
		GameObject* target = this->destroyQueue.front();
		this->destroyQueue.pop_front();

		// マップから除去
		this->nameMap.erase(target->GetName());
		this->tagMap.erase(target->GetTag());

		// リストから除去
		this->updateList.erase(std::remove(this->updateList.begin(), this->updateList.end(), target), this->updateList.end());
		this->drawList.erase(std::remove(this->drawList.begin(), this->drawList.end(), target), this->drawList.end());
		this->pendingInit.erase(std::remove(this->pendingInit.begin(), this->pendingInit.end(), target), this->pendingInit.end());

		// 所有リストから除去
		this->gameObjects.remove_if([target](const std::unique_ptr<GameObject>& obj)
		{
			if (obj.get() == target)
			{
				target->Dispose();
				return true;
			}
			return false;
		});
	}
}

/**@brief GameObjectからのイベント通知を受け取る
 * @param GameObject* _obj			通知元のGameObjectインスタンス
 * @param GameObjectEvent _event	発生したイベント種別
 * @detail
 *	-	GameObject が状態変化した際に呼び出される
 *	-	実装側では event の種類に応じて処理を分岐させる
 */
void GameObjectManager::OnGameObjectEvent(GameObject* _obj, GameObjectEvent _event)
{
	switch (_event) {
	case GameObjectEvent::Initialized:
		// 初期化直後：現在の状態に合わせて登録を正規化
		RefreshRegistration(_obj);
		break;

	case GameObjectEvent::Refreshed:
		// コンポーネント構成や有効/無効が変わった：登録を正規化
		RefreshRegistration(_obj);
		// std::cout << "Refreshed: " << _obj->GetName() << std::endl;
		break;

	case GameObjectEvent::Destroyed:
		if (!_obj) return;

		// 破棄前に、更新/描画リストから確実に外す（外し漏れ防止）
		erase_one(this->updateList, _obj);
		erase_one(this->drawList, _obj);

		// すでに破棄キューに入っていないか確認して追加する
		if (std::find(this->destroyQueue.begin(), this->destroyQueue.end(), _obj) == this->destroyQueue.end())
		{
			this->destroyQueue.push_back(_obj);
		}
		break;
	}
}

/**	@brief 現在の状態（IsUpdatable/IsDrawable）に合わせて登録を正規化
 *	@param _obj
 *	@details
 *		- 必要なら追加、不要なら除去
 *		- Initialized/Refreshed から共通で呼ぶ
 */
void GameObjectManager::RefreshRegistration(GameObject* _obj)
{
	if (!_obj) return;

	// Update 対象の正規化
	if (_obj->IsUpdatable())  { push_unique(this->updateList, _obj); }
	else{ erase_one(this->updateList, _obj); }

	// Draw 対象の正規化
	if (_obj->IsDrawable()){ push_unique(this->drawList, _obj); }
	else { erase_one(this->drawList, _obj); }
}