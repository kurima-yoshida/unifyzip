/*------------------------------------------------------------------------------
	File:	Profile.h
												2003.10.19 (c)YOSHIDA Kurima
================================================================================
 Content:
	Profile�N���X
	
 Notes:

 Sample:

------------------------------------------------------------------------------*/

#ifndef __PROFILE_H__
#define __PROFILE_H__



#include "String.h"

namespace KSDK {

class Profile {
public:
	Profile();						// �f�t�H���g�R���X�g���N�^
	Profile(LPCTSTR pszFilename);	// �R���X�g���N�^
	virtual ~Profile();			// �f�X�g���N�^

	void Open(LPCTSTR pszFilename);
	void Close();

	bool WriteInt(LPCTSTR pszAppName, LPCTSTR pszKeyName, int n);
	bool WriteString(LPCTSTR pszAppName, LPCTSTR pszKeyName, LPCTSTR pszString);

	int GetInt(LPCTSTR pszAppName, LPCTSTR pszKeyName, int nDefault);
	void GetString(LPCTSTR pszAppName, LPCTSTR pszKeyName, LPCTSTR pszDefault, String& returnedString);

private:
	Profile(const Profile &Profile);					// �R�s�[�R���X�g���N�^���ĂׂȂ��悤�� private �錾
	const Profile& operator=(const Profile& Profile);	// ������Z���ĂׂȂ��悤�� private �錾

	String m_Filename;
};

} // end of namespace KSDK

#endif //__PROFILE_H__