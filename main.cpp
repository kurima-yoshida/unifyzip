#include <tchar.h>

#include <iostream>
#include <list>
//#include <algorithm>

#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>

#pragma comment(lib, "winmm.lib")

#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

#include "ArchiveDll.h"
#include "TemporaryFolder.h"

#include "StrFunc.h"
#include "PathFunc.h"
#include "Error.h"
#include "Profile.h"
#include "CommandLine.h"
#include "String.h"

#include "Zip.h"

#include "resource.h"

using namespace std;
using namespace boost;
using namespace KSDK;

// �A�[�J�C�u�Ɋi�[�����t�@�C���E�t�H���_�̏��
class IndividualInfo {
public:

	IndividualInfo(INDIVIDUALINFO& rIndividualInfo, DWORD attribute, bool isEncrypted) {
		// INDIVIDUALINFO ���̑����̓t�H���_���������m�łȂ������肷��̂�
		// �ʂɎ擾�������̂Ŏw�肷��

		fullPath_ = rIndividualInfo.szFileName;

		// UNLHA �ł̓p�X�̃Z�p���[�^�� '/' �Ȃ̂� '\\' �ɕϊ�
		fullPath_.Replace(_TCHAR('/'), _TCHAR('\\'));

		// ������'\\'����菜��
		fullPath_.TrimRight(TCHAR('\\'));

		GetFileName(fullPath_.c_str(), filename_);

		DosDateTimeToFileTime(rIndividualInfo.wDate, rIndividualInfo.wTime, &lastWriteTime_);

		attribute_ = attribute;

		isEncrypted_ = isEncrypted;

		// wRatio �� szMode ���爳�k���x���𔻒f����
		if (lstrcmp(rIndividualInfo.szMode, _T("Store")) == 0) {
			compressLevel_ = 0;
		} else {
			compressLevel_ = -1;
		}
	}

	IndividualInfo(LPCTSTR fullPath, DWORD attribute, bool isEncrypted) {
		fullPath_ = fullPath;

		// UNLHA �ł̓p�X�̃Z�p���[�^�� '/' �Ȃ̂� '\\' �ɕϊ�
		fullPath_.Replace(_TCHAR('/'), _TCHAR('\\'));

		// ������'\\'����菜��
		fullPath_.TrimRight(TCHAR('\\'));

		GetFileName(fullPath_.c_str(), filename_);

		// �X�V�����͌��ݎ��������Ă���
		SYSTEMTIME st;
		GetSystemTime(&st);
		FILETIME ft;
		SystemTimeToFileTime(&st, &ft);
		LocalFileTimeToFileTime(&ft, &lastWriteTime_);

		attribute_ = attribute;

		isEncrypted_ = isEncrypted;

		compressLevel_ = -1;
	}

	// fileName �� ���ɓ��̃t���p�X�i�p�X�̃Z�p���[�^�� '\\'�j
	IndividualInfo(LPCTSTR fileName, FILETIME fileTime, DWORD attribute, int compressLevel, bool isEncrypted) {
		fullPath_ = fileName;
		GetFileName(fullPath_.c_str(), filename_);
		lastWriteTime_ = fileTime;
		attribute_ = attribute;
		compressLevel_ = compressLevel;
		isEncrypted_ = isEncrypted;
	}

	LPCTSTR getFullPath() const {
		return fullPath_.c_str();
	}
	LPCTSTR getFilename() const {
		return filename_.c_str();
	}
	DWORD getAttribute() {
		return attribute_;
	}
	const FILETIME* getLastWriteTime() const {
		return &lastWriteTime_;
	}
	int getCompressLevel() {
		return compressLevel_;
	}

	bool isEncrypted() {
		return isEncrypted_;
	}

private:
	String filename_;
	String fullPath_;
	FILETIME lastWriteTime_;

	// �t�@�C��������A�t�@�C�����t�H���_��
	DWORD attribute_;

	// ���k���x��
	// -1 ���ƕs��
	int compressLevel_;

	// �Í�������Ă��邩
	bool isEncrypted_;
};


// temp �t�H���_�𓾂�
bool getTemporaryPath(String& dest);

// ������
bool initialize();

// ���ɂ���������
bool process(LPCTSTR filename);

void analyzePath(vector<String>& src, vector<String>& dest);
bool analyzePathSub(LPCTSTR src, vector<String>& dest);

// �w��̃t�H���_���i�T�u�t�H���_�܂ށj�̃t�H���_�̍X�V������
// �t�H���_���̍ŐV�̍X�V���������t�@�C���E�t�H���_�ƈ�v������
void setFolderTime(LPCTSTR path);
bool setFolderTimeSub(LPCTSTR path, FILETIME& rNewestFileTime);

// �璷�t�H���_�̃p�X�𓾂�
void GetRedundantPath(String& rRedundantPath);

bool AddLackFolder();

bool GetIndividualInfo(LPCTSTR path);
bool GetIndividualInfoSub(LPCTSTR searchPath, LPCTSTR basePath);

// �t�H���_�������k
bool compress(LPCTSTR srcPath, LPCTSTR dstPath, int compressLevel, bool showsProgress);
bool compressSub(LPCTSTR pszDir, ZipWriter& zw, int basePathLength);

int CheckRemarkableFile();
int CheckRemarkableFileSub();

list<boost::shared_ptr<IndividualInfo> > g_compressFileList;

ArchiveDllManager g_ArchiveDllManager;

// �e��I�v�V����
int g_PriorityClass;

bool g_DeletesEmptyFolder;
bool UsesRecycleBin;
int g_RemovesRedundantFolder;
bool g_ShowsProgress;
String g_AppendFile;
String g_ExcludeFile;
String g_RemarkableFile;
int g_CompressLevel;	// ���k���x�� = [ -1 | 1 | 0 | 9 ]

int g_FileTimeMode;
int g_FolderTimeMode;
int g_FileAttribute;
int g_FolderAttribute;

bool g_BatchDecompress;
bool g_SearchSubfolders;

bool g_RecyclesIndividualFiles;

String g_FileTime;
String g_FolderTime;

int g_WarningLevel;

bool g_ExitsImmediately;

bool g_PreCheck;

bool g_IgnoresEncryptedFiles;

std::vector<UINT8> g_ArchiveComment;

namespace FileAttribute {
enum FileAttribute {
	READ_ONLY = 1,
	HIDDEN = 2,
	SYSTEM = 4,
	ARCHIVE = 32,
};
}



// temp �t�H���_�𓾂�
bool getTemporaryPath(String& dest) {
	const DWORD LEN = 512;
	CHAR path[LEN+1];
	dest.Empty();
	DWORD ret = ::GetTempPath(LEN, path);

	// �Z�k�`�Ȃ̂�
	CHAR longPath[LEN+1];
	GetLongPathName(path, longPath, LEN);

	dest.Copy(longPath);
	return (ret == 0);
}


bool initialize() 
{
	// ���P�[���̐ݒ�
	// ���C�u�����̃o�O������̂� 
	// Visual C++ 2005 Express Edition �ł͂Ƃ肠�����g���Ȃ�
	//_tsetlocale( LC_ALL, _T("japanese") );

	SetCurrentDirectoryEx(_T(""));

	// ini�t�@�C�����ݒ��ǂݍ���
	Profile profile;
	String Path;
	GetModuleFileName(NULL, Path);
	RemoveExtension(Path);
	Path.Cat(_T(".ini"));
	profile.Open(Path.c_str());

	g_PriorityClass = profile.GetInt(_T("Setting"), _T("PriorityClass"), 0);

	g_SearchSubfolders = (profile.GetInt(_T("Setting"), _T("SearchSubfolders"), 0) != 0);

	g_DeletesEmptyFolder = (profile.GetInt(_T("Setting"), _T("DeletesEmptyFolder"), 0) != 0);
	UsesRecycleBin = (profile.GetInt(_T("Setting"), _T("UsesRecycleBin"), 1) != 0);
	g_RemovesRedundantFolder = profile.GetInt(_T("Setting"), _T("RemovesRedundantFolder"), 0);
	g_ShowsProgress = (profile.GetInt(_T("Setting"), _T("ShowsProgress"), 0) != 0);
	g_CompressLevel = profile.GetInt(_T("Setting"), _T("CompressLevel"), 0);
	g_BatchDecompress = (profile.GetInt(_T("Setting"), _T("BatchDecompress"), 0) != 0);

	g_FileTimeMode = profile.GetInt(_T("Setting"), _T("FileTimeMode"), 0);
	g_FolderTimeMode = profile.GetInt(_T("Setting"), _T("FolderTimeMode"), 0);
	g_FileAttribute = profile.GetInt(_T("Setting"), _T("FileAttribute"), -1);
	g_FolderAttribute = profile.GetInt(_T("Setting"), _T("FolderAttribute"), -1);

	g_WarningLevel = profile.GetInt(_T("Setting"), _T("WarningLevel"), 0);

	g_RecyclesIndividualFiles = (profile.GetInt(_T("Setting"), _T("RecyclesIndividualFiles"), 0) != 0);

	profile.GetString(_T("Setting"), _T("AppendFile"), _T(""), g_AppendFile);
	profile.GetString(_T("Setting"), _T("ExcludeFile"), _T(""), g_ExcludeFile);
	profile.GetString(_T("Setting"), _T("RemarkableFile"), _T(""), g_RemarkableFile);

	profile.GetString(_T("Setting"), _T("FileTime"), _T(""), g_FileTime);
	profile.GetString(_T("Setting"), _T("FolderTime"), _T(""), g_FolderTime);

	g_ExitsImmediately = (profile.GetInt(_T("Setting"), _T("ExitsImmediately"), 0) != 0);

	g_PreCheck = (profile.GetInt(_T("Setting"), _T("PreCheck"), 1) != 0);

	g_IgnoresEncryptedFiles = (profile.GetInt(_T("Setting"), _T("IgnoresEncryptedFiles"), 1) != 0);

	if (g_CompressLevel != -1 && 
		g_CompressLevel != 0 && 
		g_CompressLevel != 1 && 
		g_CompressLevel != 9) 
	{
		cout << "���k���x���̎w�肪�s��" << endl;
		return false;
	}

	if (g_WarningLevel != 0 && 
		g_WarningLevel != 1 && 
		g_WarningLevel != 2) 
	{
		cout << "�x�����x���̎w�肪�s��" << endl;
		return false;
	}

	// �v���Z�X�̗D�揇�ʃN���X��ݒ�
	HANDLE hProcess = GetCurrentProcess();
	DWORD priorityClass = NORMAL_PRIORITY_CLASS;
	switch (g_PriorityClass) {
	case 0:
		priorityClass = NORMAL_PRIORITY_CLASS;
		break;
	case 1:
		priorityClass = IDLE_PRIORITY_CLASS;
		break;
	case 2:
		priorityClass = HIGH_PRIORITY_CLASS;
		break;
	case 3:
		priorityClass = REALTIME_PRIORITY_CLASS;
		break;
	case 4:
		priorityClass = BELOW_NORMAL_PRIORITY_CLASS;
		break;
	case 5:
		priorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
		break;
	default:
		cout << "�v���Z�X�̗D��x�̎w�肪�s��" << endl;
		break;
	}

	if (priorityClass != NORMAL_PRIORITY_CLASS) {
		if (SetPriorityClass(hProcess, priorityClass) == 0) {
			cout << "�v���Z�X�̗D�揇�ʃN���X�̐ݒ�Ɏ��s" << endl;
		}
	}

	return true;
}



