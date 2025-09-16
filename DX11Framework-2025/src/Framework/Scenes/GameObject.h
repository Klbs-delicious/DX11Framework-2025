/**@file   GameObject.h
 * @date   2025/09/16
 */
#pragma once
#include"Framework/Scenes/Component.h"
#include"Framework/Scenes//PhaseInterfaces.h"

#include<string>
#include<list>
#include<vector>
#include<memory>

 /** @class	GameObject
  *	@brief	ゲーム内の振る舞いや構造を構成する基本単位
  *	@details	
  *				- このクラスは継承を禁止しており、コンポジションによって機能を拡張する設計
  *				- 更新・描画などの責務はコンポーネントに委譲される
  */
class GameObject final
{
public:
	/** @brief  コンストラクタ
	*	@param	bool _isActive = true	オブジェクトの有効状態
	*/
	GameObject(bool _isActive = true);

	/// @brief	デストラクタ
	~GameObject();

	/// @brief	初期化処理を行う
	void Initialize();

	/**	@brief		オブジェクトの更新を行う
	 *	@param		float _deltaTime	デルタタイム
	 *	@details	継承を禁止する
	 */
	void Update(float _deltaTime);

	/**	@brief		ゲームオブジェクトの描画処理を行う
	 *	@param		float _deltaTime	デルタタイム
	 *	@details	継承を禁止する
	 */
	void Draw();

	/**	@brief		終了処理を行う
	 *	@details	継承を禁止する
	 */
	void Dispose();

	/** @brief  オブジェクトの有効状態を設定する
	 *  @param bool _active	オブジェクトの有効/無効
	 */
	void SetActive(bool _active) { this->isActive = _active; }

	/** @brief  オブジェクトが有効かどうかを取得する
	 *  @return bool オブジェクトが有効なら true
	 */
	bool IsActive() const { return this->isActive; }

	/** @brief  オブジェクトの削除申請を行う
	 */
	void OnDestroy();

	/** @brief  オブジェクトが削除申請済かどうかを取得する
	 *  @return bool	削除申請済なら true
	 */
	bool IsPendingDestroy()const;

	/** @brief  子オブジェクトの追加
	 *  @param	GameObject* _child	子オブジェクト
	 */
	void AddChildObject(GameObject* _child);

	/** @brief  子オブジェクトの削除
	 *  @param	GameObject* _child	子オブジェクト
	 */
	void RemoveChildObject(GameObject* _child);

	template<typename T>
	/** @brief  コンポーネントの追加
	 *  @return	T*	追加したコンポーネント
	 */
	T* AddComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		auto component = std::make_unique<T>(this);

		T* rawPtr = component.get();
		this->components.emplace_back(std::move(component));

		// IUpdatable に該当するなら登録
		if (auto updatable = dynamic_cast<IUpdatable*>(rawPtr)) {
			this->updatableComponents.push_back(updatable);
		}

		// IDrawable に該当するなら登録
		if (auto drawable = dynamic_cast<IDrawable*>(rawPtr)) {
			this->drawableComponents.push_back(drawable);
		}

		return rawPtr;
	}

	template<typename T>
	/** @brief  コンポーネントの取得
	 */
	void GetComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		for (auto& comp : this->components)
		{
			if (auto casted = dynamic_cast<T*>(comp.get())) {
				return casted;
			}
		}
		return nullptr;
	}

	template<typename T>
	/** @brief  コンポーネントの削除
	 */
	void RemoveComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		for (auto it = this->components.begin(); it != this->components.end(); ++it)
		{
			if (auto casted = dynamic_cast<T*>(it->get())) {
				// Updatable/Drawable リストからも削除
				this->updatableComponents.erase(
					std::remove(this->updatableComponents.begin(), this->updatableComponents.end(), dynamic_cast<IUpdatable*>(casted)),
					this->updatableComponents.end()
				);
				this->drawableComponents.erase(
					std::remove(this->drawableComponents.begin(), this->drawableComponents.end(), dynamic_cast<IDrawable*>(casted)),
					this->drawableComponents.end()
				);
				(*it)->Dispose();
				this->components.erase(it);
				break;
			}
		}
	}

private:
	bool isPendingDestroy;	///< オブジェクトの削除フラグ
	bool isActive;			///< オブジェクトの有効/無効フラグ

	GameObject* parent;	///< 親オブジェクト
	std::string name;	///< オブジェクト名
	///< [TODO]タグ名
	
	///< [TODO]位置、回転、スケール情報

	std::vector<GameObject*> children;						///< 子オブジェクトのリスト
	std::list<std::unique_ptr<Component>>	components;		///< コンポーネントのリスト
	std::vector<IUpdatable*>	updatableComponents;		///< 更新関連のコンポーネントのリスト
	std::vector<IDrawable*>		drawableComponents;			///< 描画関連のコンポーネントのリスト
};