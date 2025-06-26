/**	@file	DebugHooks.h
*	@brief 	Debug、Release時に使用するハンドラをまとめた
*	@date	2025/06/25
*/
#pragma once

/** @namespace DebugHooks
 *  @brief デバッグ用のハンドラーと例外キャッチャー群
 */
namespace DebugHooks
{
    /** @brief      デバッグ用のセットアップ
     *  @details    ビルド構成に応じて使用するハンドラを登録する
     */
    void Install();
}