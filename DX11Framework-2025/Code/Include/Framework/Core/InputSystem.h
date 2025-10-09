/**	@file	InputSystem.h
*	@date	2025/09/14
*/
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"
#include"Include/Framework/Core/IInputDevice.h"

#include<string>
#include<memory>
#include<unordered_map>
#include<vector>

/** @class	InputSystem
 *	@brief	入力管理を行う
 *	@details
 *  - このクラスはコピー、代入を禁止している
 */
class InputSystem :private NonCopyable
{
public:
	InputSystem();
	~InputSystem();

	/// @brief	リソースの解放処理
	void Dispose();

	/**@struct	KeyBinding
	 *	@brief	キーバインドを定義する構造体
	 */
	struct KeyBinding {
		int keyCode;             ///< キー、ボタンID
		std::string actionName;  ///< キーアクション名
	};
	
	/**	@brief	デバイスの登録
	　*  @param std::unique_ptr<IInputDevice>	_device 登録するデバイス（std::unique_ptr による所有権移動）
	 */
	void RegisterDevice(std::unique_ptr<IInputDevice> _device);

	/**	@brief	キーバインドの登録
	 *	@param	const std::string& _action アクション名
	 *	@param	int _keyCode	キーボタンの入力コード
	 */
	void RegisterKeyBinding(const std::string& _action, int _keyCode);

	/// @brief	入力デバイスの更新
	void Update();

	/**	@brief	アクションに対応したキー、ボタンが押された状態か取得する
	 *	@param	const std::string& _action	キーアクション名（例: "Jump"）
	 *  @return bool いずれかのデバイスで押されていれば true
	 */
	bool IsActionPressed(const std::string& _action) const;

	/**	@brief	アクションに対応したキー、ボタンがトリガー状態か取得する
	 *	@param	const std::string& _action	キーアクション
	 * @return bool 前フレームから押された瞬間であれば true
	 */
	bool IsActionTriggered(const std::string& _action) const;

private:
	std::vector<std::unique_ptr<IInputDevice>> devices;	///< 入力デバイスのリスト
	std::unordered_map<std::string, int> keyMap;		///< アクションに対応したキーマップ
};
