/** @file   GameObject.cpp
*   @date   2025/09/16
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/GameObject.h"

#include <iostream>

//-----------------------------------------------------------------------------
// GameLoop Class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
*	@param	IGameObjectObserver&	    _gameobjctObs					GameObjectの状態を通知する
*	@param	const std::string&			_name							オブジェクトの名前
*	@param	const GameTags::Tag			_tag = GameTags::Tag::None		オブジェクトのタグ名
*	@param	const bool					_isActive = true				オブジェクトの有効状態
*/
GameObject::GameObject(IGameObjectObserver& _gameobjctObs, const std::string& _name, const GameTags::Tag _tag, const bool _isActive)
    : gameObjectObs(_gameobjctObs), transform(nullptr), timeScaleComponent(nullptr) , isPendingDestroy(false), isActive(_isActive), parent(nullptr), name(_name), tag(_tag)
{};

/// @brief	デストラクタ
GameObject::~GameObject() {}

/// @brief	初期化処理を行う
void GameObject::Initialize()
{}

/**	@brief		オブジェクトの更新を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void GameObject::Update(float _deltaTime)
{
    if (!this->isActive || this->isPendingDestroy) return;

    // ここでTransformを更新する
    if (this->transform) { this->transform->UpdateWorldMatrix(); }
}

/**	@brief		終了処理を行う
 *	@details	継承を禁止する
 */
void GameObject::Dispose()
{
    std::cout << this->name << " （" << std::to_string(static_cast<int>(this->tag)) << "番）を削除した！" << std::endl;

    for (auto& component : this->components)
    {
        component->Dispose();
    }
	this->transform = nullptr;
    this->components.clear();
    this->children.clear();
    this->name.clear();
}

/** @brief  オブジェクトの削除申請を行う
 */
void GameObject::OnDestroy()
{
    if (this->isPendingDestroy) return;

    this->isPendingDestroy = true;

    // オブジェクトの削除通知
    GameObjectEventContext eventContext =
    {
        this->name,
        nullptr,
        GameObjectEvent::Destroyed
    };
    this->NotifyEvent(eventContext);

    // 子オブジェクトにも通知する
    for (auto* child : this->children) {
        child->OnDestroy();
    }
}

/** @brief  親オブジェクトの設定
 *  @param	GameObject* _parent	親オブジェクト
 */
void GameObject::SetParent(GameObject* _parent)
{
    if (this->parent)
    {
        this->parent->RemoveChildObject(this);
    }

    this->parent = _parent;
    
    if (_parent)
    {
        _parent->AddChildObject(this);
    }
}

/** @brief  子オブジェクトの追加
 *  @param	GameObject* _child	子オブジェクト
 */
void GameObject::AddChildObject(GameObject* _child)
{
    if (_child)
    {
        this->children.push_back(_child);
        _child->parent = this;
        _child->transform->SetParentInternal(this->transform);
    }
}

/** @brief  子オブジェクトの削除
 *  @param	GameObject* _child	子オブジェクト
 */
void GameObject::RemoveChildObject(GameObject* _child)
{
    this->children.erase(
        std::remove(this->children.begin(), this->children.end(), _child),
        this->children.end()
    );
    if (_child)
    {
        _child->parent = nullptr;
		_child->transform->SetParentInternal(nullptr);
    }
}