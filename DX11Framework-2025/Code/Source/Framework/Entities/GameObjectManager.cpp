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
	pendingInits(), updates(), fixedUpdates(), destroyQueue(),
	renderes(), rigidbodies(), transforms(), 
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
	if (this->gameObjects.empty()) { return; }

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
	this->destroyQueue.clear();

	// コンポーネント配列の解放
	this->pendingInits.clear();
	this->updates.clear();
	this->fixedUpdates.clear();
	this->renderes.clear();
	this->rigidbodies.clear();
	this->transforms.clear();

	// マップの解放
	this->nameMap.clear();
	this->tagMap.clear();
}

/// @brief 未初期化オブジェクトを初期化する
void GameObjectManager::FlushInitialize()
{
	while (!this->pendingInits.empty())
	{
		// コンポーネントを取り出す
		Component* comp = this->pendingInits.front();
		this->pendingInits.pop_front();

		if (!comp) { continue; }
		comp->Initialize();
	}
}

/** @brief 一括更新
 *  @param float _deltaTime デルタタイム
 */
void GameObjectManager::UpdateAll(float _deltaTime)
{
	for (auto& update : this->updates)
	{
		if (update)
		{
			// 時間スケールを考慮して更新する
			auto comp = dynamic_cast<Component*>(update);
			auto obj = comp->Owner();
			float scaledDelta = obj->TimeScale()->ApplyTimeScale(_deltaTime);

			update->Update(scaledDelta);
		}
	}
}

/**	@brief 一括固定更新
 *	@param		float _deltaTime	デルタタイム
 */
void GameObjectManager::FixedUpdateAll(float _deltaTime)
{
	// 固定更新を持つコンポーネントを更新
	for (auto& fixedUpdate : this->fixedUpdates)
	{
		if (fixedUpdate)
		{
			// 時間スケールを考慮して更新する
			auto comp = dynamic_cast<Component*>(fixedUpdate);
			auto obj = comp->Owner();
			float scaledDelta = obj->TimeScale()->ApplyTimeScale(_deltaTime);

			fixedUpdate->FixedUpdate(scaledDelta);
		}
	}
	//std::cout << "FixedUpdateAll called with deltaTime: " << _deltaTime << std::endl;
}

void GameObjectManager::UpdateAllTransforms()
{
	for (auto& transform : this->transforms)
	{
		if (transform)
		{
			transform->UpdateWorldMatrix();
		}
	}
}

void GameObjectManager::BeginPhysics(float _deltaTime)
{
	for (auto& rigidbody : this->rigidbodies)
	{
		if (rigidbody)
		{
			// 物理シミュレーションステップを進める
			rigidbody->StepPhysics(_deltaTime);
		}
	}

	// 全 Transform のワールド行列を更新する
	this->UpdateAllTransforms();

	for (auto& rigidbody : this->rigidbodies)
	{
		if (rigidbody)
		{
			// Jolt 側に最新の Transform を反映させる
			rigidbody->SyncVisualToJolt(_deltaTime);
		}
	}
}

void GameObjectManager::EndPhysics()
{
	for (auto& rigidbody : this->rigidbodies)
	{
		if (rigidbody)
		{
			// Jolt の押し戻し結果を visualTransform に反映させる
			rigidbody->SyncJoltToVisual();
		}
	}
}

/// @brief 一括描画
void GameObjectManager::RenderAll()
{
	for (auto& render : this->renderes)
	{
		if (render){ render->Draw(); }
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

	// リソース関連の参照を設定
	rawPtr->SetServices(this->services);
	
	// 名前・タグマップに登録
	this->nameMap[_name] = rawPtr;
	this->tagMap[_tag].push_back(rawPtr);

	// 管理リストに追加して所有権を保持
	this->gameObjects.push_back(std::move(newObject));

	// 必須コンポーネントを追加
	rawPtr->transform = rawPtr->AddComponent<Transform>();
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
	if (it != this->nameMap.end()) { return it->second; }

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

/**@brief GameObjectからのイベント通知を受け取る（コンテキスト版）
 * @param GameObjectEventContext _eventContext	イベントコンテキスト情報
 * @detail
 *	-	GameObject が状態変化した際に呼び出される
 *	-	実装側では eventContext.eventType の種類に応じて処理を分岐させる
 */
void GameObjectManager::OnGameObjectEvent(const GameObjectEventContext _ctx)
{
	GameObject* obj = this->GetFindObjectByName(_ctx.objectName);
	if (!obj){ return; }

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
	}

	// std::cout << "Event: " << static_cast<int>(_ctx.eventType) << " Object: " << _ctx.objectName << std::endl;
	// 現在のリストの数を表示
	//std::cout << "[GameObjectManager] UpdateList Size: " << this->updates.size() << " DrawList Size: " << this->render3D.size()<< " pendingInits Size: " << this->pendingInits.size() << std::endl;
}

//-----------------------------------------------------------------------------
// GameObjectManager - Component Phase Registration Helper
//-----------------------------------------------------------------------------

void GameObjectManager::RegisterComponentToPhases(Component* _component)
{
	if (!_component){ return; }

	// Update フェーズ
	if (auto u = dynamic_cast<IUpdatable*>(_component))
	{
		PushUnique(this->updates, u);
	}

	// FixedUpdate フェーズ
	if (auto f = dynamic_cast<IFixedUpdatable*>(_component))
	{
		PushUnique(this->fixedUpdates, f);
	}

	// 描画
	if (auto dUI = dynamic_cast<IDrawable*>(_component))
	{
		PushUnique(this->renderes, dUI);
	}

	// Rigidbody3D
	if (auto rb3D = dynamic_cast<Framework::Physics::Rigidbody3D*>(_component))
	{
		PushUnique(this->rigidbodies, rb3D);
	}

	// Transform
	if (auto tf = dynamic_cast<Transform*>(_component))
	{
		PushUnique(this->transforms, tf);
	}
}

void GameObjectManager::UnregisterComponentFromPhases(Component* _component)
{
	if (!_component) { return; }

	// Update フェーズ
	if (auto u = dynamic_cast<IUpdatable*>(_component))
	{
		EraseOne(this->updates, u);
	}

	// FixedUpdate フェーズ
	if (auto f = dynamic_cast<IFixedUpdatable*>(_component))
	{
		EraseOne(this->fixedUpdates, f);
	}

	// 描画
	if (auto dUI = dynamic_cast<IDrawable*>(_component))
	{
		EraseOne(this->renderes, dUI);
	}

	// Rigidbody3D
	if (auto rb3D = dynamic_cast<Framework::Physics::Rigidbody3D*>(_component))
	{
		EraseOne(this->rigidbodies, rb3D);
	}

	// Transform
	if (auto tf = dynamic_cast<Transform*>(_component))
	{
		EraseOne(this->transforms, tf);
	}
}