void analyzePath(vector<String>& src, vector<String>& dest) {
	vector<String>::iterator it;
	for (it=src.begin(); it!=src.end(); ++it) {

		switch(GetPathAttribute(it->c_str())) {
		case PATH_INVALID:
			// �����ȃp�X
			break;
		case PATH_FILE:
			dest.push_back(*it);
			break;
		case PATH_FOLDER:
			// �t�H���_���̃t�@�C����ǉ�
			analyzePathSub(it->c_str(), dest);
			break;
		}
	}
}

bool analyzePathSub(LPCTSTR src, vector<String>& dest) {
	WIN32_FIND_DATA fd;
	HANDLE hSearch;

	// �T���p�X���쐬
	String path(src);
	FormatTooLongPath(path);
	CatPath(path, _T("*.*"));

	hSearch = FindFirstFile(path.c_str(), &fd);

	if (hSearch==INVALID_HANDLE_VALUE) return false;

	bool succeeds = true;
	do{
		if ( (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0 ){
			if (g_SearchSubfolders) {
				// ���[�g�y�уJ�����g�łȂ��t�H���_���ǂ����`�F�b�N
				if (lstrcmp(fd.cFileName,_T("."))!=0 && lstrcmp(fd.cFileName,_T("..")) != 0) {
					String searchPath(src);
					CatPath(searchPath, fd.cFileName);
					if (!analyzePathSub(searchPath.c_str(), dest)) {
						succeeds = false;
						break;
					}
				}
			}
		}else{
			String str(src);
			CatPath(str, fd.cFileName);
			dest.push_back(str);
		}
	} while (FindNextFile(hSearch,&fd)!=0);

	FindClose(hSearch);

	return succeeds;
}



struct FileSortCriterion : public binary_function <String, String, bool> {
	bool operator()(const String& _Left, const String& _Right) const 
	{
		return ComparePath(_Left.c_str(), _Right.c_str(), false, false);
	}
};

#include <fstream>
int main(int, char*, char*) 
{
	if (!initialize()) {
		return 0;
	}

	vector<String> arguments;
	vector<String> files;
	String str;

	// �R�}���h���C���Ƀt�@�C���E�t�H���_�̎w��͂Ȃ���
	void* pv;
	pv = AllocCommandLine();
	int u = GetStringFromCommandLine(pv, 0xFFFFFFFF, str);
	if(u >= 2){
		for (int i=1; i<u; ++i) {
			GetStringFromCommandLine(pv, i, str);
			arguments.push_back(str);
		}
	}
	FreeCommandLine(pv);

	analyzePath(arguments, files);

	#ifdef _DEBUG
	// �f�o�b�O�p�ɒ��ڎw��
	files.clear();
	str.Copy("C:\\test.zip");
	files.push_back(str);
	#endif

	#ifdef _DEBUG
	{
		vector<String>::iterator it;
		int i;
		for (i = 0, it = files.begin(); it != files.end(); ++i, ++it) {
			cout << i << " " << it->c_str() << endl;
		}
	}
	#endif

	// ������ files �ɂ̓t�@�C���݂̂Ńt�H���_�͊܂܂�Ă��Ȃ��͂�
	// �G�N�X�v���[���Ɠ����Ƀ\�[�g
	sort(files.begin(), files.end(), FileSortCriterion());

	vector<String>::iterator it;
	for (it=files.begin(); it!=files.end(); ++it) {
		cout << it->c_str() << endl;
		process(it->c_str());
		cout << endl;
	}

	if (!g_ExitsImmediately) {
		cout << "�G���^�[�L�[�������Ă������� �I�����܂�" << endl;
		getchar();
	}

	return 0;
}




// �G�N�X�v���[���Ɠ���
// �t�H���_�Ƀt�H���_���̃t�@�C�����������Ƃ��ۏ؂����
struct IndividualInfoSortCriterion : public binary_function <boost::shared_ptr<IndividualInfo>, boost::shared_ptr<IndividualInfo>, bool> {
	bool operator()(const boost::shared_ptr<IndividualInfo>& _Left, const boost::shared_ptr<IndividualInfo>& _Right) const
	{
		return ComparePath(_Left->getFullPath(), _Right->getFullPath(), 
			(_Left->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0, 
			(_Right->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0);
	}
};

// ���ɓ��̃t�@�C���̏��𓾂�
// �t�@�C���I�[�v���ς݂�ArchiveDll��n��
bool GetIndividualInfo(ArchiveDll& rArchiveDll) 
{
	g_compressFileList.clear();

	// �A�[�J�C�u���̑S�t�@�C���E�t�H���_���擾
	INDIVIDUALINFO ii;
	int ret = rArchiveDll.findFirst("*", &ii);

	for (;;) {
		if (ret == 0) {
			// INDIVIDUALINFO ���̑����̓t�H���_���������m�łȂ������肷��̂ł������Ŏ擾
			int attribute = rArchiveDll.getAttribute();
			DWORD dwAttribute = 0;
			if ((attribute & FA_RDONLY) != 0) dwAttribute |= FILE_ATTRIBUTE_READONLY;
			if ((attribute & FA_HIDDEN) != 0) dwAttribute |= FILE_ATTRIBUTE_HIDDEN;
			if ((attribute & FA_SYSTEM) != 0) dwAttribute |= FILE_ATTRIBUTE_SYSTEM;
			if ((attribute & FA_DIREC) != 0) dwAttribute |= FILE_ATTRIBUTE_DIRECTORY;
			if ((attribute & FA_ARCH) != 0) dwAttribute |= FILE_ATTRIBUTE_ARCHIVE;

			// �p�X���[�h����
			bool isEncrypted = ((attribute & FA_ENCRYPTED) != 0);

			// �{�����[�����x���Ƃ̂��Ƃ����s��
			if ((attribute & FA_LABEL) != 0) dwAttribute |= 0;

			if (dwAttribute == 0) FILE_ATTRIBUTE_NORMAL;

			g_compressFileList.push_back(boost::shared_ptr<IndividualInfo>(new IndividualInfo(ii, dwAttribute, isEncrypted)));
		} else if(ret == -1) {
			// �����I��
			break;
		} else {
			// �G���[
			cout << "���ɓ��������ɃG���[���N���܂���" << endl;
			return false;
		}
		ret = rArchiveDll.findNext(&ii);
	}

	// �\�[�g
	g_compressFileList.sort(IndividualInfoSortCriterion());

	#ifdef _DEBUG
	{
		cout << "�t�H���_�ǉ��O" << endl;
		list<boost::shared_ptr<IndividualInfo> >::iterator it;	
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
			cout << (*it)->getFullPath() << endl;
		}
	}
	#endif

	AddLackFolder();

	#ifdef _DEBUG
	{
		cout << "�t�H���_�ǉ���" << endl;
		list<boost::shared_ptr<IndividualInfo> >::iterator it;	
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
			cout << (*it)->getFullPath() << endl;
		}
	}
	#endif

	return true;
}


// �w��̃t�H���_���̃t�@�C���E�t�H���_�̏��擾
bool GetIndividualInfo(LPCTSTR path) 
{
	g_compressFileList.clear();

	if (!GetIndividualInfoSub(path, path)) {
		cout << "�t�@�C�����擾���ɃG���[���N���܂���" << endl;
		return false;
	}

	// �\�[�g
	g_compressFileList.sort(IndividualInfoSortCriterion());

	#ifdef _DEBUG
	{
		list<boost::shared_ptr<IndividualInfo> >::iterator it;	
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
			cout << (*it)->getFullPath() << endl;
		}
	}
	#endif

	return true;
}


