/** @file   TreeNode.h
 *  @date   2025/10/26
 */
#pragma once
#include<memory>
#include<vector>

namespace Framework::Utils
{
    template <typename T>
    class TreeNode
    {
    public:
        T nodedata;
        TreeNode<T>* parent = nullptr;
        std::vector<std::unique_ptr<TreeNode<T>>> children;

        TreeNode() = default;
        explicit TreeNode(const T& _data) : nodedata(_data) {}

        void Addchild(std::unique_ptr<TreeNode<T>> _child)
        {
            _child->parent = this;
            children.push_back(std::move(_child));
        }
    };
}