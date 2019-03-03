/*------------------------------------------------------------------------------
	File:	Profile.cpp
												2003.10.19 (c)YOSHIDA Kurima
================================================================================
------------------------------------------------------------------------------*/

#include "Profile.h"

#include <tchar.h>

#include "PathFunc.h"

namespace KSDK {

Profile::Profile()
{
}

Profile::Profile(LPCTSTR pszFilename)
{
	Open(pszFilename);
}

Profile::~Profile()
{
	Close();
}

void Profile::Open(LPCTSTR pszFilename)
{
	m_Filename = pszFilename;

	// �t���p�X�łȂ��Ǝ��s����悤�Ȃ̂Ńt���p�X�ɕϊ�
	GetFullPath(m_Filename);
}

void Profile::Close() {
	m_Filename.Empty();
}


bool Profile::WriteInt(LPCTSTR pszAppName, LPCTSTR pszKeyName, int n)
{
	TCHAR szKey[16];
	_itot_s(n, szKey, 10);
	return WriteString(pszAppName, pszKeyName, szKey);
}

bool Profile::WriteString(LPCTSTR pszAppName, LPCTSTR pszKeyName, LPCTSTR pszString)
{
	// �������݂Ɏ��s������t�@�C�������𒲂ׂă��g���C
	if(WritePrivateProfileString(pszAppName, pszKeyName, pszString, m_Filename.c_str()) != 0){
		return true;
	}else{
		DWORD dwAttribute;
		if((dwAttribute = GetFileAttributes(m_Filename.c_str())) == 0xFFFFFFFF) return false;
		if((dwAttribute & FILE_ATTRIBUTE_READONLY) == 0) return false;
		SetFileAttributes(m_Filename.c_str(), dwAttribute & ~FILE_ATTRIBUTE_READONLY);
		return (WritePrivateProfileString(pszAppName, pszKeyName, pszString, m_Filename.c_str()) != 0);
	}
}

int Profile::GetInt(LPCTSTR pszAppName, LPCTSTR pszKeyName, int nDefault)
{
	return GetPrivateProfileInt(pszAppName, pszKeyName, nDefault, m_Filename.c_str());
}

// �T�C�Y�����Ȃ��Ō��ʂ� String �ɕԂ� GetPrivateProfileString()
// pszAppName �� pszKeyName �� NULL �w�肷��g�����͑z�肵�Ă��Ȃ�
void Profile::GetString(LPCTSTR pszAppName, LPCTSTR pszKeyName, LPCTSTR pszDefault, String& returnedString)
{
	DWORD dwSize;
	DWORD dwRet;
	LPTSTR pszString;
	dwSize = 256;
	for(;;){
		pszString = new TCHAR [dwSize];
		// �֐�����������ƃo�b�t�@�Ɋi�[���ꂽ���������Ԃ� (�I�[�� NULL �����͊܂܂Ȃ�) 
		// �o�b�t�@�̃T�C�Y������������ƕ�����͓r���Ő؂�̂Ă��
		// ������̏I�[�� NULL �����ɂȂ� �߂�l�� nSize-1 �ɂȂ�
		dwRet = GetPrivateProfileString(pszAppName, pszKeyName, pszDefault, pszString, dwSize, m_Filename.c_str());
		if(dwRet == dwSize - 1){
			delete pszString;
			dwSize*=2;
		}else{
			break;
		}
	}
	returnedString.Copy(pszString);
	delete [] pszString;
}

} // end of namespace KSDK


