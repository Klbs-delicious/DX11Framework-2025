/**	@file	IInputDevice.h
*	@date	2025/09/14
*/
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"

/** @class	IInputDevice
 *	@brief	入力デバイスの抽象クラス
 *	@details
 *  - このクラスはコピー、代入を禁止している
 */
class IInputDevice :private NonCopyable
{
public:
	/// @brief	デストラクタ
	virtual ~IInputDevice() = default;

    /// @brief	@brief 解放処理
    virtual void Dispose() = 0;

	/// @brief	入力の更新
	virtual void Update() = 0;

    /** @brief  押下状態の取得
     *  @param  int _code  入力コード
     *  @return bool 押されていればtrue
     */
    virtual bool IsPressed(int _code) const = 0;

    /** @brief  トリガー状態の取得
     *  @param  int _code  入力コード
     *  @return bool 押されていればtrue
     */
    virtual bool IsTriggered(int code) const = 0;

    /** @brief  マウスX座標の取得
     *  @return int 対応していない場合は無効値
     */
    virtual int GetMouseX() const { return -1; }

    /** @brief  マウスY座標の取得
     *  @return int 対応していない場合は無効値
     */
    virtual int GetMouseY() const { return -1; }

    /** @brief マウスの移動量（Δ）を取得
     *  @param int& _dx X方向の変化量
     *  @param int& _dy Y方向の変化量
     *  @details デフォルトは (0,0) を返す
     */
    virtual void GetMouseDelta(int& _dx, int& _dy) const { _dx = _dy = 0; }

    /** @struct MotorForce
     *  @brief  振動の大きさ
     *	@details - 振動のモーターが単一の場合と左右の場合で分けられるように
     */
    struct MotorForce {
        float left = 0.0f;
        float right = 0.0f;

        MotorForce() = default;
        MotorForce(float both) : left(both), right(both) {}
        MotorForce(float l, float r) : left(l), right(r) {}
    };

    /** @brief  振動の制御
     *  @param  const MotorForce& _morterForce 振動の強さ
     *	@details - 対応していない場合は空実装
     */
    virtual void SetVibration(const MotorForce& _morterForce) {}
};
