/** @file   GameObjectManager.cpp
*   @date   2025/09/17
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/GameObjectManager.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/TimeScaleComponent.h"

#include <algorithm>
#include<iostream>

//-----------------------------------------------------------------------------
// GameObjectManager Class
//-----------------------------------------------------------------------------

/**	@brief コンストラクタ
 *	@param const EngineServices* _services
 */
GameObjectManager::GameObjectManager(const EngineServices* _services) :
	gameObjects(), services(_services),
	pendingInit(),updateList(),drawList(), destroyQueue(),
	pendingInits(), updates(), fixedUpdates(), renderUI(), render3D(),
	nameMap(),tagMap()
{}

/// @brief デストラクタ
GameObjectManager::~GameObjectManager()
{
	this->Dispose();
}

/// @brief 解放処理
void GameObjectManager::Dispose()
{
	// すでに破棄済みなら何もしない
	if (this->gameObjects.empty()) return;
	std::cout << "[GameObjectManager] Size: " << this->gameObjects.size() << std::endl;

	// オブジェクトの解放
	for (auto& object : this->gameObjects)
	{
		if (object) 
		{
			object->Dispose();
			object.reset();
		}
	}

	// オブジェクト配列の解放
	this->gameObjects.clear();
	this->pendingInit.clear();
	this->updateList.clear();
	this->drawList.clear();
	this->destroyQueue.clear();

	// コンポーネント配列の解放
	this->pendingInits.clear();
	this->updates.clear();
	this->fixedUpdates.clear();
	this->renderUI.clear();
	this->render3D.clear();

	// マップの解放
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

	//while (!this->pendingInits.empty())
	//{
	//	// コンポーネントを取り出す
	//	Component* comp = this->pendingInits.front();
	//	this->pendingInits.pop_front();

	//	if (!comp) continue;

	//	// 初期化処理
	//	comp->Initialize();
	//}
	//std::cout << "pendingInits Size : " << pendingInits.size() << std::endl;
}

/** @brief 一括更新
 *  @param float _deltaTime デルタタイム
 */
void GameObjectManager::UpdateAll(float _deltaTime)
{
	for (auto& updatableObj : this->updateList)
	{
		if (updatableObj)
		{
			float scaledDelta = updatableObj->TimeScale()->ApplyTimeScale(_deltaTime);
			updatableObj->Update(scaledDelta);
		}
	}

	//for (auto& update : this->updates)
	//{
	//	if (update)
	//	{
	//		// 時間スケールを考慮して更新する
	//		auto comp = dynamic_cast<Component*>(update);
	//		auto obj = comp->Owner();
	//		float scaledDelta = obj->TimeScale()->ApplyTimeScale(_deltaTime);
	//		update->Update(scaledDelta);

	//		std::cout << "[GOM] Update: "
	//			<< obj->GetName()
	//			<< " raw=" << _deltaTime
	//			<< " scaled=" << scaledDelta
	//			<< std::endl;
	//	}
	//}

	// 削除申請のあるオブジェクトを消す
	this->FlushDestroyQueue();
}

/**	@brief 一括固定更新
 *	@param		float _deltaTime	デルタタイム
 */
void GameObjectManager::FixedUpdateAll(float _deltaTime)
{
	//// 固定更新を持つコンポーネントを更新
	//for (auto& fixedUpdate : this->fixedUpdates)
	//{
	//	if (fixedUpdate)
	//	{
	// 			// 時間スケールを考慮して更新する
	//		auto comp = dynamic_cast<Component*>(fixedUpdate);
	//		auto obj = comp->Owner();
	//		float scaledDelta = obj->TimeScale()->ApplyTimeScale(_deltaTime);
	//// 
	//		// [TODO] FixedUpdateが未実装のため暫定でUpdateを呼ぶ
	//		fixedUpdate->Update(scaledDelta);
	//	}
	//}
}

/// @brief 3Dコンポーネントの一括描画
void GameObjectManager::Render3DAll()
{
	for (auto& drawableObj : this->drawList)
	{
		if (drawableObj)
		{
			// [TODO] 一旦 Draw を呼ぶようにしているが、将来的に IDrawable の Draw を呼ぶように修正する
			drawableObj->Draw();
		}
	}

	//for (auto& render : this->render3D)
	//{
	//	if (render)
	//	{
	//		render->Draw();
	//	}
	//}
}

/// @brief UIコンポーネントの一括描画
void GameObjectManager::RenderUIAll()
{
	//for (auto& render : this->renderUI)
	//{
	//	if (render)
	//	{
	//		render->Draw();
	//	}
	//}
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

	// リソース関連の参照を設定
	rawPtr->SetServices(this->services);

	// 初期化待ちキューに登録（初期化は次のループで呼ぶ）
	this->pendingInit.push_back(rawPtr);
	
	// 名前・タグマップに登録
	this->nameMap[_name] = rawPtr;
	this->tagMap[_tag].push_back(rawPtr);

	// 管理リストに追加して所有権を保持
	this->gameObjects.push_back(std::move(newObject));

	// 必須コンポーネントを追加
	rawPtr->transform = rawPtr->AddComponent<Transform>();// 実行確認のためにpublicメンバに入れている、将来的にgetterのみに変更予定
	rawPtr->AddComponent<TimeScaleComponent>();

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
 *  @return std::vector<GameObject*> 当てはまるゲームオブジェクトのリスト
 */
std::vector<GameObject*> GameObjectManager::GetFindObjectsWithTag(const GameTags::Tag& _tag)
{
	return this->tagMap[_tag];
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
		EraseOne(this->updateList, _obj);
		EraseOne(this->drawList, _obj);

		// すでに破棄キューに入っていないか確認して追加する
		if (std::find(this->destroyQueue.begin(), this->destroyQueue.end(), _obj) == this->destroyQueue.end())
		{
			this->destroyQueue.push_back(_obj);
		}
		break;
	}
}

