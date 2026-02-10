#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Graphics/AnimationData.h"

void Graphics::Import::Pose::ResetForSkeleton(const SkeletonCache& _skeletonCache)
{
    const size_t nodeCount = _skeletonCache.nodes.size();

    this->globalMatrices.assign(nodeCount, DX::Matrix4x4::Identity);
    this->skinMatrices.assign(nodeCount, DX::Matrix4x4::Identity);

    // GPU用配列もリセット
    for (size_t i = 0; i < ShaderCommon::MaxBones; i++)
    {
        this->cpuBoneMatrices[i] = DX::Matrix4x4::Identity;
    }
}

void Graphics::Import::Pose::BuildFromLocalPose(const SkeletonCache& _skeletonCache, const Graphics::Animation::LocalPose& _localPose)
{
    const size_t nodeCount = _skeletonCache.nodes.size();
    if (_localPose.localMatrices.size() != nodeCount)
    {
        return;
    }

    this->ResetForSkeleton(_skeletonCache);

    // global を親子合成で埋める（order は親が必ず先）
    // 行ベクトル（mul(v, M)）運用：global = local * parentGlobal
    for (size_t oi = 0; oi < _skeletonCache.order.size(); oi++)
    {
        const int nodeIndex = _skeletonCache.order[oi];
        if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= nodeCount)
        {
            continue;
        }

        const int parentIndex = _skeletonCache.nodes[nodeIndex].parentIndex;

        if (parentIndex < 0)
        {
            this->globalMatrices[nodeIndex] = _localPose.localMatrices[nodeIndex];
        }
        else
        {
            this->globalMatrices[nodeIndex] =
                _localPose.localMatrices[nodeIndex] *
                this->globalMatrices[parentIndex];
        }
    }

    //----------------------------------------------
    // スキン行列＆GPU配列構築
    //----------------------------------------------
    // boneIndex を持つノードだけ skinMatrices と cpuBoneMatrices を埋める
    for (size_t i = 0; i < nodeCount; i++)
    {
        const int boneIndex = _skeletonCache.nodes[i].boneIndex;
        if (boneIndex < 0)
        {
            continue;
        }

        if (static_cast<size_t>(boneIndex) >= _skeletonCache.boneOffset.size())
        {
            continue;
        }

        // 行ベクトル運用
        const DX::Matrix4x4 skin =
            _skeletonCache.boneOffset[static_cast<size_t>(boneIndex)] *
            this->globalMatrices[i] *
            _skeletonCache.globalInverse;

        this->skinMatrices[i] = skin;

        // cpuBoneMatrices は「転置なしのCPU行列」を入れる
        if (boneIndex < static_cast<int>(ShaderCommon::MaxBones))
        {
            this->cpuBoneMatrices[static_cast<size_t>(boneIndex)] = skin;
        }
    }

    //// デバッグ出力
    //Graphics::Debug::Output::DumpBindPoseGlobalCheckOnce(_skeletonCache, *this);
    //Graphics::Debug::Output::DumpBindPoseSkinCheckOnce(_skeletonCache, *this);
}
