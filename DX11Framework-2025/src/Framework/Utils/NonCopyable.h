/**	@file	NonCopyable.h
*	@date	2025/06/12
*/
#pragma once

/**@class	NonCopyable
 * @brief	コピーを禁止する
 */
class NonCopyable 
{
public:
    /**@brief デフォルトコンストラクタ
     */
    NonCopyable() = default;

    /**@brief コピーコンストラクタを削除する
     * @details このクラスのオブジェクトのコピーは許可されていないため、コピーコンストラクタは削除されている
     */
    NonCopyable(const NonCopyable&) = delete;

    /**@brief コピー代入演算子を削除する
     * @details このクラスのオブジェクトの代入は許可されていないため、コピー代入演算子は削除されている
     */
    NonCopyable& operator=(const NonCopyable&) = delete;
};
