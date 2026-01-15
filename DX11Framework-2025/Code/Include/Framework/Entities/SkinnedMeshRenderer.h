/** @file   SkinnedMeshRenderer.h
 *  @brief  スキニング済みメッシュを描画するコンポーネント
 *  @date   2026/01/15
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/MaterialComponent.h"
#include "Include/Framework/Entities/MeshComponent.h"
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Entities/AnimationComponent.h"

#include "Include/Framework/Graphics/ModelImporter.h"
#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Graphics/DynamicConstantBuffer.h"

#include <cstdint>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;

//-----------------------------------------------------------------------------
// SkinnedMeshRenderer class
//-----------------------------------------------------------------------------

/** @class SkinnedMeshRenderer
 *  @brief スキニング用シェーダーでメッシュを描画するコンポーネント
 *  @details ボーン行列の更新は別コンポーネント（例：AnimationComponent）側で完了している前提
 */
class SkinnedMeshRenderer : public Component, public IDrawable
{
	using ModelData_t = Graphics::Import::ModelData;
	using ModelImporter_t = Graphics::Import::ModelImporter;
	using Vertex_t = Graphics::Import::Vertex;

public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	SkinnedMeshRenderer(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~SkinnedMeshRenderer() override;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/// @brief 描画処理
	void Draw() override;

	// 簡易ライト設定
	struct LightBuffer
	{
		DX::Vector3 lightDir;
		float pad1;
		DX::Vector4 baseColor;
	};

private:
	Transform* transform;
	Camera3D* camera;
	AnimationComponent* animationComponent;

	MeshComponent* meshComponent;          ///< メッシュ情報
	MaterialComponent* materialComponent;   ///< マテリアル情報

	LightBuffer light = {};
	std::unique_ptr<DynamicConstantBuffer<LightBuffer>> lightBuffer; ///< ライト用バッファ
};

