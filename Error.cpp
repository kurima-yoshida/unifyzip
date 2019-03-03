#include "Error.h"


#include <stdio.h>
#define ANSI            /* UNIX �o�[�W�����ł̓R�����g �A�E�g���Ă��������B */
#ifdef ANSI             /* ANSI �݊��o�[�W���� */
#include <stdarg.h>
int average( int first, ... );
#else                   /* UNIX �݊��o�[�W���� */
#include <varargs.h>
int average( va_list );
#endif

#include <Windows.h>

#include <time.h>		// debug�p�ɓ��t�����̏o�͂�����̂�

#include <tchar.h>



bool bOutputErrorlog = 
#ifdef _DEBUG
	true;  // �f�o�b�O���[�h�ł̓f�t�H���g��true
#else
	false; // �����[�X���[�h�ł̓f�t�H���g��false
#endif

TCHAR szErrorMessage[256];



void EnableOutputErrorlog(bool b)
{
	bOutputErrorlog = b;
}



void InnerLog(LPCTSTR szFormat, ...)
{
	TCHAR szDate[256];
	TCHAR szTime[256];
	TCHAR szText[256];

	_tstrdate(szDate);// ���t
	_tstrtime(szTime);// ����

	va_list marker;	// �ό������X�g�̐擪������
	va_start(marker, szFormat);
	_vstprintf(szText, szFormat, marker);
	va_end(marker);

	_stprintf(szErrorMessage,_T("%s %s %s\n"), szDate, szTime, szText);

	if (!bOutputErrorlog) return;

	//�f�o�b�K�ɕ������o��	
	OutputDebugString(szErrorMessage);
}



// �G���[���b�Z�[�W�̕\��
void SDKGetLastErrorMessage()
{
	// �������\������B
	MessageBox(NULL, szErrorMessage, _T("Error"), MB_OK | MB_ICONINFORMATION);
}



// �G���[���b�Z�[�W�̕\��
void GetLastErrorMssage()
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ����̌���
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		);

	// �������\������B
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION);

	// �o�b�t�@���������B
	LocalFree(lpMsgBuf);
}