bool GetIndividualInfoSub(LPCTSTR searchPath, LPCTSTR basePath)
{
	WIN32_FIND_DATA fd;
	String path;
	HANDLE hSearch;

	// �T���p�X���쐬
	path = searchPath;
	CatPath(path, _T("*.*"));

	hSearch = FindFirstFile(path.c_str(), &fd);

	if (hSearch==INVALID_HANDLE_VALUE) {
		return false;
	}

	// �h���C�u�݂̂̏ꍇ��\�ŏI��肻��ȊO��\�ŏI���Ȃ��̂ł��̒���
	int basePathLen = lstrlen(basePath);
	if (basePathLen <= lstrlen(_T("C:\\"))) {
	} else {
		basePathLen++;
	}

	do{
		// �f�B���N�g�����A���ʂ̃t�@�C����
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0){
			// �f�B���N�g���ł�����

			// ���[�g�y�уJ�����g�łȂ��t�H���_���ǂ����`�F�b�N
			if (lstrcmp(fd.cFileName,_T(".")) == 0 || lstrcmp(fd.cFileName, _T("..")) == 0) {
				 continue;
			}

			path = searchPath;
			CatPath(path, fd.cFileName);

			String destPath = path.c_str();
			destPath.MakeMid(basePathLen);

			g_compressFileList.push_back(boost::shared_ptr<IndividualInfo>(new IndividualInfo(destPath.c_str(), fd.ftLastWriteTime, fd.dwFileAttributes, 0, false)));

			if (!GetIndividualInfoSub(path.c_str(), basePath)) {
				FindClose(hSearch);
				return false;
			}
		} else {
			// ���ʂ̃t�@�C���ł�����
			path = searchPath;
			CatPath(path, fd.cFileName);

			String destPath = path;
			destPath.MakeMid(basePathLen);

			g_compressFileList.push_back(boost::shared_ptr<IndividualInfo>(new IndividualInfo(destPath.c_str(), fd.ftLastWriteTime, fd.dwFileAttributes, 0, false)));
		}

	} while (FindNextFile(hSearch,&fd)!=0);

	FindClose(hSearch);

	return (GetLastError() == ERROR_NO_MORE_FILES);
}




// ���X�g�����t�H���_����菜��
bool deleteEmptyFolder(LPCTSTR basePath) {

	if (g_compressFileList.empty()) {
		return true;
	}

	list<boost::shared_ptr<IndividualInfo> >::iterator cur;
	list<boost::shared_ptr<IndividualInfo> >::iterator next;

	next = g_compressFileList.end();
	cur = next;
	cur--;
	for (;;) {
		// ���̗v�f���f�B���N�g�����̂��̂������Ă��Ȃ���΋�t�H���_
		if (((*cur)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			if (next == g_compressFileList.end()
			 || (_tcsncmp((*cur)->getFullPath(), (*next)->getFullPath(), lstrlen((*cur)->getFullPath())) != 0)
			 || (*((*next)->getFullPath() + lstrlen((*cur)->getFullPath())) != _TCHAR('\\')) ) {

				if (g_BatchDecompress) {
					String path = basePath;
					CatPath(path, (*cur)->getFullPath());
					if(!RemoveDirectory(path.c_str())) {
						cout << "��t�H���_ " << (*cur)->getFullPath() << " �̍폜�Ɏ��s"<< endl;
						return false;
					}
				}

				cout << "��t�H���_ " << (*cur)->getFullPath() << " ���폜"<< endl;
				g_compressFileList.erase(cur);
				if (g_compressFileList.empty()) break;
				cur = next;
			} else {
				next = cur;
			}
		} else {
			next = cur;
		}
		if (cur == g_compressFileList.begin()) break;
		cur--;
	}
	return true;
}


// ���X�g�ɋ�t�H���_�������false��Ԃ�
bool ExistsEmptyFolder() {

	if (g_compressFileList.empty()) {
		return false;
	}

	list<boost::shared_ptr<IndividualInfo> >::iterator cur;
	list<boost::shared_ptr<IndividualInfo> >::iterator next;

	next = g_compressFileList.end();
	cur = next;
	cur--;
	for (;;) {
		// ���̗v�f���f�B���N�g�����̂��̂������Ă��Ȃ���΋�t�H���_
		if (((*cur)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			if (next == g_compressFileList.end()) {
				return true;
			} else if ((_tcsncmp((*cur)->getFullPath(), (*next)->getFullPath(), lstrlen((*cur)->getFullPath())) != 0) ||
				(*((*next)->getFullPath() + lstrlen((*cur)->getFullPath())) != _TCHAR('\\')) ) {
				return true;
			} else {
				next = cur;
			}
		} else {
			next = cur;
		}
		if (cur == g_compressFileList.begin()) break;
		cur--;
	}
	return false;
}

bool ExistsRedundantFolde() {
	String path;
	GetRedundantPath(path);
	return (path.IsEmpty() == false);
}

// �璷�t�H���_�̃p�X�𓾂�
void GetRedundantPath(String& rRedundantPath) {
	rRedundantPath.Empty();

	bool isEmpty = true;
	list<boost::shared_ptr<IndividualInfo> >::iterator it;
	for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
		if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			// �t�@�C��

			// �t�@�C�����������炻��ȏ�̏璷�p�X�̐L�т͖���
			TCHAR path[MAX_PATH + 1];
			PathCommonPrefix((*it)->getFullPath(), rRedundantPath.c_str(), path);

			if (lstrlen(path) < rRedundantPath.GetLength()) {
				rRedundantPath = path;
				if (*path == TCHAR('\0')) break;
			}
			// ���̂܂�
			isEmpty = false;

		} else {
			// �t�H���_
			TCHAR path[MAX_PATH + 1];
			PathCommonPrefix((*it)->getFullPath(), rRedundantPath.c_str(), path);

			if (lstrlen(path) < rRedundantPath.GetLength()) {
				rRedundantPath = path;
				if (*path == TCHAR('\0')) break;
				isEmpty = false;
			} else {
				if (isEmpty) {
					rRedundantPath = (*it)->getFullPath();
				} else {
					// ���̂܂�
				}
			}
		}
	}
	FixPath(rRedundantPath);
}

// wstring ��MBCS�������UNICODE�ɕϊ����ăZ�b�g����
void setMultiByteString(wstring& rWString, const char* string) {

	rWString.erase();

	// Unicode�ɕK�v�ȕ������̎擾
	int len = ::MultiByteToWideChar(CP_THREAD_ACP, 0, string, -1, NULL, 0);
	WCHAR* pUnicode = new WCHAR[len];

	//�ϊ�
	::MultiByteToWideChar(CP_THREAD_ACP, 0, string, (int)strlen(string)+1, pUnicode, len);

	rWString = pUnicode;

	delete	pUnicode;
}

// -1 : �G���[
//  0 : �Ȃɂ��Ȃ�
//  1 : ���f���ׂ�
int CheckRemarkableFile() 
{
	int r = CheckRemarkableFileSub();
	if (r == 0) {
		return 0;
	} else if (r == 1) {
		switch (g_WarningLevel) {
		case 1:
		{
			// ���b�Z�[�W�ƍ\���o��
			cout << "���ڃt�@�C���݂̂ō\������Ă��܂�" << endl;
			list<boost::shared_ptr<IndividualInfo> >::iterator it;
			for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
				cout << (*it)->getFullPath() << endl;
			}
			break;
		}
		case 2:
		{
			// ���b�Z�[�W�ƍ\���o��
			cout << "���ڃt�@�C�����܂܂�Ă��܂�" << endl;
			list<boost::shared_ptr<IndividualInfo> >::iterator it;
			for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
				cout << (*it)->getFullPath() << endl;
			}
			break;
		}
		default:
			break;
		}
		return 1;
	} else {
		return -1;
	}
}

//  0 : ����I��
//  1 : �����ɂ�������
// -1 : �G���[
int CheckRemarkableFileSub() 
{
	if (g_WarningLevel == 0) return 0;
	if (g_RemarkableFile.IsEmpty()) return 0;

	#ifdef _UNICODE
		wstring append(g_RemarkableFile.c_str());
	#else
		wstring append;
		setMultiByteString(append, g_RemarkableFile.c_str());
	#endif

	try {
		wregex regex(append, regex_constants::normal | regex_constants::icase);

		// ���K�\���ƃt�@�C�������}�b�`���Ȃ��������菜��
		list<boost::shared_ptr<IndividualInfo> >::iterator it;
		for (it=g_compressFileList.begin(); it!=g_compressFileList.end(); ++it) {
			// �t�H���_�͖���
			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				continue;
			}

			#ifdef _UNICODE
				wstring filename((*it)->getFullPath()));
			#else
				wstring filename;
				setMultiByteString(filename, (*it)->getFullPath());
			#endif

			wsmatch results;
			if (regex_match(filename, results, regex)) {
				if (g_WarningLevel == 2) return 1;
			} else {
				if (g_WarningLevel == 1) return 0;
			}
		}
		if (g_WarningLevel == 1) return 1;
	}

	catch (boost::bad_expression) {
		cout << "���K�\���̃R���p�C���Ɏ��s" << endl;
		return -1;
	}

	return 0;
}

// �ǉ��t�@�C�����`�F�b�N
bool CheckAppendFile(LPCTSTR basePath) {
	if (g_AppendFile.IsEmpty()) return true;

	#ifdef _UNICODE
		wstring append(g_AppendFile.c_str());
	#else
		wstring append;
		setMultiByteString(append, g_AppendFile.c_str());
	#endif

	try {
		wregex regex(append, regex_constants::normal | regex_constants::icase);

		// ���K�\���ƃt�@�C�������}�b�`���Ȃ��������菜��
		list<boost::shared_ptr<IndividualInfo> >::iterator it = g_compressFileList.begin();
		while (it != g_compressFileList.end()) {
			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				++it;
				continue;
			}

			#ifdef _UNICODE
				wstring filename((*it)->getFullPath()));
			#else
				wstring filename;
				setMultiByteString(filename, (*it)->getFullPath());
			#endif

			wsmatch results;
			if (regex_match(filename, results, regex)) {
				++it;
			} else {
				// �ꊇ�𓀂̏ꍇ�͉𓀍ς݂̎��ۂ̃t�@�C�����폜
				if (g_BatchDecompress) {
					String path = basePath;
					CatPath(path, (*it)->getFullPath());

					// �ǂݎ���p�t�@�C����������悤��
					// DeleteFile() �ł͂Ȃ� DeleteFileOrFolder() ���g�p
					bool usesRecycleBin = g_RecyclesIndividualFiles;
					if(!DeleteFileOrFolder(path.c_str(), usesRecycleBin)) {
						cout << "�t�@�C�� " << (*it)->getFullPath() << " �̍폜�Ɏ��s"<< endl;
						return false;
					}
				}
				cout << "�t�@�C�� " << (*it)->getFullPath() << " ���폜"<< endl;
				it = g_compressFileList.erase(it);
			}
		}
	}

	catch (boost::bad_expression) {
		cout << "���K�\���̃R���p�C���Ɏ��s" << endl;
		return false;
	}

	return true;
}

