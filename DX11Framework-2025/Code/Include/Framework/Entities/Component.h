/**@file   Component.h
 * @date   2025/09/16
 */
#pragma once
#include"Include/Framework/Core/EngineServices.h"
#include"Include/Framework/Event/GameObjectEvent.h"

class GameObject; 

 /** @class Component
  *  @brief GameObject にアタッチされる振る舞いの基底クラス
  *  @details - 各種機能（描画、物理、AIなど）をコンポーネントとして分離し、責務ごとに拡張可能にする
  */
class Component
{
public:
	/** @brief  コンストラクタ
	 *  @param GameObject* _owner	このコンポーネントがアタッチされるオブジェクト
	 *  @param bool _active	コンポーネントの有効/無効
	 */
	Component(GameObject* _owner, bool _isActive = true) :owner(_owner), isActive(_isActive) {};

	/// @brief デストラクタ
	virtual ~Component() = default;

	/// @brief 初期化処理
	virtual void Initialize() {};

	/// @brief 終了処理
	virtual void Dispose() {};

	/** @brief  コンポーネントの有効状態を設定する
	 *  @param bool _active	コンポーネントの有効/無効
	 */
	void SetActive(bool _active);

	/** @brief  コンポーネントが有効かどうかを取得する
	 *  @return bool コンポーネントが有効なら true
	 */
	bool IsActive() const { return this->isActive; }

	/**	@brief	親オブジェクトの取得
	 *	@return	GameObject*
	 */
	GameObject* Owner() const { return this->owner; }

protected:
	/**	@brief リソース関連の参照を取得する
	 *	@return EngineServices*
	 */
	const EngineServices* Services() const;

private :
	GameObject* owner;	///< このコンポーネントがアタッチされているオブジェクト
	bool isActive;
};