/**@brief GameObjectからのイベント通知を受け取る（コンテキスト版）
 * @param GameObjectEventContext _eventContext	イベントコンテキスト情報
 * @detail
 *	-	GameObject が状態変化した際に呼び出される
 *	-	実装側では eventContext.eventType の種類に応じて処理を分岐させる
 */
void GameObjectManager::OnGameObjectEvent(const GameObjectEventContext _ctx)
{
	GameObject* obj = this->GetFindObjectByName(_ctx.objectName);
	if (!obj) return;

	switch (_ctx.eventType)
	{
		// オブジェクト有効化
	case GameObjectEvent::GameObjectEnabled:
		for (auto& compUPtr : obj->GetComponents())
		{
			RegisterComponentToPhases(compUPtr.get());
		}
		break;

		// オブジェクト無効化
	case GameObjectEvent::GameObjectDisabled:
		for (auto& compUPtr : obj->GetComponents())
		{
			UnregisterComponentFromPhases(compUPtr.get());
		}
		break;

		// コンポーネント有効化
	case GameObjectEvent::ComponentEnabled:
		RegisterComponentToPhases(_ctx.component);
		break;

		// コンポーネント無効化
	case GameObjectEvent::ComponentDisabled:
		UnregisterComponentFromPhases(_ctx.component);
		break;

		// コンポーネント追加
	case GameObjectEvent::ComponentAdded:
		RegisterComponentToPhases(_ctx.component);

		// 初期化待ちキューに登録
		this->pendingInits.push_back(_ctx.component);
		break;

		// コンポーネント削除
	case GameObjectEvent::ComponentRemoved:
		UnregisterComponentFromPhases(_ctx.component);
		break;

	case GameObjectEvent::Destroyed:

		// 破棄前に、更新/描画リストから確実に外す（外し漏れ防止）
		EraseOne(this->updateList, obj);
		EraseOne(this->drawList, obj);

		// すでに破棄キューに入っていないか確認して追加する
		if (std::find(this->destroyQueue.begin(), this->destroyQueue.end(), obj) == this->destroyQueue.end())
		{
			this->destroyQueue.push_back(obj);
		}

		// フェーズからも外す
		for (auto& compUPtr : obj->GetComponents())
		{
			UnregisterComponentFromPhases(compUPtr.get());
		}

		break;
	case GameObjectEvent::Refreshed:
	case GameObjectEvent::Initialized:
		// 旧システム互換のため残している処理
		RefreshRegistration(obj);
		break;
	}

	// std::cout << "Event: " << static_cast<int>(_ctx.eventType) << " Object: " << _ctx.objectName << std::endl;
	// 現在のリストの数を表示
	std::cout << "[GameObjectManager] UpdateList Size: " << this->updates.size() << " DrawList Size: " << this->render3D.size()<< " pendingInits Size: " << this->pendingInits.size() << std::endl;
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
	if (_obj->IsUpdatable())  { PushUnique(this->updateList, _obj); }
	else{ EraseOne(this->updateList, _obj); }

	// Draw 対象の正規化
	if (_obj->IsDrawable()){ PushUnique(this->drawList, _obj); }
	else { EraseOne(this->drawList, _obj); }
}

//-----------------------------------------------------------------------------
// GameObjectManager - Component Phase Registration Helper
//-----------------------------------------------------------------------------

void GameObjectManager::RegisterComponentToPhases(Component* _component)
{
	if (!_component) return;

	// Update フェーズ
	if (auto u = dynamic_cast<IUpdatable*>(_component))
	{
		PushUnique(this->updates, u);
	}

	// FixedUpdate フェーズ
	if (auto f = dynamic_cast<IUpdatable*>(_component))
	{
		PushUnique(this->fixedUpdates, f);
	}

	// 3D 描画
	if (auto d3 = dynamic_cast<IDrawable*>(_component))
	{
		PushUnique(this->render3D, d3);
	}

	// UI 描画
	if (auto dUI = dynamic_cast<IDrawable*>(_component))
	{
		PushUnique(this->renderUI, dUI);
	}
}

void GameObjectManager::UnregisterComponentFromPhases(Component* _component)
{
	if (!_component) return;

	// Update フェーズ
	if (auto u = dynamic_cast<IUpdatable*>(_component))
	{
		EraseOne(this->updates, u);
	}

	// FixedUpdate フェーズ
	if (auto f = dynamic_cast<IUpdatable*>(_component))
	{
		EraseOne(this->fixedUpdates, f);
	}

	// 3D 描画
	if (auto d3 = dynamic_cast<IDrawable*>(_component))
	{
		EraseOne(this->render3D, d3);
	}

	// UI 描画
	if (auto dUI = dynamic_cast<IDrawable*>(_component))
	{
		EraseOne(this->renderUI, dUI);
	}
}