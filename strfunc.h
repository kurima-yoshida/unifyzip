//������Ɋւ���֐��Q

/*
���{�ꕶ���R�[�h�iShift-JIS JIS��3�A��4�����j�̏ꍇ
��1�o�C�g�͈̔͂� 0x81 �` 0x9F�A0xE0 �` 0xFC
��2�o�C�g�͈̔͂� 0x40 �` 0xFC

�����Œ��ڂ��ׂ��_��
������������ɂ����Ă͑�1�o�C�g�A��2�o�C�g�͂������
����R�[�h�i�^�u��X�y�[�X�܂ށj��
�ꕔ�̋L���i!"#$%&'()*+,-./0123456789:;<=>?@�j
�Ɠ����ɂȂ�Ȃ��Ƃ�������
*/

#ifndef __STRFUNC_H__
#define __STRFUNC_H__

#include "types.h"

namespace KSDK {

LPCTSTR strchrex(LPCTSTR pszSrc, TCHAR c);
LPCTSTR strrchrex(LPCTSTR pszSrc, TCHAR c);
LPCTSTR strstrex(LPCTSTR pszString1, LPCTSTR pszString2);

// ������̒����當���񂪍Ō�Ɍ��������ʒu��Ԃ�
LPTSTR strrstrex(LPCTSTR pszString1, LPCTSTR pszString2);

// 0xFF���Ƃ�C����̐��l�萔��int�ɕϊ�
int StringToInteger(LPCTSTR psz);

// �Q�o�C�g�����̂P�o�C�g�ڂ��ǂ������肷��
// �iIsDBCSLeadByte()�ő�p�ł���炵���j
inline bool IsKanji(TCHAR c)
{
#ifdef _UNICODE
	return false;
#else
	UINT8 n = c;
	return (0x81 <= n && n <= 0x9F) || (0xE0 <= n && n <= 0xFC);
#endif
}

// �A���t�@�x�b�g���ǂ������肷��
// ���C�u������ isalpha() �͍ŏ�ʃr�b�g�̒l�ɂ���Ă͋��������������̂Ŏ���
inline bool IsAlpha(TCHAR c)
{
	return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

// �������ǂ������肷��
inline bool IsDigit(TCHAR c)
{
	return ('0' <= c && c <= '9');
}

// �p�������ǂ������肷��
inline bool IsAlNum(TCHAR c)
{
	return (IsAlpha(c) || IsDigit(c));
}

bool IsUpper(LPCTSTR psz);
void MakeUpper(LPTSTR psz);

} // end of namespace KSDK

#endif //__STRFUNC_H__