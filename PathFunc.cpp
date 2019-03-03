#include "PathFunc.h"

#include <tchar.h>

#include <boost/shared_ptr.hpp>

#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

#include "String.h"
#include "StrFunc.h"

using namespace std;
using namespace boost;

namespace KSDK {

/*
char* GetFullPath(const char* pszFile, const char* pszDefPath, const char* pszDefExt, char* pszPath)
{
	char* psz;

	*pszPath = _TCHAR('\0');
	if(!IsFullPathEx(pszFile)){
		// �t�@�C���̎w�肪���΃p�X�̏ꍇ�f�t�H���g�̃p�X������
		if(pszDefPath != NULL && *pszDefPath != _TCHAR('\0')){
			strcpy(pszPath, pszDefPath);
			// �p�X�̍Ō�Ɂu\�v������
			if(strrchrex(pszPath, _TCHAR('\\')) != pszPath + strlen(pszPath)-1) strcat(pszPath, "\\");
		}
	}
	strcat(pszPath, pszFile);

	// �g���q�̎w�肪�����ꍇ�̓f�t�H���g�̊g���q������
	psz = strrchrex(pszFile, _TCHAR('\\'));
	if(psz == NULL){psz = (char*)pszFile;}
	if(strrchrex(psz, _TCHAR('.')) == NULL){
		strcat(pszPath, pszDefExt);
	}

	return pszPath;
}
*/


// ���΃p�X�ɃJ�����g�f�B���N�g�������ăt���p�X�ɂ���
bool GetFullPath(String& Path) {
	String Temp;

	if (IsFullPathEx(Path.c_str())) {
		return true;
	}

	GetCurrentDirectory(Temp);
	CatPath(Temp, Path.c_str());
	Path = Temp;

	return true;
}



bool IsUNCPath(LPCTSTR pszPath)
{
	if(*pszPath == _TCHAR('\0'))
		return false;

	if(pszPath[0] == _TCHAR('\\') && pszPath[1] == _TCHAR('\\'))
		return true;

	return false;
}



bool IsFullPath(LPCTSTR pszPath)
{
	if(*pszPath == _TCHAR('\0'))
		return false;

	if(pszPath[1] == _TCHAR(':')){
		return true;
	}
	return false;
}



bool IsFullPathEx(LPCTSTR pszPath)
{
	return (IsUNCPath(pszPath) || IsFullPath(pszPath));
}

/*
// ���ꂪ�t�@�C���̃p�X���`�F�b�N����
// �i�P�Ɋg���q�����邩�`�F�b�N���邾���j
bool IsFilePath(LPCTSTR pszPath)
{
	char* psz;

	psz = strrchrex(pszPath, _TCHAR('\\'));

	if(psz == NULL)
		psz = (char*)pszPath;

	if(strchrex(psz, _TCHAR('.')) != NULL)
		return true;

	return false;
}
*/


// �w��̃t�@�C�������݂��邩
// �f�B���N�g�����w��\
// �t���p�X�łȂ��ꍇ�J�����g�f�B���N�g���ȉ��̂��̂Ƃ݂Ȃ����
// NULL ���w�肵���ꍇ false ��Ԃ�
bool FileExists(LPCTSTR pszPath)
{
	return (GetPathAttribute(pszPath) != PATH_INVALID);
}



// �擪�̃X�y�[�X �����̃X�y�[�X�ƃs���I�h ���폜
void FixFileName(String& FileName)
{
	FileName.TrimLeft(_TCHAR(' '));
	FileName.TrimRight(_T(" ."));
}



// �������`���ɂ���
// �i�ŏ��Ɂu\�v�����������菜���j
// �i������2�ȏ�u\�v�������ꍇ�̓l�b�g���[�N�p�X�ƍl��2�c���j
// �i�Ō�Ɂu\�v�����������菜�� ������ �uc:\�v�̗l�Ƀ��[�g�̏ꍇ�͂����Ȃ�Ȃ��j
void FixPath(String& Path)
{
	int nPos = 0;
	int n = 0;

	for(;;){
		if(Path[nPos++] == _TCHAR('\\')){
			n++;
		}else{
			break;
		}
	}

	// �ŏ��Ɂu\�v�����������菜��
	// ������2�ȏ�u\�v�������ꍇ�̓l�b�g���[�N�p�X�ƍl��2�c��
	if(n == 0 || n == 2){

	}else if(n == 1){
		Path.Delete(0, 1);
	}else{
		Path.Delete(0, n - 2);
	}

	// �Ō�Ɂu\�v�����������菜��
	// �uc:\�v�̗l�Ƀ��[�g�̏ꍇ�͂����Ȃ�Ȃ�
	if(Path.ReverseFind(_TCHAR('\\')) == Path.GetLength()-1){
		if(!IsFullPath(Path.c_str()) || Path.GetLength() != (int)_tcslen(_T("C:\\"))){
			Path.Delete(Path.GetLength()-1, 1);
		}
	}
}


// �e�X�g�ς�
// �T�C�Y�����Ȃ��Ō��ʂ� String �ɕԂ� GetCurrentDirectory()
void GetCurrentDirectory(String& string)
{
	DWORD dwSize;
	DWORD dwRet;
	LPTSTR pszString;

	dwSize = MAX_PATH;

	for(;;){
		pszString = new TCHAR [dwSize];

		// �֐�����������ƃo�b�t�@�ɏ������܂ꂽ������ (�I�[�� NULL ����������) ���Ԃ�
		// �o�b�t�@�̃T�C�Y�������������Ƃ��͕K�v�ȃo�b�t�@�̃T�C�Y (�I�[�� NULL �������܂�)���Ԃ�
		dwRet = ::GetCurrentDirectory(dwSize, pszString);

		if(dwRet == 0){
			// �G���[
			string.Empty();
			delete [] pszString;
			return;
		}else if(dwRet > dwSize){
			// �o�b�t�@������������
			dwSize = dwRet;
			delete [] pszString;
		}else{
			// ����
			break;
		}
	}
	string = pszString;
	delete [] pszString;
}


// �e�X�g�ς�
// �T�C�Y�����Ȃ��Ō��ʂ� String �ɕԂ� GetModuleFileName()
void GetModuleFileName(HMODULE hModule, String& filename)
{
	DWORD dwSize = 256;
	DWORD dwRet;
	LPTSTR pszString;

	for(;;){
		pszString = new TCHAR [dwSize];

		// �R�s�[���ꂽ������̒����������P�ʁiNULL�������������������j�ŕԂ�
		// ���ʂ����������ꍇ�؂�̂Ă���
		// �؂�̂Ă��ꍇ�I�[��NULL�����͕t���Ȃ�
		dwRet = ::GetModuleFileName(hModule, pszString, dwSize);

		if(dwRet == 0){
			// �G���[
			delete [] pszString;
			filename.Empty();
			return;
		}else if(dwRet == dwSize){
			// �o�b�t�@������������
			delete [] pszString;
			dwSize*=2;
		}else{
			// ����
			break;
		}
	}
	filename.Copy(pszString);
	delete [] pszString;
}

// MAX_PATH�𒴂��钷���̃p�X��
// ���C�h���� (W) �o�[�W������ API �֐��ɓn���Ƃ���
// �t�H�[�}�b�g�ɕϊ�����
void FormatTooLongPath(String& String)
{
	if(String.GetLength() <= MAX_PATH) return;

	if(IsUNCPath(String.c_str())){
		String.Delete(0, 1);// �擪��\���ЂƂ���
		String.Insert(0, _T("\\\\?\\UNC"));
	}else{
		String.Insert(0, _T("\\\\?\\"));
	}
}

// �w��̃p�X���t�H���_���t�@�C������Ԃ�
PathAttribute GetPathAttribute(LPCTSTR pszPath)
{
	String Path;

	if(IsFullPathEx(pszPath)){
		Path = pszPath;
	}else{
		GetCurrentDirectory(Path);
		CatPath(Path, pszPath);
	}

	FormatTooLongPath(Path);
	DWORD dw = GetFileAttributes(Path.c_str());

	if(dw == 0xFFFFFFFF){
		return PATH_INVALID;
	}else if(dw & FILE_ATTRIBUTE_DIRECTORY){
		return PATH_FOLDER;
	}else{
		return PATH_FILE;
	}
}

// �f�B���N�g���̍쐬�i�����K�w�̃f�B���N�g�����쐬�\�j
// �t���p�X�̂ݑΉ�
bool CreateDirectoryEx(LPCTSTR path) 
{
	String temp = path;
	LPCTSTR p;

	// ���̃f�B���N�g�������ɑ��݂���ꍇ�͏I��
	if(FileExists(path)) return true;

	// ���[�g�f�B���N�g�����牺�ʃf�B���N�g���Ɍ�������
	// ���̃f�B���N�g�������邩�`�F�b�N�Ȃ���΍쐬
	// ���J��Ԃ�

	// �͂��߂́u\�v�̈ʒu�𓾂�
	// �u\�v��������Ȃ��ꍇ�͕s���ȃp�X
	// �͂��߂́u\�v�̓h���C�u���̎��ɂ���͂��Ȃ̂�2�Ԗڈȍ~��\�ŋ�؂��Ă���

	if((p = strchrex(path, _TCHAR('\\'))) == NULL) return false;

	temp.NCopy(path, (int)(p-path+1));

	// �h���C�u�̑��݂��m�F
	if(FileExists(temp.c_str()) == false) return false;

	for(;;){
		if((p = strchrex(p+1, _TCHAR('\\'))) == NULL) break;
		temp.NCopy(path, (int)(p-path));
		if(!FileExists(temp.c_str())){
			// �w�肵���f�B���N�g�������݂��Ȃ�������쐬
			// �f�B���N�g���̍쐬�Ɏ��s������I��
			if(CreateDirectory(temp.c_str(), NULL) == 0) return false;
		}
	}

	// �w��̃f�B���N�g���̃��[�g�f�B���N�g���܂ł͍쐬���ꂽ�͂��Ȃ̂�
	// �Ō�Ɏw��̃f�B���N�g�����쐬����
	// �f�B���N�g���̍쐬�Ɏ��s������I��
	if (CreateDirectory(path, NULL) == 0) {
		return false;
	}

	return true;
}


// �p�X����t�@�C��������菜��
// �L���ȃp�X�ł��邩�͊m���߂Ȃ�
bool RemoveFileName(String& path)
{
	return (StripPath(path, 1) == 1);
}

// �t���p�X����t�@�C��������菜��
// �L���ȁi���݂���j�p�X�ɂ��Ă̂ݗL��
bool RemoveFileNameEx(String& path)
{
	switch(GetPathAttribute(path.c_str())){
	case PATH_FILE:
		return RemoveFileName(path);
	case PATH_FOLDER:
		return true;
	}
	return false;
}

// �p�X����g���q����菜��
// �L���ȃp�X�ł��邩�͊m���߂Ȃ�
bool RemoveExtension(String& path)
{
	int nPos0, nPos1;
	nPos0 = path.ReverseFind(_TCHAR('\\'));

	if(nPos0 == -1){
		nPos0 = 0;
	}
	nPos1 = path.ReverseFind(_TCHAR('.'));
	if(nPos0 < nPos1) path.MakeLeft(nPos1);
	return true;
}

// �g���q�𓾂�(�擪��'.'���܂�)
void GetExtention(LPCTSTR filename, String& ext) {

	String str = filename;

	int nPos0, nPos1;
	nPos0 = str.ReverseFind(_TCHAR('\\'));

	if(nPos0 == -1){
		nPos0 = 0;
	}
	nPos1 = str.ReverseFind(_TCHAR('.'));
	if(nPos0 < nPos1) {
		ext.Copy(str.c_str() + nPos1);
	} else {
		ext.Empty();
	}
}

// �J�����g�f�B���N�g���ݒ�
// ��΃p�X�ł��A���΃p�X�i�󕶎���ł��j�ł��w��\
// "\\data" �Ƃ� "C:\\data" �Ƃ������悤�Ɏw�肷��
// ��΃p�X�łȂ��ꍇexe�t�@�C�����܂ރt�H���_����̑��΃p�X�Ƃ݂Ȃ�
bool SetCurrentDirectoryEx(LPCTSTR pszDir)
{		
	if(IsFullPathEx(pszDir)){
		return (::SetCurrentDirectory(pszDir) != 0);
	}else{
		String Path;
		GetModuleFileName(NULL, Path);
		RemoveFileName(Path);
		CatPath(Path, pszDir);
		return (::SetCurrentDirectory(Path.c_str()) != 0);
	}
}

/*
// �p�X�̒ǉ�
void CatPath(char* pszPath1, const char* pszPath2)
{
	if(*pszPath2 == _TCHAR('\0')) return;

	// �󕶎���łȂ��čŌ�Ɂu\�v���Ȃ����������
	DWORD dwLen = strlen(pszPath1);
	if(	*pszPath1 != _TCHAR('\0')
	 && strrchrex(pszPath1, _TCHAR('\\')) != pszPath1 + dwLen-1){
		strcat(pszPath1,"\\"); // ��������t�@�C�����쐬
	}
	strcat(pszPath1, pszPath2);
}
*/

// �p�X�̒ǉ�
void CatPath(String& String, LPCTSTR pszPath)
{
	if(*pszPath == _TCHAR('\0')) return;

	// �󕶎���łȂ��čŌ�Ɂu\�v���Ȃ����������
	int nLen = lstrlen(String.c_str());
	if(	!String.IsEmpty()
	 && strrchrex(String.c_str(), _TCHAR('\\')) != String.c_str() + (nLen-1) ){
		String += _TCHAR('\\');
	}
	String+=pszPath;
}

// �t�H���_���󂩂ǂ������ׂ�
// TODO �V�X�e�������̃t�@�C���͖�������悤�ɂ���H
bool IsEmptyFolder(LPCTSTR pszPath) {
	// �u�w�肳�ꂽ�t�@�C����������܂���B�v�� GetLastError() ���Ԃ��l
	static const DWORD FILE_NOT_FOUND = 0x00000002;

	WIN32_FIND_DATA fFind;
	String Path;
	HANDLE hSearch;
	bool bFind;

	bFind = false;// �w��t�H���_���ɉ��炩�̃t�@�C���E�t�H���_����������

	// �T���p�X���쐬
	Path = pszPath;
	FormatTooLongPath(Path);
	CatPath(Path, _T("*"));

	hSearch = FindFirstFile(Path.c_str(), &fFind);

	if (hSearch == INVALID_HANDLE_VALUE) {
		if (GetLastError() == FILE_NOT_FOUND) {
			// ���[�g�f�B���N�g���ł��t�@�C�����S�����݂��Ȃ��ꍇ
			return true;
		} else {
			return false;
		}
	}

	do{
		// �f�B���N�g�����A���ʂ̃t�@�C����
		if((fFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0){// �f�B���N�g���ł�����
			// ���[�g�y�уJ�����g�łȂ��t�H���_���ǂ����`�F�b�N
			if(	_tcscmp(fFind.cFileName, _T(".")) != 0 &&
				_tcscmp(fFind.cFileName, _T("..")) != 0) {
				bFind = true;
				break;
			}
		}else{// ���ʂ̃t�@�C���ł�����
			bFind = true;
			break;
		}
	} while (FindNextFile(hSearch, &fFind)!=0);

	FindClose(hSearch);

	return  !bFind;
}

// 2�̃p�X�̐擪���狤�ʂ���p�X�̒�����Ԃ�
// �u���ʂ���p�X�v�Ȃ̂ő啶���������̈Ⴂ�͖�������
// �S�p�A���t�@�x�b�g�ɂ��čl����K�v����
int PathCommonPrefixLen(const char* psz1, const char* psz2)
{
	char *p1, *p2;
	char c1, c2;
	char* pBS;		// �Ō�́u\�v�̈ʒu
	bool bKanji;	// 2�o�C�g������2�����ڂ�

	bKanji = false;

	if(psz1 == NULL || psz2 == NULL) return 0;

	p1 = (char*)psz1;
	p2 = (char*)psz2;
	pBS = p1;

	for(;;){
		c1 = *p1;
		c2 = *p2;

		if(c1== _TCHAR('\0') || c2 == _TCHAR('\0')){
			if(c1 == c2 || c1 == _TCHAR('\\') || c2 == _TCHAR('\\')) pBS = p1;
			break;
		}

		if(bKanji){
			if(c1 != c2) break;
			bKanji = false;
		}else{
			if(c1 >= _TCHAR('A') && c1 <= _TCHAR('Z')) c1 = c1 + (_TCHAR('a') - _TCHAR('A'));	// c1����������
			if(c2 >= _TCHAR('A') && c2 <= _TCHAR('Z')) c2 = c2 + (_TCHAR('a') - _TCHAR('A'));	// c2����������
			if(c1 != c2) break;
			if(c1 == _TCHAR('\\')) pBS = p1;
			bKanji = IsKanji(c1);
		}
		p1++;
		p2++;
	}
	return (int)(pBS - psz1);
}

/*
// �w�肵���p�X�𒷂��p�X�ɕϊ�����
// UNC�p�X�̏ꍇ��
// �c��̕����񂪁uC:�v�ƂȂ����Ƃ�FindFirstFile�Ɏ��s����Ƃ����
// �������K�v������
bool GetLongPathName(const char* pszShortPath, char* pszLongPath)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	char szTempShortPath[MAX_PATH];
	char szLongPath[MAX_PATH];
	char szTemp[MAX_PATH];
	LPTSTR pYen;
	int nPos;

	if(!FileExists(pszShortPath)) return false;

	if(IsUNCPath(pszShortPath)){
		// UNC�p�X�̏ꍇ�͂��̂܂�
		return true;
	}else{
		szLongPath[0] = _TCHAR('\0');

		strcpy(szTempShortPath, pszShortPath);

		hFind = FindFirstFile( szTempShortPath, &wfd );
		if (hFind == INVALID_HANDLE_VALUE) return false;
		FindClose(hFind);

		do{
			pYen = strrchrex( szTempShortPath, _TCHAR('\\') );
			if( pYen != NULL ){
				nPos = pYen - szTempShortPath;
				if(strlen(szLongPath) == 0){
					lstrcpy( szLongPath, wfd.cFileName );
				}else{
					wsprintf( szTemp, "%s\\%s", wfd.cFileName, szLongPath );
					lstrcpy( szLongPath, szTemp );
				}
				szTempShortPath[nPos] = _TCHAR('\0');
				hFind = FindFirstFile( szTempShortPath ,&wfd );
				if (hFind == INVALID_HANDLE_VALUE){
					wsprintf( szTemp, "%s\\%s", szTempShortPath, szLongPath );
					lstrcpy( szLongPath, szTemp );
					break;
				}
				FindClose(hFind);
			}else{
				wsprintf( szTemp, "%s\\%s", szTempShortPath, szLongPath );
				lstrcpy( szLongPath, szTemp );
			}
		}while( pYen != NULL );
	}

	strcpy(pszLongPath, szLongPath);

	return true;
}

// �w�肵���p�X�𒷂��p�X�ɕϊ�����
bool GetLongPathName(char* pszPath)
{
	return (GetLongPathName(pszPath, pszPath));
}
*/


// �w�肳�ꂽ�����񂪃t�@�C�����Ƃ��Đ�������
// �i�t�@�C�����Ɏg���Ȃ��������g���Ă��Ȃ����j�`�F�b�N
// �t�@�C���Ɏg���Ȃ�������
// WinXP���ƁuTAB�v�u\�v�u/�v�u:�v�u*�v�u?�v�u"�v�u<�v�u>�v�u|�v
// Win98SE���Ƃ���Ɂu,�v�u;�v�������
// �i�ǂ���烍���O�t�@�C���l�[���ɂ����Ắu,�v���u;�v���g����͗l�j
// ��̕������擪�Ƀs���I�h������ꍇ�s���Ƃ���
bool IsFileName(LPCTSTR pszFileName)
{
	LPTSTR p;
	TCHAR c;

	// ��̕������擪�Ƀs���I�h������ꍇ�s���Ƃ���
	if(*pszFileName == _TCHAR('\0') || *pszFileName == _TCHAR('.')) return false;

	p = const_cast<LPTSTR>(pszFileName);

	for(;;){
		c = *p++;
		if(c == _TCHAR('\0')){
			break;

#ifndef _UNICODE
		}else if(IsKanji(c)){
			p++;
			continue;
#endif
		//}else if(c==_TCHAR('\t') || c==_TCHAR('\\') || c==_TCHAR('/') || c==_TCHAR(':') 
		//	  || c==_TCHAR(',')  || c==_TCHAR(';')  || c==_TCHAR('*') || c==_TCHAR('?')
		//	  || c==_TCHAR('"')  || c==_TCHAR('<')  || c==_TCHAR('>') || c==_TCHAR('|')){
		//	return false;

		}else if(c==_TCHAR('\t') || c==_TCHAR('\\') || c==_TCHAR('/')  || c==_TCHAR(':')  || c==_TCHAR('*') 
			  || c==_TCHAR('?')  || c==_TCHAR('"')  || c==_TCHAR('<')  || c==_TCHAR('>')  || c==_TCHAR('|')){
			return false;
		}
	}
	return true;
}

// �p�X��\�������񂩂�X�y�[�X��������
// �X�y�[�X���������������ꍇ
// ������S�̂��_�u���N�H�[�e�[�V�����}�[�N�ň͂�
void QuotePath(String& Path)
{
	if(Path.IsEmpty()) return;

	// �X�y�[�X���܂ނ�
	if(Path.Find(_TCHAR(' ')) == -1) return;

	Path.Insert(0, _TCHAR('\"'));
	Path += _TCHAR('\"');
}

// �p�X�̍Ō��'\\'�̌��Ԃ�
// '\\'�������ꍇ�͂��̂܂ܕԂ�
LPCTSTR GetFileName(LPCTSTR Path)
{
	LPCTSTR p;
	p = strrchrex(Path, _TCHAR('\\'));
	if(p == NULL){
		return Path;
	}else{
		return p+1;
	}
}

void GetFileName(LPCTSTR path, String& filename)
{
	LPCTSTR p;
	p = strrchrex(path, _TCHAR('\\'));
	if(p == NULL){
		filename.Copy(path);
	}else{
		filename.Copy(p+1);
	}
}

#include <shellapi.h>
#pragma comment(lib,"shell32.lib")

// �t�@�C����t�H���_�̍폜
// ���ݔ��ɑ��邱�Ƃ��ł���
// �ǂݎ���p�̃t�@�C�����폜�ł���
// �t�H���_�͂��̃t�H���_�Ɋ܂܂��t�@�C�����ƍ폜����
// 
bool DeleteFileOrFolder(LPCTSTR filename, bool usesRecycleBin) {

	TCHAR* deletePath = new TCHAR [_tcslen(filename) + 2];

	_tcscpy(deletePath, filename);

	deletePath[_tcslen(deletePath)+1] = '\0';
	SHFILEOPSTRUCT fileOp;
	fileOp.hwnd = NULL;
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = deletePath;
	fileOp.pTo = NULL;
	fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;

	// SHFileOperation() �̏I�������̒l�Œm�邽��
	fileOp.fAnyOperationsAborted = TRUE;

	if (usesRecycleBin) fileOp.fFlags |= FOF_ALLOWUNDO;
	if (SHFileOperation(&fileOp) != 0) {
		// �t�@�C���폜���s
		delete [] deletePath;
		return false;
	}
	
	while (fileOp.fAnyOperationsAborted == TRUE) {
		MessageBox(NULL, _T("SHFileOperation() �̓r���ł�\n\n���̃��b�Z�[�W���o�����Ƃ���҂܂ł��A��������\n�\�t�g�̉��P�Ɍq����܂�"), _T("Notice"), MB_OK);
		Sleep(1);
	}

	delete [] deletePath;

	return true;
}

// �p�X�̔�r
// �ȗ������p�X�ɑΉ����Ă��Ȃ�
// �啶����������\��/�Ȃǂɂ͑Ή�
// ���̊֐����g���ă\�[�g�����
// �t�H���_�̒���ɂ��̃t�H���_�Ɋ܂܂��t�@�C���E�t�H���_�����������ۏ؂����
// �i�������r�̍� '\\' �� '/' �� '\0' �̎��ɏ��������̂Ƃ��Ĉ������߁j
int ComparePath(LPCTSTR path1, LPCTSTR path2) {
	#ifdef _UNICODE
		TCHAR str1[2];
		TCHAR str2[2];
	#else
		TCHAR str1[3];
		TCHAR str2[3];
		str1[2] = _TCHAR('\0');
		str2[2] = _TCHAR('\0');
	#endif

	str1[1] = _TCHAR('\0');
	str2[1] = _TCHAR('\0');

	
	for (int len = 0; ; ) {
		#ifdef _UNICODE
			str1[0] = path1[len];
			str2[0] = path2[len];
			int r = lstrcmpi(str1, str2);
			if (r == 0) {
				if (str1[0] == _TCHAR('\0')) {
					return 0;
				} else {
					len++;
				}
			} else {
				if (str1[0] == _TCHAR('\0') || str2[0] == _TCHAR('\0')) {
					return r;
				}

				// '\\' �� '/' �� '\0' �̎��ɏ��������̂Ƃ��Ĉ���
				if (str1[0] == _TCHAR('\\') || str1[0] == _TCHAR('/')) {
					return -1;
				}
				if (str2[0] == _TCHAR('\\') || str2[0] == _TCHAR('/')) {
					return 1;
				}
				return r;
			}
		#else
			str1[0] = path1[len];
			str2[0] = path2[len];

			bool b1 = IsKanji(str1[0]);
			bool b2 = IsKanji(str2[0]);

			if (b1) str1[1] = path1[len+1];
			if (b2) str2[1] = path2[len+1];

			int r = lstrcmpi(str1, str2);
			if (r != 0) {
				if (str1[0] == _TCHAR('\0') || str2[0] == _TCHAR('\0')) {
					return r;
				}

				// '\\' �� '/' �� '\0' �̎��ɏ��������̂Ƃ��Ĉ���
				if (b1 == false && (str1[0] == _TCHAR('\\') || str1[0] == _TCHAR('/'))) {
					return -1;
				}
				if (b2 == false && (str2[0] == _TCHAR('\\') || str2[0] == _TCHAR('/'))) {
					return 1;
				}
				return r;

			} else if (str1[0] == _TCHAR('\0')) {
				return 0;
			} else {
				if (b1) {
					str1[1] = _TCHAR('\0');
					str2[1] = _TCHAR('\0');
					len+=2;
				} else {
					len++;
				}
			}
		#endif
	}
}


// �G�N�X�v���[���Ɠ����\�[�g���������邽�߂̃p�X��r
// �t�H���_�Ƀt�H���_���̃t�@�C�����������Ƃ��ۏ؂����
// "<" ���Z�q�Ƃ��Ďg��
bool ComparePath(LPCTSTR path1, LPCTSTR path2, bool isFolder1, bool isFolder2)
{
	struct PathAndAttribute {
		LPCTSTR path;
		bool isFolder;
	};

	PathAndAttribute smallOne;
	PathAndAttribute largeOne;

	int r = ComparePath(path1, path2);
	if (r == 0) {
		// path1 �������t�H���_�Ȃ� true
		return (isFolder1 && !isFolder2);
	} else if (r < 0) {
		if (isFolder1) return (r < 0);
		smallOne.path = path1;
		smallOne.isFolder = isFolder1;
		largeOne.path = path2;
		largeOne.isFolder = isFolder2;
	} else {
		if (isFolder2) return (r < 0);
		smallOne.path = path2;
		smallOne.isFolder = isFolder2;
		largeOne.path = path1;
		largeOne.isFolder = isFolder1;
	}

	// ��r�̌��ʏ������̂��t�@�C���ł���΃t�H���_�D��ɂ���s����t�]�����蓾��
	// ��r�̂��ߐe�t�H���_�ɂ͖����� '\\' ���c���Ă���
	String s1 = smallOne.path;
	String p1(s1);
	int n1 = p1.ReverseFind(_TCHAR('\\'));
	int n2 = p1.ReverseFind(_TCHAR('/'));
	p1.MakeLeft(max(n1, n2) + 1);

	String s2 = largeOne.path;
	String p2(s2);
	p2.MakeLeft(p1.GetLength());
	// ��������ɂ���Ă͂Q�o�C�g�����̂P�o�C�g�ڂŐ؂邱�ƂɂȂ�̂��C�����������c

	if (lstrcmpi(p1.c_str(), p2.c_str()) == 0) {
		if ((largeOne.isFolder)
			|| s2.ReverseFind(_TCHAR('\\')) > p2.GetLength() 
			|| s2.ReverseFind(_TCHAR('/')) > p2.GetLength()) 
		{
			return !(r < 0);
		}
	}

	return (r < 0);
}


// �t�@�C�����������łȂ��t�@�C�����ɕς���
// �g���q��ۂ��ăt�@�C�����̏I���Ɂu(1)�v����t����
bool EvacuateFileName(String& rFilename) {
	if (!FileExists(rFilename.c_str())) return true;

	String path;
	String extention;

	GetExtention(rFilename.c_str(), extention);
	path = rFilename;
	path.MakeLeft(path.GetLength() - extention.GetLength());

	for (int i=2; ; ++i) {
		rFilename.Format(_T("%s(%d)%s"), path.c_str(), i, extention.c_str());
		if (!FileExists(rFilename.c_str())) {
			break;
		}
	}
	return true;
}

// �t�H���_���������łȂ��t�H���_���ɕς���
bool EvacuateFolderName(String& folderName) {
	if (!FileExists(folderName.c_str())) return true;
	String path;
	int i=1;
	do {
		path.Format(_T("%s(%d)"), folderName.c_str(), i);
		i++;
	} while(FileExists(path.c_str()));
	folderName = path;
	return true;
}

// �p�X�̐[���𓾂�
// �󕶎��񂾂Ɛ[���� 0
// ��̃t�H���_���Ȃ���ΐ[���� 1
// '\\' ������Ԃ悤�ȃp�X�ɂ͑Ή����Ă��Ȃ�
int PathGetDepth(LPCTSTR path) 
{
	if (path == NULL || *path == '\0') {
		return 0;
	}

	// �Z�p���[�^�𐔂���
	// �����������̃Z�p���[�^�͖���
	String str(path);
	int depth = 1;
	for (;;) {
		int pos = str.FindOneOf(_T("\\/"));
		if (pos == -1 || pos == str.GetLength()-1) break;
		depth++;
		str.MakeMid(pos+1);
	}
	return depth;
}

// �w��̒i���p�X�����
// ������i����Ԃ�
int StripPath(String& path, int n)
{
	if (n <= 0) return 0;
	int d = PathGetDepth(path.c_str());

	// �S�����̂�
	if (n >= d) {
		path.Empty();
		return d;
	}

	// �����ڂ̃Z�p���[�^�܂Ŏc���΂����̂�
	int dd = d-n;
	int len = 0;

	String str(path);
	for (int i=0; i<dd; ++i) {
		int pos = str.FindOneOf(_T("\\/"));
		len += (pos+1);
		str.MakeMid(pos+1);
	}

	if (IsFullPath(path.c_str()) && dd==1) {
		path.MakeLeft(len);
	} else {
		path.MakeLeft(len-1);
	}

	return n;
}

int GetCommonPrefix(LPCTSTR path1, LPCTSTR path2, String& commonPrefix) 
{
	int n = ::PathCommonPrefix(path1, path2, NULL);
	if (n == 0) {
		commonPrefix.Empty();
		return 0;
	}
	LPTSTR p = commonPrefix.GetBuffer(n);
	n = ::PathCommonPrefix(path1, path2, p);
	commonPrefix.ReleaseBuffer();
	return n;
}



// �p�X������� "\\..\\" ��W�J����
// �p�X�̋�؂蕶���� '\\'
// ��΃p�X�ł����Ă����΃p�X�ł����Ă�
// �w�肵��������̊K�w�ɂ͋삯�オ��Ȃ�
// '.' �݂̂ō\�����ꂽ�G�������g�̌�ɕ����� '\\' ���A�����Ă��Ă�
// ��̂ݎ�菜�����
bool expandPath(LPCTSTR srcPath, String& dstPath) {
	String src = srcPath;

	if (src.Find(_T(".\\")) == -1) {
		dstPath = srcPath;
		return true;
	}

	vector<boost::shared_ptr<String> > elements;

	while (!src.IsEmpty()) {
		boost::shared_ptr<String> pDst(new String());
		int delimiterPos = src.Find(_TCHAR('\\'), 0);
		if (delimiterPos != -1) {
			pDst->NCopy(src.c_str(), delimiterPos);
			src.MakeMid(delimiterPos + 1);
		} else {
			pDst->Copy(src.c_str());
			src.Empty();
		}
		int dot = 0;
		for (int i = 0; i < pDst->GetLength(); ++i) {
			TCHAR c = pDst->GetAt(i);
			if (c == _TCHAR('.')) {
				dot++;
			} else {
				dot = 0;
				break;
			}
		}
		if (dot == 0) {
			elements.push_back(pDst);
		} else {
			while (dot >= 2) {
				if (elements.size() != 0) {
					if (!elements.back()->IsEmpty()) {
						dot--;
					}
					elements.pop_back();
				} else {
					// pop ����v�f���Ȃ���΃G���[���o��
					return false;
				}
			}
		}
	}

	// elements ��S�Č������Ċ���
	dstPath.Empty();
	vector<boost::shared_ptr<String> >::iterator it;
	for (it = elements.begin(); it != elements.end(); ++it) {
		dstPath += *(*it);
		if (it + 1 != elements.end()) dstPath += _TCHAR('\\');
	}

	return true;
}


bool FileEnumerator::enumerate(
	LPCTSTR path, 
	LPCTSTR pattern, 
	bool searchesSubfolders,
	vector<String>& v) {

	v.clear();
	return enumerateSub(path, pattern, searchesSubfolders, v);
}

bool FileEnumerator::enumerateSub(
	LPCTSTR path, 
	LPCTSTR pattern, 
	bool searchesSubfolders, 
	vector<String>& v) {

	String p;
	WIN32_FIND_DATA fd;
	HANDLE hSearch;

	// �T���p�X���쐬
	p = path;
	CatPath(p, pattern);

	hSearch = FindFirstFile(p.c_str(), &fd);

	if (hSearch == INVALID_HANDLE_VALUE) {
		return false;
	}

	do{
		// �f�B���N�g�����A���ʂ̃t�@�C����
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			// �f�B���N�g���ł�����

			// ���[�g�y�уJ�����g�łȂ��t�H���_���ǂ����`�F�b�N
			if (lstrcmp(fd.cFileName,_T(".")) == 0 || lstrcmp(fd.cFileName, _T("..")) == 0) {
				 continue;
			}

			p = path;
			CatPath(p, fd.cFileName);
			v.push_back(p);

			if (searchesSubfolders) {
				if (!enumerateSub(p.c_str(), pattern, true, v)) {
					FindClose(hSearch);
					return false;
				}
			}
		} else {
			// ���ʂ̃t�@�C���ł�����
			p = path;
			CatPath(p, fd.cFileName);
			v.push_back(p);
		}
	} while (FindNextFile(hSearch,&fd) != 0);

	FindClose(hSearch);

	return (GetLastError() == ERROR_NO_MORE_FILES);
}


} // end of namespace KSDK