// �ǉ��t�@�C�����`�F�b�N
bool ExistsNotAppendFile() {

	#ifdef _UNICODE
		wstring append(g_AppendFile.c_str());
	#else
		wstring append;
		setMultiByteString(append, g_AppendFile.c_str());
	#endif

	try {
		wregex regex(append, regex_constants::normal | regex_constants::icase);

		list<boost::shared_ptr<IndividualInfo> >::iterator it = g_compressFileList.begin();
		while (it != g_compressFileList.end()) {
			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				++it;
				continue;
			}

			#ifdef _UNICODE
				wstring filename((*it)->getFullPath()));
			#else
				wstring filename;
				setMultiByteString(filename, (*it)->getFullPath());
			#endif

			wsmatch results;
			if (regex_match(filename, results, regex)) {
				++it;
			} else {
				return true;
			}
		}
	}

	// �G���[�͂����ł͏o����
	// �`�F�b�N�𒆒f������`�ŕԂ�
	catch (boost::bad_expression) {
		return true;
	}

	return false;
}

// ���O�t�@�C�����`�F�b�N
bool CheckExcludeFile(LPCTSTR basePath) {

	if (g_ExcludeFile.IsEmpty()) return true;

	#ifdef _UNICODE
		wstring exclude(g_ExcludeFile.c_str());
	#else
		wstring exclude;
		setMultiByteString(exclude, g_ExcludeFile.c_str());
	#endif

	try {
		wregex regex(exclude, regex_constants::normal | regex_constants::icase);

		// ���K�\���ƃt�@�C�������}�b�`�������菜��
		list<boost::shared_ptr<IndividualInfo> >::iterator it = g_compressFileList.begin();
		while (it != g_compressFileList.end()) {
			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				++it;
				continue;
			}

			#ifdef _UNICODE
				wstring filename((*it)->getFullPath()));
			#else
				wstring filename;
				setMultiByteString(filename, (*it)->getFullPath());
			#endif

			wsmatch results;
			if ( regex_match(filename, results, regex) ) {
				// �ꊇ�𓀂̏ꍇ�͉𓀍ς݂̎��ۂ̃t�@�C�����폜
				if (g_BatchDecompress) {
					String path = basePath;
					CatPath(path, (*it)->getFullPath());

					// �ǂݎ���p�t�@�C����������悤��
					// DeleteFile() �ł͂Ȃ� DeleteFileOrFolder() ���g�p
					bool usesRecycleBin = g_RecyclesIndividualFiles;
					if(!DeleteFileOrFolder(path.c_str(), usesRecycleBin)) {
						cout << "�t�@�C�� " << (*it)->getFullPath() << " �̍폜�Ɏ��s"<< endl;
						return false;
					}
				}
				cout << "�t�@�C�� " << (*it)->getFullPath() << " ���폜"<< endl;
				it = g_compressFileList.erase(it);
			} else {
				++it;
			}
		}
	}

	catch (boost::bad_expression) {
		cout << "���K�\���̃R���p�C���Ɏ��s" << endl;
		return false;
	}

	return true;
}


// ���O�t�@�C�����`�F�b�N
bool ExistsExcludeFile() {
	#ifdef _UNICODE
		wstring exclude(g_ExcludeFile.c_str());
	#else
		wstring exclude;
		setMultiByteString(exclude, g_ExcludeFile.c_str());
	#endif

	try {
		wregex regex(exclude, regex_constants::normal | regex_constants::icase);

		// ���K�\���ƃt�@�C�������}�b�`�������菜��
		list<boost::shared_ptr<IndividualInfo> >::iterator it = g_compressFileList.begin();
		while (it != g_compressFileList.end()) {
			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				++it;
				continue;
			}

			#ifdef _UNICODE
				wstring filename((*it)->getFullPath()));
			#else
				wstring filename;
				setMultiByteString(filename, (*it)->getFullPath());
			#endif

			wsmatch results;
			if ( regex_match(filename, results, regex) ) {
				return true;
			} else {
				++it;
			}
		}
	}

	// �G���[�͂����ł͏o����
	// �`�F�b�N�𒆒f������`�ŕԂ�
	catch (boost::bad_expression) {
		return true;
	}

	return false;
}


void GetToken(String& rDest, String& rSrc, LPCTSTR token) {
	rDest.Empty();
	int pos = rSrc.FindOneOf(token);
	if (pos == -1) return;
	rDest.NCopy(rSrc.c_str(), pos);
	rSrc.MakeMid(pos+1);
}

void StringToFileTime(LPCTSTR fileTime, FILETIME& rFileTime) {
	SYSTEMTIME st;

	String src(fileTime);
	String token;
	GetToken(token, src, _T("/"));
	st.wYear = (WORD)_ttoi(token.c_str());
	GetToken(token, src, _T("/"));
	st.wMonth = (WORD)_ttoi(token.c_str());
	GetToken(token, src, _T(" "));
	st.wDay = (WORD)_ttoi(token.c_str());
	GetToken(token, src, _T(":"));
	st.wHour = (WORD)_ttoi(token.c_str());
	GetToken(token, src, _T(":"));
	st.wMinute = (WORD)_ttoi(token.c_str());
	token = src;
	st.wSecond = (WORD)_ttoi(token.c_str());
	st.wDayOfWeek = 0;
	st.wMilliseconds = 0;

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	LocalFileTimeToFileTime(&ft, &rFileTime); //�ϊ�
}

// ������Ńt�@�C���̍X�V������ݒ肷��
bool SetFileLastWriteTime(LPCTSTR filename, LPCTSTR lastWriteTime) {

	FILETIME ft;

	StringToFileTime(lastWriteTime, ft);

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		cout << "�t�@�C���̍X�V�����ύX�Ɏ��s���܂���" << endl;
		return false;
	}

	bool b = (SetFileTime(hFile, NULL, NULL, &ft) != 0);

	if (b == false) {
		cout << "�t�@�C���̍X�V�����ύX�Ɏ��s���܂���" << endl;
	}

	CloseHandle(hFile);

	return b;
}


DWORD MyFileAttributeToFileAttribute(int attribute, bool isFolder) {
	DWORD dw = 0;
	if (isFolder) dw = FILE_ATTRIBUTE_DIRECTORY;
	if ((attribute & FileAttribute::READ_ONLY) != 0)	dw |= FILE_ATTRIBUTE_READONLY;
	if ((attribute & FileAttribute::HIDDEN) != 0)		dw |= FILE_ATTRIBUTE_HIDDEN;
	if ((attribute & FileAttribute::SYSTEM) != 0)		dw |= FILE_ATTRIBUTE_SYSTEM;
	if ((attribute & FileAttribute::ARCHIVE) != 0)		dw |= FILE_ATTRIBUTE_ARCHIVE;
	if (dw == 0) dw = FILE_ATTRIBUTE_NORMAL;
	return dw;
}



bool AddLackFolder() {

	bool adds = false;
	
	String path;
	list<boost::shared_ptr<IndividualInfo> >::iterator it;
	for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {

		if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			// �t�H���_����������p�X���L��
			// �������A�Q�K�w��C�ɒǉ������\��������̂�
			// �P�K�w���`�F�b�N

			TCHAR common[MAX_PATH + 1];
			PathCommonPrefix((*it)->getFullPath(), path.c_str(), common);
			LPCTSTR p;
			if (*common == _TCHAR('\0')) {
				p = (*it)->getFullPath();
			} else {
				p = (*it)->getFullPath() + lstrlen(common) + 1;
			}
			if (strchrex(p, _TCHAR('\\')) == NULL) {
				// �P�K�w�����ω����Ă��Ȃ����
				// ���̑O�Ƀt�H���_��}������K�v�͂Ȃ�
				path = (*it)->getFullPath();
			} else {
				// �t�H���_���o�^����Ă��Ȃ�
				String filePath = (*it)->getFullPath();
				RemoveFileName(filePath);

				adds = true;

				path = common;
				for (;;) {
					LPCTSTR p1 = filePath.c_str() + path.GetLength() + 1;
					LPCTSTR p2 = strchrex(p1, _TCHAR('\\'));
					if (p2 == NULL) {
						path = filePath;
					} else {
						path.NCopy(filePath.c_str(), path.GetLength() + (int)(p2 - p1) + 1);
					}
					it = g_compressFileList.insert(it, boost::shared_ptr<IndividualInfo>(new IndividualInfo(path.c_str(), FILE_ATTRIBUTE_DIRECTORY, false)));
					it++;
					if (p2 == NULL) break;
				}
			}

		} else {
			// �t�@�C������������
			// ���̃t�@�C�������f�B���N�g�������邩�`�F�b�N����
			String filePath = (*it)->getFullPath();
			RemoveFileName(filePath);
			int r = lstrcmpi(filePath.c_str(), path.c_str());
			if (r == 0) {
				// �t�H���_�͓o�^�ς�
			} else if (r < 0) {
				// ��̊K�w�̃t�H���_�ɂ�����
				path = filePath;
			} else {
				// �t�H���_���o�^����Ă��Ȃ�
				adds = true;

				TCHAR common[MAX_PATH + 1];
				PathCommonPrefix((*it)->getFullPath(), path.c_str(), common);
				path = common;

				for (;;) {
					LPCTSTR p1 = filePath.c_str() + path.GetLength() + 1;
					LPCTSTR p2 = strchrex(p1, _TCHAR('\\'));
					if (p2 == NULL) {
						path = filePath;
					} else {
						path.NCopy(filePath.c_str(), path.GetLength() + (int)(p2 - p1) + 1);
					}
					it = g_compressFileList.insert(it, boost::shared_ptr<IndividualInfo>(new IndividualInfo(path.c_str(), FILE_ATTRIBUTE_DIRECTORY, false)));
					it++;
					if (p2 == NULL) break;
				}
			}
		}
	}
	
	return adds;
}




