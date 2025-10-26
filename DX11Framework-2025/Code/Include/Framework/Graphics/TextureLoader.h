/** @file   TextureLoader.h
 *  @brief  画像データからGPUテクスチャを生成するローダークラス
 *  @date   2025/10/27
 */
#pragma once
#include <string>
#include <memory>

#include "Include/Framework/Graphics/TextureResource.h"

 /**@class TextureLoader
  * @brief ファイルまたはメモリデータからGPU上にテクスチャを作成する
  * @details
  *     - 本クラスは テクスチャの読み込み専用
  *     - 管理・キャッシュは行わない
  *     - SpriteManagerやModelImporterが利用する前提
  */
class TextureLoader
{
public:
    TextureLoader() = default;

    /**
     * @brief 画像ファイルを読み込んでテクスチャを生成する
     * @param[in] _path ファイルパス
     * @return 生成されたTextureResourceのunique_ptr失敗時はnullptr
     */
    std::unique_ptr<TextureResource> FromFile(const std::string& _path) const;

    /**
     * @brief メモリ上の画像データからテクスチャを生成する
     * @param[in] _data 画像データのポインタ
     * @param[in] _len  画像データのバイト数
     * @return 生成されたTextureResourceのunique_ptr。失敗時はnullptr
     */
    std::unique_ptr<TextureResource> FromMemory(const unsigned char* _data, int _len) const;
};
