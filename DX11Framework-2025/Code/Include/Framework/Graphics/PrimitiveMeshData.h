/** @file   PrimitiveMeshData.h
 *  @brief  プリミティブメッシュの頂点・インデックスデータ定義（スムーズ法線対応）
 *  @date   2025/11/10
 */
#pragma once
#include "Include/Framework/Graphics/Mesh.h"

#include <vector>
#include <cmath>

namespace Graphics::Primitives
{
    //-----------------------------------------------------------------------------
    // 内部ユーティリティ
    //-----------------------------------------------------------------------------
    /// @brief ベクトルを正規化する（DX::Vector3版）
    inline DX::Vector3 Normalize(const DX::Vector3& v)
    {
        float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (len <= 1e-6f) return { 0, 0, 0 };
        float inv = 1.0f / len;
        return { v.x * inv, v.y * inv, v.z * inv };
    }

    /** @enum PrimitiveType
     *  @brief 生成可能なプリミティブ形状の種類
     */
    enum class PrimitiveType
    {
        Box,
        Sphere,
        Capsule,
        Plane,
    };

    //-----------------------------------------------------------------------------
    // @namespace Box
    // @brief     スムーズ法線を持つ立方体メッシュデータ（1x1x1）
    //-----------------------------------------------------------------------------
    namespace Box
    {
        inline const std::vector<ModelVertexGPU> Vertices =
        {
            {{-0.5f, -0.5f, -0.5f}, Normalize({-1, -1, -1}), {0, 1}},
            {{ 0.5f, -0.5f, -0.5f}, Normalize({ 1, -1, -1}), {1, 1}},
            {{ 0.5f,  0.5f, -0.5f}, Normalize({ 1,  1, -1}), {1, 0}},
            {{-0.5f,  0.5f, -0.5f}, Normalize({-1,  1, -1}), {0, 0}},
            {{-0.5f, -0.5f,  0.5f}, Normalize({-1, -1,  1}), {0, 1}},
            {{ 0.5f, -0.5f,  0.5f}, Normalize({ 1, -1,  1}), {1, 1}},
            {{ 0.5f,  0.5f,  0.5f}, Normalize({ 1,  1,  1}), {1, 0}},
            {{-0.5f,  0.5f,  0.5f}, Normalize({-1,  1,  1}), {0, 0}},
        };

        inline const std::vector<uint32_t> Indices =
        {
            0, 1, 2, 2, 3, 0,   // 前
            4, 5, 6, 6, 7, 4,   // 後
            4, 0, 3, 3, 7, 4,   // 左
            1, 5, 6, 6, 2, 1,   // 右
            3, 2, 6, 6, 7, 3,   // 上
            4, 5, 1, 1, 0, 4    // 下
        };
    }

    //-----------------------------------------------------------------------------
    // @namespace Sphere
    //-----------------------------------------------------------------------------
    namespace Sphere
    {
        inline constexpr uint32_t DefaultSegments = 16;
        inline constexpr uint32_t DefaultRings = 16;

        inline std::vector<ModelVertexGPU> CreateVertices(uint32_t _segments = DefaultSegments, uint32_t _rings = DefaultRings)
        {
            std::vector<ModelVertexGPU> vertices;
            vertices.reserve((_rings + 1) * (_segments + 1));

            const float PI = 3.14159265359f;
            for (uint32_t y = 0; y <= _rings; ++y)
            {
                float v = static_cast<float>(y) / static_cast<float>(_rings);
                float theta = v * PI;

                for (uint32_t x = 0; x <= _segments; ++x)
                {
                    float u = static_cast<float>(x) / static_cast<float>(_segments);
                    float phi = u * (2.0f * PI);

                    float sinTheta = std::sin(theta);
                    float cosTheta = std::cos(theta);
                    float sinPhi = std::sin(phi);
                    float cosPhi = std::cos(phi);

                    DX::Vector3 pos = { 0.5f * sinTheta * cosPhi, 0.5f * cosTheta, 0.5f * sinTheta * sinPhi };
                    DX::Vector3 normal = Normalize({ sinTheta * cosPhi, cosTheta, sinTheta * sinPhi });
                    DX::Vector2 uv = { u, 1.0f - v };
                    vertices.push_back({ pos, normal, uv });
                }
            }
            return vertices;
        }

        inline std::vector<uint32_t> CreateIndices(uint32_t _segments = DefaultSegments, uint32_t _rings = DefaultRings)
        {
            std::vector<uint32_t> indices;
            indices.reserve(_rings * _segments * 6);

            for (uint32_t y = 0; y < _rings; ++y)
            {
                for (uint32_t x = 0; x < _segments; ++x)
                {
                    uint32_t i0 = y * (_segments + 1) + x;
                    uint32_t i1 = i0 + 1;
                    uint32_t i2 = i0 + (_segments + 1);
                    uint32_t i3 = i2 + 1;
                    indices.insert(indices.end(), { i0, i2, i1, i1, i2, i3 });
                }
            }
            return indices;
        }
    }

    //-----------------------------------------------------------------------------
    // @namespace Plane
    //-----------------------------------------------------------------------------
    namespace Plane
    {
        inline const std::vector<ModelVertexGPU> Vertices =
        {
            {{-0.5f, 0.0f, -0.5f}, {0, 1, 0}, {0, 1}},
            {{ 0.5f, 0.0f, -0.5f}, {0, 1, 0}, {1, 1}},
            {{ 0.5f, 0.0f,  0.5f}, {0, 1, 0}, {1, 0}},
            {{-0.5f, 0.0f,  0.5f}, {0, 1, 0}, {0, 0}},
        };

