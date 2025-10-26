/**	@file	PhaseInterfaces.h
 *	@brief	更新、描画フェーズをまとめた抽象群
 *	@date	2025/09/16
 */
#pragma once

 /** @class  IUpdatable
  *  @brief  更新フェーズ抽象化クラス
  *  @details - 毎フレーム呼び出される更新処理を定義するためのインターフェース
  */
class IUpdatable
{
public:
	/// @brief デストラクタ
	virtual ~IUpdatable() = default;

	/** @brief  更新処理
	 *  @param float _deltaTime	前フレームからの経過時間（秒）
	 */
	virtual void Update(float _deltaTime) = 0;
};

/** @class	IDrawable
 *	@brief	描画フェーズ抽象化クラス
 *	@details	- 毎フレーム呼び出される描画処理を定義するためのインターフェース
 */
class IDrawable
{
public:
	/// @brief デストラクタ
	virtual ~IDrawable() = default;

	/// @brief 描画処理
	virtual void Draw() = 0;
};
