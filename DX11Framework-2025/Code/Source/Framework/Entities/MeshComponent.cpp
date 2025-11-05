/** @file   MeshComponent.cpp
 *  @date   2025/11/02
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/MeshComponent.h"
#include "Include/Framework/Entities/GameObject.h"

//-----------------------------------------------------------------------------
// MeshComponent Class
//-----------------------------------------------------------------------------

/**@brief コンストラクタ
 * @param _owner 
 * @param _isActive 
 */
MeshComponent::MeshComponent(GameObject* _owner, bool _isActive)
    : Component(_owner, _isActive), mesh(nullptr)
{}

/// @brief	デストラクタ
MeshComponent::~MeshComponent()
{
    this->Dispose();
}

/// @brief 初期化処理
void MeshComponent::Initialize() {}

/// @brief 終了処理
void MeshComponent::Dispose()
{
    // メモリ管理はMeshResource側で行う
    this->mesh = nullptr;
}