        inline const std::vector<uint32_t> Indices = { 0, 1, 2, 2, 3, 0 };
    }

    //-----------------------------------------------------------------------------
    // @namespace Capsule
    //-----------------------------------------------------------------------------
    namespace Capsule
    {
        inline constexpr float DefaultRadius = 0.5f;
        inline constexpr float DefaultHeight = 2.0f;
        inline constexpr uint32_t DefaultSegments = 16;
        inline constexpr uint32_t DefaultRings = 8;

        inline std::vector<ModelVertexGPU> CreateVertices(
            uint32_t _segments = DefaultSegments,
            uint32_t _rings = DefaultRings,
            float _radius = DefaultRadius,
            float _height = DefaultHeight)
        {
            std::vector<ModelVertexGPU> vertices;
            const float PI = 3.14159265359f;
            const float halfCylinderHeight = _height * 0.5f - _radius;

            // --- 上半球 ---
            for (uint32_t y = 0; y <= _rings; ++y)
            {
                float v = static_cast<float>(y) / static_cast<float>(_rings);
                float theta = (v * 0.5f) * PI;
                for (uint32_t x = 0; x <= _segments; ++x)
                {
                    float u = static_cast<float>(x) / static_cast<float>(_segments);
                    float phi = u * (2.0f * PI);
                    float sinTheta = std::sin(theta);
                    float cosTheta = std::cos(theta);
                    float sinPhi = std::sin(phi);
                    float cosPhi = std::cos(phi);
                    DX::Vector3 normal = Normalize({ sinTheta * cosPhi, cosTheta, sinTheta * sinPhi });
                    DX::Vector3 pos = { normal.x * _radius, normal.y * _radius + halfCylinderHeight, normal.z * _radius };
                    DX::Vector2 uv = { u, 1.0f - (v * 0.5f) };
                    vertices.push_back({ pos, normal, uv });
                }
            }

            // --- 円柱部 ---
            for (uint32_t y = 0; y <= 1; ++y)
            {
                float height = (y == 0) ? -halfCylinderHeight : halfCylinderHeight;
                float v = (y == 0) ? 0.5f : 0.5f + 0.5f;
                for (uint32_t x = 0; x <= _segments; ++x)
                {
                    float u = static_cast<float>(x) / static_cast<float>(_segments);
                    float phi = u * (2.0f * PI);
                    float sinPhi = std::sin(phi);
                    float cosPhi = std::cos(phi);
                    DX::Vector3 normal = Normalize({ cosPhi, 0, sinPhi });
                    DX::Vector3 pos = { _radius * cosPhi, height, _radius * sinPhi };
                    DX::Vector2 uv = { u, v };
                    vertices.push_back({ pos, normal, uv });
                }
            }

            // --- 下半球 ---
            for (uint32_t y = 0; y <= _rings; ++y)
            {
                float v = static_cast<float>(y) / static_cast<float>(_rings);
                float theta = (v * 0.5f) * PI + (PI / 2.0f);
                for (uint32_t x = 0; x <= _segments; ++x)
                {
                    float u = static_cast<float>(x) / static_cast<float>(_segments);
                    float phi = u * (2.0f * PI);
                    float sinTheta = std::sin(theta);
                    float cosTheta = std::cos(theta);
                    float sinPhi = std::sin(phi);
                    float cosPhi = std::cos(phi);
                    DX::Vector3 normal = Normalize({ sinTheta * cosPhi, cosTheta, sinTheta * sinPhi });
                    DX::Vector3 pos = { normal.x * _radius, normal.y * _radius - halfCylinderHeight, normal.z * _radius };
                    DX::Vector2 uv = { u, 1.0f - (0.5f + v * 0.5f) };
                    vertices.push_back({ pos, normal, uv });
                }
            }

            return vertices;
        }

        inline std::vector<uint32_t> CreateIndices(uint32_t _segments = DefaultSegments, uint32_t _rings = DefaultRings)
        {
            std::vector<uint32_t> indices;
            const uint32_t ringStride = _segments + 1;
            const uint32_t topStart = 0;
            const uint32_t cylStart = (_rings + 1) * ringStride;
            const uint32_t bottomStart = cylStart + (2) * ringStride;

            auto AddQuad = [&](uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3)
                {
                    indices.insert(indices.end(), { i0, i2, i1, i1, i2, i3 });
                };

            for (uint32_t y = 0; y < _rings; ++y)
                for (uint32_t x = 0; x < _segments; ++x)
                    AddQuad(topStart + y * ringStride + x, topStart + y * ringStride + x + 1,
                        topStart + (y + 1) * ringStride + x, topStart + (y + 1) * ringStride + x + 1);

            for (uint32_t y = 0; y < 1; ++y)
                for (uint32_t x = 0; x < _segments; ++x)
                    AddQuad(cylStart + y * ringStride + x, cylStart + y * ringStride + x + 1,
                        cylStart + (y + 1) * ringStride + x, cylStart + (y + 1) * ringStride + x + 1);

            for (uint32_t y = 0; y < _rings; ++y)
                for (uint32_t x = 0; x < _segments; ++x)
                    AddQuad(bottomStart + y * ringStride + x, bottomStart + y * ringStride + x + 1,
                        bottomStart + (y + 1) * ringStride + x, bottomStart + (y + 1) * ringStride + x + 1);

            return indices;
        }
    }
} // namespace Graphics::Primitives