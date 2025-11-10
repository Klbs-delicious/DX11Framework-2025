/** @file   Mesh.cpp
 *  @date   2025/11/01
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/Mesh.h"

//-----------------------------------------------------------------------------
// Mesh Class
//-----------------------------------------------------------------------------
namespace Graphics
{
    /**@brief メッシュをバインド
     * @param _context
     */
    void Mesh::Bind(ID3D11DeviceContext& _context) const
    {
        // 頂点・インデックスを IA ステージに設定
        if (this->vertexBuffer)
        {
            this->vertexBuffer->Bind(&_context);
		}
        if (this->indexBuffer)
        {
            this->indexBuffer->Bind(&_context);
        }
    }
}// namespace Graphics