//  0:	���ɂ̏����擾�� �����ς�
//  1:	���ɂ̏����擾�� ������
// -1:	���ɂ̏��擾�ł���
int isProcessedArchive(LPCTSTR filename) 
{
	g_compressFileList.clear();

	ZipReader zip;

	if (!zip.open(filename)) {
		return -1;
	}

	bool isProcessed = true;

	if (g_CompressLevel != 0) {
		isProcessed = false;
	}

	boost::shared_ptr<IndividualInfo> pLastIndividualInfo;

	int localFileNumber = zip.getLocalFileNumber();
	for (int i=0; i<localFileNumber; ++i) {

		boost::shared_ptr<LocalFile> p = zip.getLocalFile(i);
		
		int compressLevel;
		if (p->getCompressionMethod() == 0) {
			compressLevel = 0;
		} else {
			compressLevel = -1;
		}
		bool isEncrypted = p->getGeneralPurposeBitFlag() & 1;

		boost::shared_ptr<IndividualInfo> pIndividualInfo(new IndividualInfo(p->getFileName(), p->getFileTime(), p->getExternalFileAttributes(), compressLevel, isEncrypted));
		g_compressFileList.push_back(pIndividualInfo);

		if (p->getVersionMadeBy() != 0x0014
		 || p->getCompressionMethod() != 0
		 || p->getGeneralPurposeBitFlag() != 0) 
		{
	 		isProcessed = false;
		}

		// internal file attributes �͖����k�łȂ��ꍇ���؂ł��Ȃ�
		if ((p->getExternalFileAttributes() & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			if (p->getVersionNeededToExtract() != 0x000a) {
				isProcessed = false;
			}
			if (p->getInternalFileAttributes() != p->obtainInternalFileAttributes()) {
				isProcessed = false;
			}
		} else {
			if (p->getVersionNeededToExtract() != 0x0014) isProcessed = false;
			if (p->getInternalFileAttributes() != 0) isProcessed = false;
		}

		// �\�[�g���s�v���ǂ����`�F�b�N
		if (pLastIndividualInfo && 
			IndividualInfoSortCriterion()(pIndividualInfo, pLastIndividualInfo)) 
		{
			isProcessed = false;
		}

		/*
		if (pLastIndividualInfo && CompareLocalFileName(pIndividualInfo->getFullPath(), pLastIndividualInfo->getFullPath()) < 0) {
			isProcessed = false;
		}
		*/

		pLastIndividualInfo = pIndividualInfo;
	}

	// ���ɂ̏����擾���I������̂�
	// �����܂łŏ����ς݂łȂ��Ɣ��f���ꂽ�ꍇ������
	if (isProcessed == false) {
		return 1;
	}

	// �\�[�g
	// Zip�Ƃ��Đ������\�[�g����Ă��Ă������p�Ƃ��Đ������Ȃ���������Ȃ��̂ōă\�[�g
	//g_compressFileList.sort(IndividualInfoSortCriterion());

	#ifdef _DEBUG
	{
		list<boost::shared_ptr<IndividualInfo> >::iterator it;	
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
			cout << (*it)->getFullPath() << endl;
		}
	}
	#endif

	// �����Ă���t�H���_�̃`�F�b�N
	// �t�H���_�������Ă���ꍇ�͒ǉ��Ɠ����ɏ����ς݂ł͂Ȃ��Ɣ��f�����
	if (AddLackFolder()) {
		return 1;
	}

	// �ǉ��t�@�C���̃`�F�b�N
	if (g_AppendFile.IsEmpty() == false) {
		if (ExistsNotAppendFile()) return 1;
	}

	// ���O�t�@�C���̃`�F�b�N
	if (g_ExcludeFile.IsEmpty() == false) {
		if (ExistsExcludeFile()) return 1;
	}

	// ��t�H���_�̃`�F�b�N
	if (g_DeletesEmptyFolder) {
		if (ExistsEmptyFolder()) return 1;
	}

	// �璷�t�H���_�̃`�F�b�N
	if (g_RemovesRedundantFolder != 0) {
		if (ExistsRedundantFolde()) return 1;
	}

	{
		// �����̃`�F�b�N
		list<boost::shared_ptr<IndividualInfo> >::iterator it;
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {

			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				if (g_FolderAttribute == -1) continue;
				DWORD attribute = MyFileAttributeToFileAttribute(g_FolderAttribute, true);
				if (attribute != (*it)->getAttribute()) return 1;
			} else {
				if (g_FileAttribute == -1) continue;
				DWORD attribute = MyFileAttributeToFileAttribute(g_FileAttribute, false);
				if (attribute != (*it)->getAttribute()) return 1;
			}
		}
	}

	{
		// �X�V�����̃`�F�b�N
		list<boost::shared_ptr<IndividualInfo> >::iterator it;
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {

			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				if (g_FolderTimeMode == 0) continue;

				if (g_FolderTimeMode == 1) {
					FILETIME ft;
					StringToFileTime(g_FolderTime.c_str(), ft);
					if (CompareFileTime((*it)->getLastWriteTime(),  &ft) != 0) {
						return 1;
					}					
				} else if (g_FolderTimeMode == 2) {

					// �t�H���_�̍X�V���������V�����t�@�C���E�t�H���_��
					// �t�H���_�ȉ��ɂ������璆�~
					// �t�H���_�ȉ��̍ŐV�̃t�@�C���E�t�H���_�ƃt�H���_�̍X�V������
					// ��v���Ȃ��ꍇ�����~

					FILETIME ft;
					ft.dwHighDateTime = 0;
					ft.dwLowDateTime = 0;

					list<boost::shared_ptr<IndividualInfo> >::iterator i = it;
					++i;
					for ( ; i != g_compressFileList.end(); ++i) {
						if (_tcsncmp((*it)->getFullPath(), (*i)->getFullPath(), lstrlen((*it)->getFullPath())) != 0 ||
						*((*i)->getFullPath() + lstrlen((*it)->getFullPath())) != _TCHAR('\\')) {
							break;
						}
						if (CompareFileTime((*it)->getLastWriteTime(),  (*i)->getLastWriteTime()) == -1) {
							return 1;
						}
						if (CompareFileTime(&ft,  (*i)->getLastWriteTime()) == -1) {
							ft = *((*i)->getLastWriteTime());
						}
					}
					if (CompareFileTime(&ft,  (*it)->getLastWriteTime()) != 0) {
						return 1;
					}
				}
			} else {
				if (g_FileTimeMode == 0) continue;

				FILETIME ft;
				StringToFileTime(g_FileTime.c_str(), ft);
				if (CompareFileTime((*it)->getLastWriteTime(),  &ft) != 0) {
					return 1;
				}
			}
		}
	}

	return 0;
}



// �p�X���[�h�_�C�A���O
String g_password;
String g_archive;
String g_file;
bool g_masksPassword = false;

// �T�C�Y�����Ȃ��Ō��ʂ� String �ɕԂ� GetWindowText()
void GetWindowText(HWND hWnd, String& String)
{
	DWORD dwSize;
	DWORD dwRet;
	LPTSTR pszString;
	dwSize = 256;
	for(;;){
		pszString = new TCHAR [dwSize];

		// �֐�����������ƃR�s�[���ꂽ������̕��������Ԃ�(�I�[�� NULL �����͊܂܂Ȃ�)
		dwRet = GetWindowText(hWnd, pszString, dwSize);

		if(dwRet == 0){
			// �G���[���̕�����
			delete [] pszString;
			String.Empty();
			return;
		}else if(dwRet == dwSize - 1){
			// �o�b�t�@������������
			delete [] pszString;
			dwSize*=2;
		}else{
			// ����
			break;
		}
	}
	String = pszString;
	delete [] pszString;
}

INT_PTR CALLBACK PasswordDialogProc(HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM /* lParam */) {

	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC1), g_archive.c_str());
		SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC2), g_file.c_str());
		if (g_masksPassword) {
			CheckDlgButton(hwndDlg, IDC_CHECK1, BST_CHECKED);
			SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), (UINT)EM_SETPASSWORDCHAR, (WPARAM)'*', (LPARAM) 0);  
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT1), g_password);
			g_masksPassword = (IsDlgButtonChecked(hwndDlg, IDC_CHECK1) == BST_CHECKED);
			EndDialog(hwndDlg, IDOK);
			break;
		case IDCANCEL:
			g_password.Empty();
			g_masksPassword = (IsDlgButtonChecked(hwndDlg, IDC_CHECK1) == BST_CHECKED);
			EndDialog(hwndDlg, IDCANCEL);
			break;
		case IDC_CHECK1:
			if (IsDlgButtonChecked(hwndDlg, IDC_CHECK1) == BST_CHECKED) {
				SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), (UINT)EM_SETPASSWORDCHAR, (WPARAM)'*', (LPARAM)0);  
			}else{
				SendMessage(GetDlgItem(hwndDlg, IDC_EDIT1), (UINT)EM_SETPASSWORDCHAR, (WPARAM)0, (LPARAM)0);
			}
			InvalidateRect(GetDlgItem(hwndDlg, IDC_EDIT1), NULL, FALSE);
			return TRUE;

		default:
			return FALSE;
		}
    default:
        return FALSE;
    }
}

