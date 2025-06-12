/**	@file	NonCopyable.h
*	@date	2025/06/12
*/
#pragma once

/**@class	NonCopyable
 * @brief	�R�s�[���֎~����
 */
class NonCopyable 
{
public:
    /**@brief �f�t�H���g�R���X�g���N�^
     */
    NonCopyable() = default;

    /**@brief �R�s�[�R���X�g���N�^���폜����
     * @details ���̃N���X�̃I�u�W�F�N�g�̃R�s�[�͋�����Ă��Ȃ����߁A�R�s�[�R���X�g���N�^�͍폜����Ă���
     */
    NonCopyable(const NonCopyable&) = delete;

    /**@brief �R�s�[������Z�q���폜����
     * @details ���̃N���X�̃I�u�W�F�N�g�̑���͋�����Ă��Ȃ����߁A�R�s�[������Z�q�͍폜����Ă���
     */
    NonCopyable& operator=(const NonCopyable&) = delete;
};