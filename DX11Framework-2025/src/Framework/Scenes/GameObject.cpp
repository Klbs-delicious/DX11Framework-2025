/** @file   GameObject.cpp
*   @date   2025/09/16
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Framework/Scenes/GameObject.h"

#include <algorithm>

//-----------------------------------------------------------------------------
// GameLoop Class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
*	@param	const std::string& _name                        オブジェクトの名前
*	@param	const GameTags::Tag _tag = GameTags::Tag::None	オブジェクトのタグ名
*	@param	const bool _isActive = true						オブジェクトの有効状態
*/
GameObject::GameObject(const std::string& _name, const GameTags::Tag _tag, const bool _isActive)
    : isPendingDestroy(false), isActive(_isActive), parent(nullptr), name(_name), tag(_tag) {}

/// @brief	デストラクタ
GameObject::~GameObject() {}

/// @brief	初期化処理を行う
void GameObject::Initialize()
{
    for (auto& component : this->components)
    {
        component->Initialize();
    }
}

/**	@brief		オブジェクトの更新を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void GameObject::Update(float _deltaTime)
{
    if (!this->isActive || this->isPendingDestroy) return;

    for (auto* updatable : this->updatableComponents)
    {
        updatable->Update(_deltaTime);
    }
}

/**	@brief		ゲームオブジェクトの描画処理を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void GameObject::Draw()
{
    if (!this->isActive || this->isPendingDestroy) return;

    for (auto* drawable : this->drawableComponents)
    {
        drawable->Draw();
    }
}

/**	@brief		終了処理を行う
 *	@details	継承を禁止する
 */
void GameObject::Dispose()
{
    for (auto& component : this->components)
    {
        component->Dispose();
    }
    this->components.clear();
    this->updatableComponents.clear();
    this->drawableComponents.clear();
    this->children.clear();
    this->name.clear();
}

/** @brief  オブジェクトの削除申請を行う
 */
void GameObject::OnDestroy()
{
    this->isPendingDestroy = true;

    // 子オブジェクトにも通知する
    for (auto* child : this->children)
    {
        child->OnDestroy();
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
    if (_child) _child->parent = nullptr;
}
