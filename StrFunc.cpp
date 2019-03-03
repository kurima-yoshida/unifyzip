#include "StrFunc.h"

#include <tchar.h>

namespace KSDK {

// �Q�o�C�g�������܂ޕ�����̒�����w��̕����������A�ʒu��Ԃ��B
LPCTSTR strchrex(LPCTSTR pszSrc, TCHAR c)
{
#ifdef _UNICODE
	// Unicode ���ƃ��C�u�����֐����g����
	return _tcschr(pszSrc, c);
#else
	for ( ; *pszSrc; pszSrc++) {
		if (IsKanji(*pszSrc)) {
			pszSrc++;
			continue;
		}
		if (*pszSrc == c) {
			return (char*)pszSrc;
		}
	}
	return NULL;
#endif
}



// �Q�o�C�g�������܂ޕ�����̒�����w��̕�������납�猟���A�ʒu��Ԃ��B
LPCTSTR strrchrex(LPCTSTR pszSrc, TCHAR c)
{
	// �������A���ۂ͌�납��T�����Ƃ͕s�\
	// 2�o�C�g������1�o�C�g�ڂ�2�o�C�g������2�o�C�g�ڂɂ��g���邱�Ƃ�����̂ŁB
	// ����āA�O�����猟�����Ă����A�Ō�Ɍ��������̂�Ԃ������Ȃ��B
#ifdef _UNICODE
	// Unicode ���ƃ��C�u�����֐����g����
	return _tcsrchr(pszSrc, c);
#else
	LPTSTR pszHit = NULL;
	for ( ; *pszSrc; pszSrc++) {
		if (IsKanji(*pszSrc)) {
			pszSrc++;
			continue;
		}
		if (*pszSrc == c) {
			pszHit = (LPTSTR)pszSrc;
		}
	}
	return pszHit;
#endif
}



// �Q�o�C�g�������܂ޕ�����̒�����w��̕�����������A�ʒu��Ԃ��B
LPCTSTR strstrex(LPCTSTR pszString1, LPCTSTR pszString2) {
#ifdef _UNICODE
	// Unicode ���ƃ��C�u�����֐����g����
	return _tcsstr(pszString1, pszString2);
#else
	int nLen1 = (int)_tcslen(pszString1);
	int nLen2 = (int)_tcslen(pszString2);
	int nSub = nLen1-nLen2;

	for(int i=0; i<=nSub; i++){
		if(strncmp(pszString1+i,pszString2,nLen2) == 0){
			return (char*)(pszString1+i);
		}else{
			if(IsKanji(*(pszString1+i))) i++;
		}
	}
	return NULL;
#endif
}



// ������̒����當���񂪍Ō�Ɍ��������ʒu��Ԃ�
LPTSTR strrstrex(LPCTSTR pszString1, LPCTSTR pszString2) {
	// �������A���ۂ͌�납��T�����Ƃ͕s�\
	// 2�o�C�g������1�o�C�g�ڂ�2�o�C�g������2�o�C�g�ڂɂ��g���邱�Ƃ�����̂ŁB
	// ����āA�O�����猟�����Ă����A�Ō�Ɍ��������̂�Ԃ������Ȃ��B
	LPTSTR  pszHit = NULL;
	for ( ; *pszString1; pszString1++) {

		if (*pszString1 == *pszString2 && _tcscmp(pszString1, pszString2) == 0) {
			pszHit = (LPTSTR)pszString1;
		}
		if (IsKanji(*pszString1)) pszString1++;
	}
	return pszHit;
}



// 0xFF���Ƃ�C����̐��l�萔��int�ɕϊ�
int StringToInteger(LPCTSTR psz)
{
	TCHAR c;
	int n;

	n = 0;
	c = *psz;

	if(!IsDigit(c)) return 0;

	// �萔
	if(c == '0'){
		// 0 �� 8�i�� �� 16�i��
		
		c = *(psz+1);
		if('0' <= c && c <= '7'){
			for(;;){
				c = *psz++;
				if('0' <= c && c <= '7'){
					n *= 8;
					n += (c - '0');
				}else{
					return n;
				}
			}
		}else if(c == 'x' || c == 'X'){
			psz+=2;// psz��0x�̎�������
			for(;;){
				c = *psz++;
				if(isxdigit(c)){
					n *= 16;
					if(IsDigit(c)){
						n += (c - '0');
					}else{
						if(_istupper(c)){
							n += ((c - 'A') + 10);
						}else{
							n += ((c - 'a') + 10);
						}
					}
				}else{
					return n;
				}
			}
		}else{
			return 0;
		}
	}else{
		// 10�i��
		for(;;){
			c = *psz++;
			if(IsDigit(c)){
				n *= 10;
				n += (c - '0');
			}else{
				return n;
			}
		}
	}
}

// ������̍ŏ��̕��������������ǂ����`�F�b�N����
// �S�p�̃A���t�@�x�b�g���`�F�b�N����
// Shift-JIS�p
bool IsLower(LPCTSTR psz)
{
	TCHAR c;
	c = *psz;

#ifdef _UNICODE
	return (_TCHAR('a') <= c && c <= _TCHAR('z')) || (_TCHAR('��') <= c && c <= _TCHAR('��'));
#else
	if(IsKanji(c)){
		if(c == 0x82){
			c = *(psz + 1);
			return (0x81 <= c && c <= 0x9a);
		}
		return false;
	}else{
		return ('a' <= c && c <= 'z');
	}
#endif
}

// �v����m�F
void MakeUpper(LPTSTR psz)
{
	for(;;){
		if(*psz == '\0') return;
		if(IsLower(psz)){

#ifdef _UNICODE
			if (_TCHAR('a') <= (*psz) && (*psz) <= _TCHAR('z')) {
				*(++psz) += (_TCHAR('A') - _TCHAR('a'));
			} else if (_TCHAR('��') <= (*psz) && (*psz) <= _TCHAR('��')) {
				*(++psz) += (_TCHAR('�`') - _TCHAR('��'));
			}
#else
			if(IsKanji(*psz)){
				*(++psz) += (0x81 - 0x60);
			}else{
				*psz += ('A' - 'a');
			}
#endif

		}
		psz++;
	}
}

} // end of namespace KSDK