// IDOK : ���͂��ꂽ
// IDCANCEL : �L�����Z�����ꂽ
int getPassword(LPCTSTR archive, LPCTSTR file, String& password) {
	g_archive = archive;
	g_file = file;
	INT_PTR r = DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)PasswordDialogProc);
	password = g_password;
	return (int)r;
}

// ���ɂ���������
bool process(LPCTSTR filename) 
{
	if (!FileExists(filename)) {
		cout << "���ɂ�������܂���" << endl;
		return false;
	}

	// ���ɂ���\���t�@�C���̃��X�g�𓾂�
	// ���̃��X�g���Q�ƁA���삵�A�K�v�Ȃ��̂̂݉�
	// �i�𓀂̍ہA�璷�t�H���_���l�����ēW�J����j
	// �𓀂��ꂽ���̂��w��̈��k���ň��k����

	// ���΃p�X�ł̎w������蓾��̂�
	// �A�[�J�C�u�t�@�C�������t���p�X�ɂ���
	String srcFilename(filename);
	GetFullPath(srcFilename);

	if (srcFilename.GetLength() > MAX_PATH) {
		cout << "���ɂ̃p�X���������܂�" << endl;
		return false;
	}

	// �����ς݂��ǂ����`�F�b�N
	int result = isProcessedArchive(srcFilename.c_str());
	if (g_PreCheck && result == 0) {
		cout << "�����͕s�v�ł�" << endl;

		// ���ڃt�@�C���̃`�F�b�N
		CheckRemarkableFile();

		// �t�@�C�������`�F�b�N
		String ext;
		GetExtention(srcFilename.c_str(), ext);
		if (lstrcmpi(ext.c_str(), _T(".zip")) == 0) {
			return true;
		}

		// ���̏��ɍ폜��ڕW�̖��O�ɕς���
		String destFilename(srcFilename);
		RemoveExtension(destFilename);
		destFilename.Cat(_T(".zip"));
		EvacuateFileName(destFilename);
		if (MoveFile(srcFilename.c_str(), destFilename.c_str()) == 0) {
			cout << "���ɂ̃��l�[���Ɏ��s���܂���" << endl;
			cout << destFilename.c_str() << endl;
		}
		return true;
	}

	ArchiveDll* pArchiveDll;
	pArchiveDll = g_ArchiveDllManager.getSuitableArchiveDll(srcFilename.c_str());
	if (pArchiveDll == NULL) {
		cout << "���ɂ������̂ɓK����DLL��������܂���" << endl;
		return false;
	}

	pArchiveDll->setArchiveFilename(srcFilename.c_str());






	// �𓀐�Ɏg���e���|�����t�H���_�𓾂�
	String destPath;
	getTemporaryPath(destPath);

	// ��ӂȖ��O�̃t�H���_�����
	CatPath(destPath, "uz");
	EvacuateFolderName(destPath);

	// �t�@�C���������Ƀt�H���_�����쐬����ꍇ
	//CatPath(destPath, srcFilename.c_str());
	//RemoveExtension(destPath);
	//evacuateFolderName(destPath);

	// destPath �Ɉ�ӂȃt�H���_�����������̂ł��̃t�H���_���쐬
	TemporaryFolder temp_folder;
	if (!temp_folder.create(destPath.c_str())) {
		cout << "�e���|�����t�H���_�쐬�Ɏ��s���܂���" << endl;
		return false;
	}

	if (g_BatchDecompress) {
		// ���ɓ��̃t�@�C�����ɂ���Ă� UNZIP32.DLL �ŉ𓀂����݂�ƈُ�I������̂�
		// ���ɓ��̃t�@�C�������擾�ł��Ă���ꍇ���̃t�@�C�������`�F�b�N
		if (result >= 0) {
			list<boost::shared_ptr<IndividualInfo> >::iterator it;
			for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
				bool error = false;
				String filename = (*it)->getFilename();
				String path = destPath;
				CatPath(path, filename.c_str());

				// ���ɓ��̃t�@�C���̃p�X���������Ȃ���
				if (path.GetLength() > 255) {
					error = true;
				}

				// NTFS�̃t�@�C�����Ɏg���Ȃ������́u?  "  /  \  <  >  *  |  :�v
				// �p�X�̋�؂�Ɏg���Ă��� '/' �� '\\' �̃`�F�b�N�͏ȗ�
				if (filename.FindOneOf(":") != -1) {
					error = true;
				}
				if (path.FindOneOf("?/<>*|") != -1) {
					error = true;
				}

				if (error) {
					cout << "�𓀎��ɃG���[���������܂���" << endl;
					return false;
				}
			}
		} else {
			// g_compressFileList ���擾�ł��Ă��Ȃ��Ƃ��̓`�F�b�N�͂�����߂�
			// ���s����̂� UNZIP32.DLL �ł̎��ł��邪
			// ��{�I��ZIP�t�@�C���ł���Ȃ�Ύ擾�o���Ă���͂�
			// �R�X�g�������Ă܂Ń`�F�b�N���Ȃ�
		}

		// �ꊇ��
		int r = pArchiveDll->extract(temp_folder.getPath(), g_ShowsProgress, true);

		if (r != 0) {
			cout << "�𓀎��ɃG���[���������܂���" << endl;
			return false;
		}

		// �t�@�C�����擾
		if (!GetIndividualInfo(temp_folder.getPath())) {
			return false;
		}
	} else {
		// �𓀂���t�@�C���̏��𖢎擾�Ȃ�Ύ擾
		if (result == 1) {
			// �������͕s���S�ł���\��������̂�
			// �\�[�g���Č����Ă���t�H���_��₤
			g_compressFileList.sort(IndividualInfoSortCriterion());
			AddLackFolder();
		} else if (result == -1) {
			if (!pArchiveDll->openArchive(NULL, 0)) {
				cout << "���ɂ��J���܂���" << endl;
				return false;
			}
			if (!GetIndividualInfo(*pArchiveDll)) {
				return false;
			}
			pArchiveDll->closeArchive();
		}
	}

	// ���̎��_�ŏ��ɓ��Ƀt�@�C����������Ȃ������ꍇ
	// ��̏��ɁA�������́A���ċ�Ɍ����鏑�ɂƂ������ɂȂ�
	if (g_compressFileList.empty()) {
		cout << "���ɓ��Ƀt�@�C������������܂���" << endl;
		return false;
	}

	// ���ڃt�@�C���̃`�F�b�N
	int r = CheckRemarkableFile();
	if (r == 0) {
	} else if (r == 1) {
		return true;
	} else {
		return false;
	}

	// �ǉ��t�@�C���̃`�F�b�N
	if (CheckAppendFile(temp_folder.getPath()) == false) {
		return false;
	}

	// ���O�t�@�C���̃`�F�b�N
	if (CheckExcludeFile(temp_folder.getPath()) == false) {
		return false;
	}

	// ��t�H���_�̏��O
	if (g_DeletesEmptyFolder) {
		if (deleteEmptyFolder(temp_folder.getPath()) == false) {
			return false;
		}
	}

	// �p�X���[�h�t���t�@�C���̃`�F�b�N
	if (g_IgnoresEncryptedFiles && g_BatchDecompress == false) {
		list<boost::shared_ptr<IndividualInfo> >::iterator it = g_compressFileList.begin();
		while (it != g_compressFileList.end()) {
			if ((*it)->isEncrypted()) {
				cout << "�Í������ꂽ�t�@�C�� " << (*it)->getFullPath() << " ���폜"<< endl;
				it = g_compressFileList.erase(it);
			} else {
				++it;
				continue;
			}
		}
	}

	#ifdef _DEBUG
	{
		cout << "�璷�t�H���_���o�O" << endl;
		list<boost::shared_ptr<IndividualInfo> >::iterator it;	
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {
			cout << (*it)->getFullPath() << endl;
		}
	}
	#endif

	// �璷�t�H���_�̌��o
	String redundantPath;
	int redundantPathLen;
	if (g_RemovesRedundantFolder != 0) {
		GetRedundantPath(redundantPath);
		if (redundantPath.IsEmpty() == false) {
			cout << "�璷�t�H���_ " << redundantPath.c_str() << " ��Z�k" << endl;
		}
	}
	// �璷�p�X�̒����𓾂�
	// �������璷�p�X���p�X�����̂Ɏg���̂Ńp�X���Ȃ�'\\'�̕������C���N�������g
	redundantPathLen = redundantPath.GetLength();
	if (redundantPathLen != 0 && redundantPath.ReverseFind(TCHAR('\\')) != redundantPathLen - 1) {
		redundantPathLen += 1;
	}

	// ��

	// �w��̃t�@�C�����w��̏ꏊ�ɉ�
	if (g_BatchDecompress == false) {
		bool hasValidPassword = false;
		String password;

		list<boost::shared_ptr<IndividualInfo> >::iterator it;
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {

			String extractFilename = (*it)->getFullPath();
			if (extractFilename.GetLength() <= redundantPathLen) continue;

			String destPath(temp_folder.getPath());
			CatPath(destPath, extractFilename.c_str() + redundantPathLen);

			// TODO �t�@�C�������s���ȕ������g���Ă��Ȃ����`�F�b�N���ׂ���

			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				// �t�H���_�̓p�X���������Ă��Ă��֌W�Ȃ�

				// UNLHA32.dll �̓t�H���_���𓀂ł��Ȃ��悤�Ȃ̂Ŏ��O�ō쐬
				if (CreateDirectory(destPath.c_str(), NULL) == 0) {
					cout << "�𓀎��ɃG���[���������܂���" << endl;
					return false;
				}
			} else {
				// �𓀐�̓t�@�C��������菜�����ꏊ�ɂȂ�
				RemoveFileName(destPath);

				bool isEncrypted = (*it)->isEncrypted();

				RETRY:

				// �p�X���[�h����
				if (isEncrypted) {
					if (hasValidPassword == false) {
						if (getPassword(pArchiveDll->getArchiveFilename(), (*it)->getFullPath(), password) == IDCANCEL) {
							cout << "�p�X���[�h�̓��͂��L�����Z������܂���" << endl;
							return false;
						}
					}

					int r = pArchiveDll->extract(extractFilename.c_str(), destPath.c_str(), g_ShowsProgress, password.c_str());
					if (r == 0) {
						hasValidPassword = true;
						continue;
					} else if (r == -1) {
						// �p�X���[�h���Ⴄ
						if (hasValidPassword) {
							hasValidPassword = false;
						} else {
							cout << "�p�X���[�h���Ⴂ�܂�" << endl;
						}
						goto RETRY;
					} else {
						cout << "�𓀎��ɃG���[���������܂���" << endl;
						return false;
					}
				} else {
					int r = pArchiveDll->extract(extractFilename.c_str(), destPath.c_str(), g_ShowsProgress);
					if (r==0) {
						// ����
						continue;
					} else if (r == -1) {
						isEncrypted = true;
						goto RETRY;
					} else {
						cout << "�𓀎��ɃG���[���������܂���" << endl;
						return false;
					}
				}
			}
		}
	}

	// �悭������Ȃ����ꉞ�O��DLL�̏������I���悤�ɂ��Ă݂�
	{
		DWORD startTime = timeGetTime();
		while (pArchiveDll->getRunning()) {
			Sleep(10);
			if (timeGetTime() - startTime > 3000) {
				cout << "�𓀎��ɃG���[���������܂���" << endl;
				return false;
			}
		}
	}

	// �t�@�C���E�t�H���_�̓����ݒ�
	if (g_FolderTimeMode != 2 || g_FileTimeMode != 0) {
		list<boost::shared_ptr<IndividualInfo> >::iterator it;
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {

			String path(temp_folder.getPath());
			if (lstrlen((*it)->getFullPath()) <= redundantPathLen) continue;
			
			if (g_BatchDecompress) {
				CatPath(path, (*it)->getFullPath());
			} else {
				CatPath(path, (*it)->getFullPath() + redundantPathLen);
			}

			if (((*it)->getAttribute() & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				// �t�H���_�̏ꍇ
				if (g_FolderTimeMode != 0 && g_FolderTimeMode != 1) continue;

				DWORD attribute = (*it)->getAttribute();

				// �t�H���_�͎��O�ō쐬���ꂽ���̂Ȃ̂�
				// �ǂݍ��ݐ�p�����͂��Ă��Ȃ�
				// �������ꊇ�𓀂̏ꍇ�͂��̌���ł͂Ȃ�
				// �������ǂݍ��ݐ�p���ƍX�V�����ύX�Ɏ��s����̂ňꎞ�I�ɕύX����
				bool isReadOnly = false;
				if (g_BatchDecompress && (attribute & FILE_ATTRIBUTE_READONLY) != 0) {
					if (SetFileAttributes(path.c_str(), (attribute & (~FILE_ATTRIBUTE_READONLY)))) {
						isReadOnly = true;
					} else {
						cout << "�t�H���_�̑����ꎞ�ύX�Ɏ��s���܂���" << endl;
					}
				}

				HANDLE hFile = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
				if (hFile == INVALID_HANDLE_VALUE) {
					cout << "�t�H���_�̍X�V�����ύX�Ɏ��s���܂���" << endl;
				} else {
					FILETIME ft;
					if (g_FolderTimeMode == 0) {
						// �t�H���_�͉𓀎��̓��t�ɂȂ��Ă��܂��̂Œ���				
						ft = *((*it)->getLastWriteTime());
					} else if (g_FolderTimeMode == 1) {
						StringToFileTime(g_FolderTime.c_str(), ft);
					}
					if (SetFileTime(hFile, NULL, NULL, &ft) == 0) {
						cout << "�t�H���_�̍X�V�����ύX�Ɏ��s���܂���" << endl;
					}
					CloseHandle(hFile);
				}

				// ���������ɖ߂�
				if (isReadOnly && g_FolderAttribute == -1) {
					if (!SetFileAttributes(path.c_str(), attribute)) {
						cout << "�t�H���_�̑������A�Ɏ��s���܂���" << endl;					
					}
				}
			} else {
				if (g_FileTimeMode != 1) continue;

				DWORD attribute = (*it)->getAttribute();

				// �������ǂݍ��ݐ�p���ƍX�V�����ύX�Ɏ��s����̂ňꎞ�I�ɕύX����
				bool isReadOnly = false;
				if ((attribute & FILE_ATTRIBUTE_READONLY) != 0) {
					if (SetFileAttributes(path.c_str(), (attribute & (~FILE_ATTRIBUTE_READONLY)))) {
						isReadOnly = true;
					} else {
						cout << "�t�@�C���̑����ꎞ�ύX�Ɏ��s���܂���" << endl;
					}
				}

				if (SetFileLastWriteTime(path.c_str(), g_FileTime.c_str()) == false) {
					cout << "�t�@�C���̍X�V�����ύX�Ɏ��s���܂���" << endl;
				}

				// ���������ɖ߂�
				if (isReadOnly && g_FileAttribute == -1) {
					if (!SetFileAttributes(path.c_str(), attribute)) {
						cout << "�t�@�C���̑������A�Ɏ��s���܂���" << endl;					
					}
				}
			}
		}
	}

	if (g_FolderTimeMode == 2) {
		setFolderTime(temp_folder.getPath());
	}

	// �t�@�C���E�t�H���_�̑����ݒ�
	if (g_FolderAttribute != -1 || g_FileAttribute != -1) {
		list<boost::shared_ptr<IndividualInfo> >::iterator it;
		for (it = g_compressFileList.begin(); it != g_compressFileList.end(); ++it) {

			String path(temp_folder.getPath());
			if (lstrlen((*it)->getFullPath()) <= redundantPathLen) continue;

			if (g_BatchDecompress) {
				CatPath(path, (*it)->getFullPath());
			} else {
				CatPath(path, (*it)->getFullPath() + redundantPathLen);
			}

			DWORD attribute = (*it)->getAttribute();

			if ((attribute & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				// �t�H���_�̏ꍇ
				DWORD newAttribute;
				if (g_FolderAttribute == -1) {
					newAttribute = attribute;
				} else {
					newAttribute = FILE_ATTRIBUTE_DIRECTORY;
					if ((g_FolderAttribute & FileAttribute::READ_ONLY) != 0)	newAttribute |= FILE_ATTRIBUTE_READONLY;
					if ((g_FolderAttribute & FileAttribute::HIDDEN) != 0)		newAttribute |= FILE_ATTRIBUTE_HIDDEN;
					if ((g_FolderAttribute & FileAttribute::SYSTEM) != 0)		newAttribute |= FILE_ATTRIBUTE_SYSTEM;
					if ((g_FolderAttribute & FileAttribute::ARCHIVE) != 0)		newAttribute |= FILE_ATTRIBUTE_ARCHIVE;
				}
				if (SetFileAttributes(path.c_str(), newAttribute) == 0) {
					cout << "�t�H���_�̑����ύX�Ɏ��s���܂���" << endl;
					continue;
				}
			} else {
				if (g_FileAttribute != -1) {
					DWORD newAttribute = 0;
					if ((g_FileAttribute & FileAttribute::READ_ONLY) != 0)	newAttribute |= FILE_ATTRIBUTE_READONLY;
					if ((g_FileAttribute & FileAttribute::HIDDEN) != 0)		newAttribute |= FILE_ATTRIBUTE_HIDDEN;
					if ((g_FileAttribute & FileAttribute::SYSTEM) != 0)		newAttribute |= FILE_ATTRIBUTE_SYSTEM;
					if ((g_FileAttribute & FileAttribute::ARCHIVE) != 0)	newAttribute |= FILE_ATTRIBUTE_ARCHIVE;
					if (newAttribute == 0) newAttribute = FILE_ATTRIBUTE_NORMAL;
					if (newAttribute == attribute) continue;
					if (SetFileAttributes(path.c_str(), newAttribute) == 0) {
						cout << "�t�@�C���̑����ύX�Ɏ��s���܂���" << endl;
						continue;
					}
				}
			}
		}
	}

	// ���k���悤�Ƃ���t�H���_����
	// �w��̊g���q�̃t�@�C��(�Ⴆ�΃A�[�J�C�u)�݂̂�������
	// ���̃t�H���_�ɂ��̃t�@�C�����ړ����ďI���

	// ���k���悤�Ƃ���t�H���_������ł�������I��
	// �i��̃A�[�J�C�u�ƂȂ邱�Ƃ�\���j
	if (IsEmptyFolder(temp_folder.getPath())) {
		cout << "��̏��ɂƂȂ邽�ߏ��ɂ��쐬���܂���" << endl;

		// �e���|�����t�H���_�̍폜(�����I�ɍs��)
		if (!temp_folder.destroy()) {
			// �t�H���_�폜���s
			cout << "�e���|�����t�H���_�̍폜�Ɏ��s���܂���" << endl;
			cout << temp_folder.getPath() << endl;
		}

		// ���̏��ɂ��폜
		if (!DeleteFileOrFolder(srcFilename.c_str(), UsesRecycleBin)) {
			// �t�@�C���폜���s
			cout << "���̏��ɂ̍폜�Ɏ��s���܂���" << endl;
			cout << srcFilename.c_str() << endl;
		}

		return true;
	}

	// �쐬����A�[�J�C�u�t�@�C�������w��
	// ��{�I�Ɍ��Ɠ����t�@�C�����ɂ���
	// �������A�g���q�� zip
	// ���ƈقȂ�ꍇ�͊��ɑ��݂���t�@�C�����������
	String destFilename(srcFilename);
	String ext;
	GetExtention(destFilename.c_str(), ext);
	if (lstrcmpi(ext.c_str(), _T(".zip")) != 0) {
		RemoveExtension(destFilename);
		destFilename.Cat(_T(".zip"));
		EvacuateFileName(destFilename);
	}

	// ���̃A�[�J�C�u�Ɠ����t�@�C�����̏ꍇ��
	// �ꎞ�I�ɕʂ̃t�@�C�����ō쐬��
	// ���̏��ɍ폜��t�@�C������߂�
	String finalDestFilename(destFilename);
	if (redundantPathLen != 0 && g_RemovesRedundantFolder == 2) {
		StripPath(finalDestFilename);
		String fileName(redundantPath);
		int n = fileName.Find(_TCHAR('\\'));
		if (n > 0) {
			fileName.MakeLeft(n);
		}
		fileName.Cat(_T(".zip"));
		CatPath(finalDestFilename, fileName.c_str());
	}

	if (destFilename == srcFilename) {
		EvacuateFileName(destFilename);
	}

	// �w��̖��O�ŕK�v�ȃt�H���_�ȉ������k
	{
		bool b;

		if (g_BatchDecompress) {
			// �ꊇ�𓀂̏ꍇ�͂����ŏ璷�t�H���_���l������
			String path = temp_folder.getPath();
			CatPath(path, redundantPath.c_str());
			b = compress(path.c_str(), destFilename.c_str(), g_CompressLevel, g_ShowsProgress);
		} else {
			b = compress(temp_folder.getPath(), destFilename.c_str(), g_CompressLevel, g_ShowsProgress);
		}

/*
		if (pArchiveDll->getID() != ArchiveDllID::SEVEN_ZIP) {
			pArchiveDll = g_ArchiveDllManager.getArchiveDll(ArchiveDllID::SEVEN_ZIP);
			if (pArchiveDll == NULL) {
				pArchiveDll = g_ArchiveDllManager.addArchiveDll(ArchiveDllID::SEVEN_ZIP);
			}
			if (pArchiveDll == NULL) {
				cout << "���ɂ����k����̂ɓK����DLL��������܂���" << endl;
				return false;
			}
		}
		if (g_BatchDecompress) {
			// �ꊇ�𓀂̏ꍇ�͂����ŏ璷�t�H���_���l������
			String path = temp_folder.getPath();
			CatPath(path, redundantPath.c_str());
			b = pArchiveDll->compress(path.c_str(), destFilename.c_str(), g_CompressLevel, g_ShowsProgress);
		} else {
			b = pArchiveDll->compress(temp_folder.getPath(), destFilename.c_str(), g_CompressLevel, g_ShowsProgress);
		}
*/

		if (b == false) {
			cout << "���k���ɃG���[���������܂���" << endl;
			return false;
		}
	}

	// �e���|�����t�H���_�̍폜(�����I�ɍs��)
	if (!temp_folder.destroy()) {
		// �t�H���_�폜���s
		cout << "�e���|�����t�H���_�̍폜�Ɏ��s���܂���" << endl;
		cout << temp_folder.getPath() << endl;
	}

	// ���̏��ɂ��폜
	if (!DeleteFileOrFolder(srcFilename.c_str(), UsesRecycleBin)) {
		// �t�@�C���폜���s
		cout << "���̏��ɂ̍폜�Ɏ��s���܂���" << endl;
		cout << srcFilename.c_str() << endl;
	}

	// ���̏��ɍ폜��ڕW�̖��O�ɕς���
	if (finalDestFilename != destFilename) {
		// �O�̂��߃t�@�C�����f�B�X�N��Ŏ��ۂɈړ�����܂ő҂�
		// MoveFileEx() �� Windows 2000 �ȍ~�ł����g���Ȃ�
		int n = 2;
		for (int i = 0; i < n; ++i) {
			if (MoveFileEx(destFilename.c_str(), finalDestFilename.c_str(), MOVEFILE_WRITE_THROUGH) != 0) {
				break;
			} else if (i < n - 1) {
				Sleep(3000);
			} else {
				cout << "�쐬�������ɂ̃��l�[���Ɏ��s���܂���" << endl;
				cout << destFilename.c_str() << endl;
			}
		}
	}

	// �����������ʒ��ڃt�@�C���݂̂ō\�������ꍇ
	// ���̎|��\������
	CheckRemarkableFile();

	return true;
}






// �w��̃t�H���_���i�T�u�t�H���_�܂ށj�̃t�H���_�̍X�V������
// �t�H���_���̍ŐV�̍X�V���������t�@�C���E�t�H���_�ƈ�v������
void setFolderTime(LPCTSTR path) {
	FILETIME ft;
	setFolderTimeSub(path, ft);
}

bool setFolderTimeSub(LPCTSTR path, FILETIME& rNewestFileTime) {

	rNewestFileTime.dwLowDateTime = 0;
	rNewestFileTime.dwHighDateTime = 0;

	bool isEmptyFolder = true;

	WIN32_FIND_DATA fd;
	HANDLE hSearch;

	// �T���p�X���쐬
	String searchPath(path);
	FormatTooLongPath(searchPath);
	CatPath(searchPath, _T("*.*"));

	hSearch = FindFirstFile(searchPath.c_str(), &fd);

	if (hSearch==INVALID_HANDLE_VALUE) return false;

	do {
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0){
			// ���[�g�y�уJ�����g�łȂ��t�H���_���ǂ����`�F�b�N
			if (lstrcmp(fd.cFileName,_T("."))!=0 && lstrcmp(fd.cFileName,_T("..")) != 0) {
				isEmptyFolder = false;

				searchPath = path;
				CatPath(searchPath, fd.cFileName);
				FILETIME ft;

				if (setFolderTimeSub(searchPath.c_str(), ft) == false) continue;

				// �������ǂݍ��ݐ�p���ƍX�V�����ύX�Ɏ��s����̂ňꎞ�I�ɕύX����
				bool isReadOnly = false;
				if ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {
					isReadOnly = true;
					SetFileAttributes(searchPath.c_str(), (fd.dwFileAttributes & (~FILE_ATTRIBUTE_READONLY)));
				}
				HANDLE hFile = CreateFile(searchPath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
				if (hFile == INVALID_HANDLE_VALUE) {
					cout << "�t�H���_�̍X�V�����ύX�Ɏ��s���܂���" << endl;
				} else {
					if (SetFileTime(hFile, NULL, NULL, &ft) == 0) {
						cout << "�t�H���_�̍X�V�����ύX�Ɏ��s���܂���" << endl;
						GetLastErrorMssage();
					}
					CloseHandle(hFile);
				}
				// ���������ɖ߂�
				if (isReadOnly && g_FolderAttribute == -1) {
					SetFileAttributes(searchPath.c_str(), fd.dwFileAttributes);
				}

				if (CompareFileTime(&rNewestFileTime,  &ft) == -1) {
					rNewestFileTime = ft;
				}
			}
		} else {
			isEmptyFolder = false;

			if (CompareFileTime(&rNewestFileTime, &fd.ftLastWriteTime) == -1) {
				rNewestFileTime = fd.ftLastWriteTime;
			}
		}
	} while (FindNextFile(hSearch,&fd)!=0);

	FindClose(hSearch);

	return (!isEmptyFolder);
}



bool compress(LPCTSTR srcPath, LPCTSTR dstPath, int compressLevel, bool /* showsProgress */)
{
	// �t�H���_�ȉ������k
	bool b;
	ZipWriter zw;
	b = zw.open(dstPath);
	if (!b) return false;

	// basePathLength �� ������ '\\' �����Ă��邩�ǂ����ŕς��
	int basePathLength = lstrlen(srcPath);
	if (strrchrex(srcPath, '\\') != srcPath+(basePathLength-1)) {
		basePathLength++;
	}

	b = compressSub(srcPath, zw, basePathLength);
	if (!b) return false;
	zw.setCompressLevel(compressLevel);
	int n = zw.close();
	if (n != 0) return false;
	return true;
}

// �w��t�H���_�����������ăt�@�C����ǉ�
// basePathLength �� ZipWriter �ɒǉ�����t�@�C���̃t���p�X���牽����
// ������� Zip ���ł̂��̃t�@�C���̃p�X�ɂȂ邩�i'\\' �������ɂ��Ă�p�X�Ȃ̂��𒍈Ӂj
bool compressSub(LPCTSTR pszDir, ZipWriter& zw, int basePathLength)
{
	WIN32_FIND_DATA fd;

	String searchPath;
	HANDLE hSearch;

	// �T���p�X���쐬
	searchPath += pszDir;
	FormatTooLongPath(searchPath);
	CatPath(searchPath, _T("*.*"));

	hSearch = FindFirstFile(searchPath.c_str(), &fd);

	if (hSearch==INVALID_HANDLE_VALUE) return false;

	do {
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0) {
			// ���[�g�y�уJ�����g�łȂ��t�H���_���ǂ����`�F�b�N
			if (lstrcmp(fd.cFileName,_T("."))!=0 && lstrcmp(fd.cFileName,_T(".."))!=0) {

				String path = pszDir;
				CatPath(path, fd.cFileName);

				// �t�H���_�̃p�X��ǉ�
				zw.add(path.c_str()+basePathLength, path.c_str());

				// �T�u�t�H���_�̌���
				if(!compressSub(path.c_str(), zw, basePathLength)) {
					return false;
				}
			}
		}else{
			String path = pszDir;
			CatPath(path, fd.cFileName);

			// �t�@�C���̃p�X��vector�ɒǉ�
			zw.add(path.c_str()+basePathLength, path.c_str());
		}
		// ���b�Z�[�W����
		//DoEvents();
	} while (FindNextFile(hSearch, &fd)!=0);

	FindClose(hSearch);

	return true;
}



