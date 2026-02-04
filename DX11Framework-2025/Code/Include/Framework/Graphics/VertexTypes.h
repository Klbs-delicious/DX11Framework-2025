/** @file   VertexTypes.h
 *  @brief  頂点関連の型定義
 *  @date   2026/01/24
 */
#pragma once
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Utils/CommonTypes.h"

//-----------------------------------------------------------------------------
// Namespace : Graphics
//-----------------------------------------------------------------------------
namespace Graphics
{
	/** @brief GPU用モデル頂点構造体
     */
    struct ModelVertexGPU
    {
         DirectX::XMFLOAT3 position;
         DirectX::XMFLOAT3 normal;
         DirectX::XMFLOAT2 texcoord;

        UINT  boneIndex[4] = { 0,0,0,0 };
        float boneWeight[4] = { 0,0,0,0 };
    };
} // namespace Graphics
