/** @file   ShaderCommon.cpp
*   @date   2025/10/19
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Shaders/ShaderCommon.h"
#include"Include/Framework/Shaders/ShaderBase.h"


/**	@brief シェーダーのバインドを行う
 *	@param ID3D11DeviceContext& _ctx
 */
void ShaderProgram::Bind(ID3D11DeviceContext& _ctx) const
{
	if (this->vs) { this->vs->Bind(_ctx); }
	if (this->ps) { this->ps->Bind(_ctx); }
}