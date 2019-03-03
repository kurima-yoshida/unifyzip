// �o�C�i���t�@�C��

#ifndef __BINARYFILE_H__
#define __BINARYFILE_H__

#include <Windows.h>
#include "types.h"
#include <fstream>

#include "Exception.h"

namespace KSDK {

class BinaryFileWriter {
public:
	BinaryFileWriter() : hFile_(NULL) {}

	bool open(LPCTSTR fileName) {
		hFile_ = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile_ != INVALID_HANDLE_VALUE) {
			return true;
		}
		hFile_ = NULL;

		// �t�@�C���I�[�v���Ɏ��s�����̂ő����ύX�����݂�
		DWORD dwAttribute;
		dwAttribute = GetFileAttributes(fileName);
		if (dwAttribute == 0xFFFFFFFF) {
			return false;
		}
		if ((dwAttribute & FILE_ATTRIBUTE_READONLY) == 0) {
			return false;
		}
		SetFileAttributes(fileName, dwAttribute & ~FILE_ATTRIBUTE_READONLY);

		hFile_ = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile_ != INVALID_HANDLE_VALUE) {
			return true;
		}
		hFile_ = NULL;
		return false;
	}

	void close() {
		if (hFile_ != NULL) {
			CloseHandle(hFile_);
		}
	}

	// �������݂Ɏ��s�������O�𓊂���
	// ��Ƀf�B�X�N�e�ʕs���ŋN����
	bool write(void* p, UINT32 size) {
		DWORD numberOfBytesWritten;
		if (hFile_ != NULL && WriteFile(hFile_, p, size, &numberOfBytesWritten, NULL) != 0) {
		} else {
			return false;
		}
		return true;
	}

	// �t�@�C�����J���Ă��Ȃ��ꍇ�� 0 ��Ԃ�
	// �t�@�C���T�C�Y�� DWORD, int �̂����ꂩ�̍ő�l�𒴂���ꍇ
	// �Ԃ�l�͕s��
	int tell() {
		if (hFile_ == NULL) {
			return 0;
		}
		return (int)GetFileSize(hFile_, NULL);
	}

private:
	BinaryFileWriter(const BinaryFileWriter&);
	BinaryFileWriter& operator=(const BinaryFileWriter&);

	// 2007/09/09 
	// std::ofstream ���� Microsoft Visual C++ 2005 �̃��C�u������
	// �}���`�o�C�g�����Z�b�g���g�p����ꍇ
	// ���{��̃p�X��n���ƃt�@�C�����I�[�v���ł��Ȃ�
	// _tsetlocale( LC_ALL, _T("japanese") ); �ŉ������邪
	// ����� cout �ւ̓��{��̏o�͂��ł��Ȃ��Ȃ�̂� CreateFile ���g�p����
	HANDLE hFile_;
};

} // end of namespace KSDK

#endif //__BINARYFILE_H__