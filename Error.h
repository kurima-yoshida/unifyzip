/*------------------------------------------------------------------------------
	File:	Error.h
												2003.02.20 (c)YOSHIDA Kurima
===============================================================================
 Content:
	�G���[���O�o�͊֐��Q

 Notes:
	_DEBUG����`����Ă���Ƃ��̂ݗL������
	EnableOutputErrorlog()��Release�����o�͉\

 Sample:

------------------------------------------------------------------------------*/

#ifndef __ERROR_H__
#define __ERROR_H__



// Release�r���h�ł��G���[�̏o�͂�L���ɂ���
void EnableOutputErrorlog(bool b);

// ���b�Z�[�W�ɓ��t�����������ăf�o�b�K�ɏo��
void InnerLog(const char* szFormat, ...);

// InnerLog()�ōŌ�ɏo�͂����������MessageBox()�ŕ\��
void SDKGetLastErrorMssage();

// GetLastError()�����s���G���[�R�[�h�ɑΉ�����G���[�������MessageBox()�ŕ\��
void GetLastErrorMssage();



#endif //__ERROR_H__