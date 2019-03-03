#ifndef __ARCHIVEDLL_H__
#define __ARCHIVEDLL_H__

#include <vector>
#include <boost/shared_ptr.hpp>
#include "comm-arc.h"
#include "String.h"

namespace KSDK {

namespace ArchiveDllID {
enum ArchiveDllID {
	SEVEN_ZIP,
	UNLHA,
	UNRAR,
	UNZIP,
	MAX_ARCHIVE_DLL,
};
}

class ArchiveDll {
public:
	ArchiveDll();
	~ArchiveDll();

	bool init(ArchiveDllID::ArchiveDllID archiveDllID);

	//bool setDllFilename(LPCTSTR filename, LPCTSTR prefix);

	//LPCTSTR getDllFilename() {
	//	return mDllFilename.c_str();
	//}

	ArchiveDllID::ArchiveDllID getID() {
		return archiveDllID_;
	}

	WORD getVersion();

	bool openArchive(const HWND hwnd, const DWORD mode);
	bool closeArchive();

	LPCTSTR getArchiveFilename();

	void setArchiveFilename(LPCTSTR filename) {
		archiveFilename_ = filename;
	}

	// �ꊇ��
	int extract(LPCTSTR destPath, bool showsProgress, bool overwritesFile);

	// �w��̃t�@�C���E�t�H���_���w��̃t�H���_�ɉ�
	int extract(LPCTSTR srcPath, LPCTSTR destPath, bool showsProgress, LPCTSTR password = NULL);

	// �w��t�H���_�ȉ����w��t�@�C�����ň��k
	bool compress(LPCTSTR srcPath, LPCTSTR destPath, int compressLevel, bool showsProgress);

	int findFirst(LPCTSTR wildName, INDIVIDUALINFO* p);
	int findNext(INDIVIDUALINFO* p);

	// ������  1
	int command(const HWND hwnd, LPCTSTR cmdLine, String& rOutput);

	// ������  10
	bool getRunning();

	// ������  12
	bool checkArchive();

	// ������  47
	int getAttribute();

	// ������  40
	int getFileName(String& rFilename);

	// ������  65
	bool getWriteTimeEx(FILETIME& rLastWriteTime);

private:
	ArchiveDll(const ArchiveDll& archiveDll);
	ArchiveDll& operator=(const ArchiveDll& archiveDll);

	FARPROC getFuncAddress(LPCTSTR funcName);

	String mDllFilename;
	String mPrefix;

	String archiveFilename_;

	HINSTANCE mDllHandle;
	HARC mArchiveHandle;

	ArchiveDllID::ArchiveDllID archiveDllID_;
};


class ArchiveDllManager {
public:
	ArchiveDll* addArchiveDll(ArchiveDllID::ArchiveDllID archiveDllID);
	ArchiveDll* getArchiveDll(ArchiveDllID::ArchiveDllID archiveDllID);

	void releaseArchiveDll(ArchiveDllID::ArchiveDllID archiveDllID);

	// �w��̏��ɂ������̂ɓK���ȃA�[�J�C�uDLL��Ԃ�
	ArchiveDll* getSuitableArchiveDll(LPCTSTR filename);

private:
	std::vector<boost::shared_ptr<ArchiveDll> > archiveDlls_;
};

} // end of namespace KSDK

#endif //__ARCHIVEDLL_H__