#include "CommandLine.h"

#include <tchar.h>

namespace KSDK {

// ���������m�ۂ��R�}���h���C����������R�s�[
// �m�ۂ����������ւ̃|�C���^��Ԃ�
void* AllocCommandLine(void)
{
	LPTSTR psz;
	size_t size;
	void* pv;

	psz = GetCommandLine();
	size = _tcslen(psz) + 1;
	if((pv = malloc(size * sizeof(TCHAR))) == NULL) return NULL;
	_tcscpy((LPTSTR)pv, psz);
	return pv;
}


void FreeCommandLine(void* pvCommandLine)
{
	free(pvCommandLine);
}


// �����̋�؂�͔��p�X�y�[�X�܂���TAB�i���������Ă��j
// �u"�v�ň͂܂ꂽ������Ɋւ��Ă͔��p�X�y�[�X�͋�؂�Ƃ��Ă̈Ӗ���������TAB�͖��������
// DragQueryFile() �̎d�l���Q�l�ɂ���
// iParam
// 0xFFFFFFFF : �����̐���Ԃ�
UINT GetStringFromCommandLine(void* pvCommandLine, UINT iParam, String& Param)
{
	LPTSTR pszSrc;
	TCHAR c;
	int nIndex;		// ���ݏ������̈����̃C���f�b�N�X
	bool bTrim;		// �����̑O�̕s�v�� �X�y�[�X TAB ����蕥������
	bool bDQ;		// �_�u���N�H�[�e�[�V�����ň͂܂�Ă��邩

	if(iParam != 0xFFFFFFFF) Param.Empty();

	pszSrc = (LPTSTR)pvCommandLine;

	nIndex = -1;

	bTrim = false;
	bDQ = false;

	for(;;){
		c = *pszSrc++;

		// NULL�����ŏI��
		if(c == _TCHAR('\0')) break;
		
		if(!bTrim){
			// �R�}���h�̑O�̕s�v�� TAB �X�y�[�X ����蕥���Ă��Ȃ�
			if(c == _TCHAR(' ') || c == _TCHAR('\t')){
				if(iParam != 0xFFFFFFFF && nIndex == (int)iParam){
					break;		// �ړI�̈����̎擾���I������ꍇ
				}else{
					continue;	// �X�y�[�X TAB �͖���
				}
			}else{
				bTrim = true;
				nIndex++;
			}
		}
					
		if(c == _TCHAR('"')){
			bDQ = !bDQ;
		}else{
			if(!bDQ){
				// �_�u���N�H�[�e�[�V�����ł������Ė����ꍇ
				if(c == _TCHAR(' ') || c == _TCHAR('\t')){
					// �����̋�؂�Ƃ��Ă� �X�y�[�X TAB
					bTrim = false;
				}else{
					if(iParam != 0xFFFFFFFF && nIndex == (int)iParam) Param += c;
				}
			}else{
				// �_�u���N�H�[�e�[�V�����ł������Ă���ꍇ
				if(c == _TCHAR('\t')){
					// TAB �͖���
				}else{
					if(iParam != 0xFFFFFFFF && nIndex == (int)iParam) Param += c;
				}
			}
		}
	}
	
	if(iParam == 0xFFFFFFFF){
		return nIndex + 1;			// �����̐���Ԃ�
	}else{
		return Param.GetLength();	// �R�s�[������������Ԃ�
	}
}

} // end of namespace KSDK
