/*------------------------------------------------------------------------------
	File:	String.h
												2003.08.30 (c)YOSHIDA Kurima
================================================================================
 Content:
	���앶����N���X
	
 Notes:
	MakeMid()�𕶎���̈ړ���������
	������̐擪�̃|�C���^��ς��鎖�ł���Ă���̂ō����Ȃ̂�����

	�܂�Unicode�EANSI���Ή��ł���
	SHIFT-JIS �̂Q�o�C�g�ڂɊւ���Ή����Ȃ���Ă���

	�����̒P�ʂ͑S�ĕ������P��
	�}���`�o�C�g������2�����Ƃ��Ĉ�����̂Œ���

	TODO:
		MakeLeft -> Left�̃`�F�b�N
		format �� Boost ��p����H



 Sample:

------------------------------------------------------------------------------*/

#ifndef __STRING_H__
#define __STRING_H__

#include <windows.h>
#include <memory.h>
#include <string.h>
#include "types.h"

namespace KSDK {

class String {
public:
	String();
	String(LPCTSTR pszSrc);
	String(const String& another);

	~String();

	String& Copy(LPCTSTR pszString);
	String& Copy(int n);
	String& Copy(TCHAR c);

	String& NCopy(LPCTSTR pszString, int n);

	String& Cat(LPCTSTR pszString);
	String& Cat(int n);
	String& Cat(TCHAR c);

	String& NCat(LPCTSTR pszString, int n);

	void Empty(void);
	bool Reserve(int nSize);

	LPCTSTR c_str(void) const {return (LPCTSTR)m_pszStr;}	// ������ւ̃|�C���^��Ԃ�
	bool IsEmpty(void) const {return m_nStrLen == 0;}		// �����񂪋�

	int GetLength(void) const {return m_nStrLen;}			// �����񒷂��i�������P�ʁj

	// �����z��
	TCHAR GetAt(int nIndex) const {return *(m_pszStr + nIndex);}

	// ���o
	int MakeMid(int nFirst);
	int MakeMid(int nFirst, int nCount);
	int MakeLeft(int nCount);

	// ���̑��̕ϊ�	
	void MakeUpper();// �啶����
	void MakeLower();
	int Replace( TCHAR chOld, TCHAR chNew );	// �u��
	int Replace(LPCTSTR pszOld, LPCTSTR pszNew);
	int Insert( int nIndex, TCHAR ch );			// �}��
	int Insert( int nIndex, LPCTSTR pstr );
	int Delete( int nIndex, int nCount = 1 );	// �폜
	void Format( LPCTSTR lpszFormat, ...);
	void FormatV( LPCTSTR lpszFormat, va_list argList);
	void TrimLeft( TCHAR chTarget );
	void TrimLeft( LPCTSTR lpszTargets );
	void TrimRight( TCHAR chTarget );
	void TrimRight( LPCTSTR lpszTargets );


	// ����
	int Find( TCHAR ch, int nStart = 0) const;
	int Find( LPCTSTR lpszSub, int nStart = 0) const;
	int ReverseFind( TCHAR ch) const;
	int FindOneOf( LPCTSTR lpszCharSet ) const;

	// �o�b�t�@ �A�N�Z�X
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);

	// �I�y���[�^

	// �����ϊ�
	//operator LPCTSTR() const {return (LPCTSTR)m_pszStr;}

	String& operator=(const String& String){return Copy(String.c_str());}
	String& operator=(LPCTSTR pszString){return Copy(pszString);}
	String& operator=(int n){return Copy(n);}
	String& operator=(TCHAR c){return Copy(c);}

	// �u[]�v���Z�q
	TCHAR operator []( int nIndex ) const{return GetAt(nIndex);}

	// �u+=�v���Z�q
	const String& operator +=( const String& String ){return Cat(String.c_str());}
	const String& operator +=( LPCTSTR pszString ){return Cat(pszString);}
	const String& operator +=( int n ){return Cat(n);}
	const String& operator +=( TCHAR ch ){return Cat(ch);}

	// ��r���Z�q
	bool operator==(LPCTSTR pszString ) const {return m_pszStr && lstrcmp( m_pszStr, pszString ) == 0; }
	bool operator!=(LPCTSTR pszString ) const {return m_pszStr && lstrcmp( m_pszStr, pszString ) != 0; }
	bool operator< (LPCTSTR pszString ) const {return m_pszStr && lstrcmp( m_pszStr, pszString ) < 0; }
	bool operator> (LPCTSTR pszString ) const {return m_pszStr && lstrcmp( m_pszStr, pszString ) > 0; }
	bool operator<=(LPCTSTR pszString ) const {return m_pszStr && lstrcmp( m_pszStr, pszString ) <= 0; }
	bool operator>=(LPCTSTR pszString ) const {return m_pszStr && lstrcmp( m_pszStr, pszString ) >= 0; }

	bool operator==(const String& String ) const {return m_pszStr && lstrcmp( m_pszStr, String.c_str() ) == 0;}
	bool operator!=(const String& String ) const {return m_pszStr && lstrcmp( m_pszStr, String.c_str() ) != 0;}
	bool operator< (const String& String ) const {return m_pszStr && lstrcmp( m_pszStr, String.c_str() ) < 0;}
	bool operator> (const String& String ) const {return m_pszStr && lstrcmp( m_pszStr, String.c_str() ) > 0;}
	bool operator<=(const String& String ) const {return m_pszStr && lstrcmp( m_pszStr, String.c_str() ) <= 0;}
	bool operator>=(const String& String ) const {return m_pszStr && lstrcmp( m_pszStr, String.c_str() ) >= 0;}

protected:
	// ���ۂɂ͏I�[����������̂� m_nBufLen - 1 ��������
	// ���������g�������Ɋi�[�ł��邱�ƂɂȂ�

	LPTSTR m_pszBuf;	// �m�ۂ��Ă��郁�����ւ̃|�C���^
	LPTSTR m_pszStr;	// ������ւ̃|�C���^
	int m_nMaxBufLen;	// �m�ۂ��Ă��郁�����ʁi�������P�ʁj
	int m_nBufLen;		// �����񂪎g�p�ł��郁�����ʁi�������P�ʁj
	int m_nStrLen;		// ������̒����i�������P�ʁj
};

} // end of namespace KSDK

#endif //__STRING_H__