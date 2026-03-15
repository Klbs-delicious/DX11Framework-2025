#pragma once
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
