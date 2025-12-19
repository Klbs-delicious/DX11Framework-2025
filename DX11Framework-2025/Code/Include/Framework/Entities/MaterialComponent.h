/**@file   MaterialComponent.h
 * @date   2025/10/19
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Graphics/Material.h"
#include "Include/Framework/Entities/GameObject.h"

 /** @class	MaterialComponent
  *	 @brief	マテリアル情報を保持するコンポーネント
  */
class MaterialComponent :public Component
{
public:
	MaterialComponent(GameObject* _owner, bool _isActive = true);
	~MaterialComponent()override;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/**	@brief マテリアル情報の設定
	 *	@param Material* _baseMaterial	画像情報
	 */
	void SetMaterial(Material* _baseMaterial);

	/**	@brief 画像情報の設定
	 *	@param TextureResource* _overrideSprite	画像情報
	 */
	void SetTexture(TextureResource* _overrideSprite);

	/**	@brief パラメータ情報の設定
	 *	@param const MaterialParams& _params	パラメータ情報の参照
	 */
	void SetParams(const MaterialParams& _params);

	/** @brief 指定したブレンドステートを設定
	 *  @param BlendStateType _blendState 使用するブレンドステートの種類
	 */
	void SetBlendState(BlendStateType _blendState);

	/**	@brief	マテリアル情報の取得
	 *	@return	Material*	画像情報
	 */
	Material* GetMaterial()const;

	/** @brief マテリアルを適用する
	 *  @param ID3D11DeviceContext* _context
	 *  @param RenderSystem* _renderSystem
	 */
	void Apply(ID3D11DeviceContext* _context, RenderSystem* _renderSystem);

private:
	Material* baseMaterial;				///< 描画に必要な共通リソースをまとめたデータ構造
	TextureResource* overrideTexture;	///< 個別設定用のテクスチャ
	MaterialParams param;				///< オブジェクトごとに変化するパラメータ
};
