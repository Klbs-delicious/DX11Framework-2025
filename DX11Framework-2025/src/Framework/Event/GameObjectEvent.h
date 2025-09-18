﻿/**	@file   GameObjectEvent.h
 *	@brief	ゲームオブジェクト専用の通知処理 
 *	@date   2025/09/18
 */
#pragma once
class GameObject;

/**	@enum	GameObjectEvent
 *	@brief	GameObjectに関する軽量イベント種別
 */
enum class GameObjectEvent {
	Refreshed,
	Destroyed,
	Initialized
};

 /** @class IGameObjectObserver
  *  @brief GameObjectの状態変化を監視するインターフェース
  */
class IGameObjectObserver {
public:
	/**@brief GameObjectからのイベント通知を受け取る
	 * @param GameObject* _obj			通知元のGameObjectインスタンス
	 * @param GameObjectEvent _event	発生したイベント種別
	 * @detail	
	 *	-	GameObject が状態変化した際に呼び出される
	 *	-	実装側では event の種類に応じて処理を分岐させる
	 */
	virtual void OnGameObjectEvent(GameObject* _obj, GameObjectEvent _event) = 0;

	/// @brief 仮想デストラクタ
	virtual ~IGameObjectObserver() = default;
};
