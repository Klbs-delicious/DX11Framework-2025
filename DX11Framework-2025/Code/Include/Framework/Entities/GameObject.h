/**@file   GameObject.h
 * @date   2025/09/16
 */
#pragma once
#include"Include/Framework/Entities/Component.h"
#include"Include/Framework/Entities/PhaseInterfaces.h"
#include"Include/Framework/Entities/Transform.h"
#include"Include/Framework/Entities/TimeScaleComponent.h"

#include"Include/Framework/Event/GameObjectEvent.h"

#include<string>
#include<list>
#include<vector>
#include<memory>

 /** @namespace GameTags
  *  @brief     ゲームオブジェクトの識別に使用するタグやレイヤーを定義する名前空間
  */

namespace GameTags
{
	/** @enum Tag
	 *  @brief ゲームオブジェクトの分類に使用するタグ
	 */
	enum class Tag
	{
		None = 0,
		Camera,
		Player,
		Enemy,
		UI,
		Environment,
	};

	/** @enum Layer
	 *  @brief 描画や衝突判定などに使用するレイヤー定義
	 */
	enum class Layer
	{
		Default = 0,
		TransparentFX,
		UI,
		IgnoreRaycast,
		Background,
	};
}

 /** @class	GameObject
  *	@brief	ゲーム内の振る舞いや構造を構成する基本単位
  *	@details	
  *		- このクラスは継承を禁止しており、コンポジションによって機能を拡張する設計
  *		- 更新・描画などの責務はコンポーネントに委譲される
  */
class GameObject final
{
public:
	/** @brief  コンストラクタ
	*	@param	IGameObjectObserver&		_gameobjctObs					GameObjectの状態を通知する
	*	@param	const std::string&			_name							オブジェクトの名前
	*	@param	const GameTags::Tag			_tag = GameTags::Tag::None		オブジェクトのタグ名
	*	@param	const bool					_isActive = true				オブジェクトの有効状態
	*/
	GameObject(IGameObjectObserver& _gameobjctObs, const std::string& _name, const GameTags::Tag _tag = GameTags::Tag::None, const bool _isActive = true);

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

	/** @brief  オブジェクトの名前を取得する
	 *  @return const std::string& オブジェクトの名前（読み取り専用）
	 */
	const std::string& GetName()const { return this->name; }

	/** @brief  ゲームオブジェクトを識別するタグを取得する
	 *  @return const GameTags::Tag& オブジェクトを識別するタグ（読み取り専用）
	 */
	const GameTags::Tag& GetTag()const { return this->tag; }

	/// @brief	オブジェクトの削除申請を行う
	void OnDestroy();

	/** @brief  オブジェクトが削除申請済かどうかを取得する
	 *  @return bool	削除申請済なら true
	 */
	bool IsPendingDestroy()const { return this->isPendingDestroy; }

	/** @brief  オブジェクトが更新処理を持っているかどうか
	 *  @return bool	更新処理を持っているなら true
	 */
	bool IsUpdatable() const { return !this->updatableComponents.empty(); }

	/** @brief  オブジェクトが描画処理を持っているかどうか
	 *  @return bool	描画処理を持っているなら true
	 */
	bool IsDrawable() const { return !this->drawableComponents.empty(); }

	/** @brief  親オブジェクトの設定
	 *  @param	GameObject* _parent	親オブジェクト
	 */
	void SetParent(GameObject* _parent);

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
		static_assert(std::is_base_of<Component, T>::value, " クラス T はComponentから派生する必要があります。");

		//コンポーネントの生成
		auto component = std::make_unique<T>(static_cast<GameObject*>(this));

		T* rawPtr = component.get();
		this->components.emplace_back(std::move(component));

		// IUpdatable,IDrawableがあるか取得
		bool isUpdatable = this->IsUpdatable();
		bool isDrawable = this->IsDrawable();

		// IUpdatable に該当するなら登録
		if (auto updatable = dynamic_cast<IUpdatable*>(rawPtr))
		{
			this->updatableComponents.push_back(updatable);

			if(!isUpdatable)
			{
				// GameObject管理に配列に入れるように申請する
				this->gameObjectObs.OnGameObjectEvent(this, GameObjectEvent::Refreshed);
			}
		}

		// IDrawable に該当するなら登録
		if (auto drawable = dynamic_cast<IDrawable*>(rawPtr))
		{
			this->drawableComponents.push_back(drawable);

			if (!isDrawable) 
			{
				// GameObject管理に配列に入れるように申請する	
				this->gameObjectObs.OnGameObjectEvent(this, GameObjectEvent::Refreshed);
			}
		}

		return rawPtr;
	}

	template<typename T>
	/** @brief  コンポーネントの取得
	 *	@return	T*	見つからなければnullptrを返す	
	 */
	T* GetComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "クラス T はComponentから派生する必要があります。");

		// 同じ型のコンポーネントを取得
		for (auto& comp : this->components)
		{
			if (auto casted = dynamic_cast<T*>(comp.get())) 
			{
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
		static_assert(std::is_base_of<Component, T>::value, "クラス T はComponentから派生する必要があります。");

		for (auto it = this->components.begin(); it != this->components.end(); ++it)
		{
			if (auto casted = dynamic_cast<T*>(it->get()))
			{
				// Updatable/Drawable リストからも削除
				this->updatableComponents.erase(
					std::remove(this->updatableComponents.begin(), this->updatableComponents.end(), dynamic_cast<IUpdatable*>(casted)),
					this->updatableComponents.end()
				);
				this->drawableComponents.erase(
					std::remove(this->drawableComponents.begin(), this->drawableComponents.end(), dynamic_cast<IDrawable*>(casted)),
					this->drawableComponents.end()
				);

				// コンポーネントを削除する
				(*it)->Dispose();
				this->components.erase(it);
				break;
			}
		}
	}

	/**	@brief リソース関連の参照を取得する
	 *	@return EngineServices*
	 */
	const EngineServices* Services() const { return this->services; }

	/**	@brief リソース関連の参照を設定
	 *	@param  const EngineServices* _services
	 */
	void SetServices(const EngineServices* _services) { this->services = _services; }

	/**	@brief	親オブジェクトの取得
	 *	@return	GameObject*
	 */
	[[nodiscard]] GameObject* Parent() const { return this->parent; }

	/**	@brief	オブジェクト固有の時間スケールコンポーネントを取得
	 *	@return	TimeScaleComponent*
	 */
	[[nodiscard]] TimeScaleComponent* TimeScale() const { return this->timeScaleComponent; }

public:
		Transform* transform;	///< 位置、回転、スケール情報
private:
	IGameObjectObserver& gameObjectObs;			///< GameObjectの状態を通知するObserver
	const EngineServices* services = nullptr;	///< リソース関連の参照
	TimeScaleComponent* timeScaleComponent;		///< オブジェクト固有の時間スケールコンポーネント

	bool isPendingDestroy;	///< オブジェクトの削除フラグ
	bool isActive;			///< オブジェクトの有効/無効フラグ

	GameObject* parent;	///< 親オブジェクト
	std::string name;	///< オブジェクト名
	GameTags::Tag tag;	///< タグ名

	std::vector<GameObject*> children;						///< 子オブジェクトのリスト
	std::list<std::unique_ptr<Component>>	components;		///< コンポーネントのリスト
	std::vector<IUpdatable*>	updatableComponents;		///< 更新関連のコンポーネントのリスト
	std::vector<IDrawable*>		drawableComponents;			///< 描画関連のコンポーネントのリスト
};