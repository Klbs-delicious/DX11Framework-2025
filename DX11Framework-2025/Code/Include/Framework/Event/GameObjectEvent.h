/**	@file   GameObjectEvent.h
 *	@brief	ゲームオブジェクト専用の通知処理 
 *	@date   2025/09/18
 */
#pragma once
#include<string>

class GameObject;
class Component;

/**	@enum	GameObjectEvent
 *	@brief	GameObjectに関する軽量イベント種別
 */
enum class GameObjectEvent {
	Destroyed,
	GameObjectEnabled,
	GameObjectDisabled,
	ComponentEnabled,
	ComponentDisabled,
	ComponentAdded,
	ComponentRemoved
};

 /** @struct GameObjectEventContext
  *	 @brief	 GameObjectイベントのコンテキスト情報
  */
struct GameObjectEventContext
{
	std::string objectName;		///< イベント発生元のオブジェクト名
	Component* component;		///< 関連するコンポーネント（該当する場合）
	GameObjectEvent eventType;	///< 発生したイベント種別
};

 /** @class IGameObjectObserver
  *  @brief GameObjectの状態変化を監視するインターフェース
  */
class IGameObjectObserver {
public:

	/**@brief GameObjectからのイベント通知を受け取る（コンテキスト版）
	 * @param GameObjectEventContext _eventContext	イベントコンテキスト情報
	 * @detail	
	 *	-	GameObject が状態変化した際に呼び出される
	 *	-	実装側では eventContext.eventType の種類に応じて処理を分岐させる
	 */
	virtual void OnGameObjectEvent(const GameObjectEventContext _eventContext) = 0;

	/// @brief 仮想デストラクタ
	virtual ~IGameObjectObserver() = default;
};