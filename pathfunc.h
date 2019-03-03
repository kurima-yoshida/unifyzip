/*------------------------------------------------------------------------------
	File:	PathFunc.h
												2003.06.11 (c)YOSHIDA Kurima
===============================================================================
 Content:
	�p�X������Ɋւ���֐��Q

 Notes:
	���Q�o�C�g�����̃A���t�@�x�b�g�̑啶���E�������̈Ⴂ�������Ă�
	�����p�X�Ƃ��Ĉ�����悤�i�e�X�g���Ċm�F����K�v����j�Ȃ̂�
	����ɑΉ����Ȃ���΂Ȃ�Ȃ�

	���l�b�g���[�N�p�X�ɑΉ�����K�v������

	�����̃t�@�C���̊֐��Q�̓p�X�̋�؂蕶�����u\�v�݂̂Ɖ��肵�đg��ł���

	����{���j�Ƃ��ē��ɒf���Ă��Ȃ�����
	�����Ƃ��Đ������`���̃p�X���n�����Ɖ��肵��O�����͍s���Ă��Ȃ�
	�󕶎���͐������p�X�̂ЂƂƂ���

	�t���p�X�̕\����
	�����[�g�h���C�u�̂Ƃ��̂ݖ�����\����

	c:\
	c:\data
	c:\data\sub
	c:\data\sub\a.txt
	c:\a.txt


	���΃p�X�̕\����
	���擪��\�͂��Ȃ�

	data
	data\a.txt
	data\sub\a.txt


	�l�b�g���[�N�t�H���_
	\\Prius\�V�����t�H��

	���Ƃ��AC�h���C�u�����L���Ă��ȉ��̂悤�ɂȂ�
	\\Prius\c

 Sample:

------------------------------------------------------------------------------*/

#ifndef __PATHFUNC_H__
#define __PATHFUNC_H__

#include "types.h"
#include <vector>

namespace KSDK {

class String;

// �p�X�̎��
enum PathAttribute {
	PATH_INVALID,	// ���݂��Ȃ��p�X
	PATH_FILE,		// �t�@�C���̃p�X
	PATH_FOLDER,	// �t�H���_�̃p�X
};

// �w��̃t�@�C�������t���p�X�ɂ��ĕԂ�
// �t�@�C���̎w��͑��΃p�X�A�t���p�X�A�g���q����A�g���q�����̂��������
// pszDefPath�͋�̕������NULL�w��������pszFile�݂̂Ŏw�肷�邱�ƂɂȂ�
//char* GetFullPath(const char* pszFile, const char* pszDefPath, const char* pszDefExt, char* pszPath);

// ���΃p�X�ɃJ�����g�f�B���N�g�������ăt���p�X�ɂ���
bool GetFullPath(String& Path);

// UNC (Universal Naming Convention) �p�X ��
// �u\\�v�Ŏn�܂��Ă��邩
bool IsUNCPath(LPCTSTR pszPath);

// �w��̃p�X���t���p�X���ǂ���
// �u:�v��2�����ڂɂ��邩
bool IsFullPath(LPCTSTR pszPath);

// �w��̃p�X���t���p�X���ǂ����iUNC���t���p�X�ƈ����j
bool IsFullPathEx(LPCTSTR pszPath);

// �w��̃p�X���t�@�C���̃p�X���i�g���q�����邩�j
//bool IsFilePath(LPCTSTR pszPath);

// �w��̃t�@�C�������݂��邩
bool FileExists(LPCTSTR pszPath);

// �擪�̃X�y�[�X �����̃X�y�[�X�ƃs���I�h ���폜
void FixFileName(String& FileName);

// �������`���ɂ���
// �i�ŏ��Ɂu\�v�����������菜���j
// �i������2�ȏ�u\�v�������ꍇ�̓l�b�g���[�N�p�X�ƍl��2�c���j
// �i�Ō�Ɂu\�v�����������菜�� ������ �uc:\�v�̗l�Ƀ��[�g�̏ꍇ�͂����Ȃ�Ȃ��j
void FixPath(String& Path);

// �T�C�Y�����Ȃ��Ō��ʂ� String �ɕԂ� GetCurrentDirectory()
void GetCurrentDirectory(String& string);

// MAX_PATH�𒴂��钷���̃p�X��
// ���C�h���� (W) �o�[�W������ API �֐��ɓn���Ƃ���
// �t�H�[�}�b�g�ɕϊ�����
void FormatTooLongPath(String& String);

// �w��̃p�X���t�H���_���t�@�C������Ԃ�
PathAttribute GetPathAttribute(LPCTSTR pszPath);

// �f�B���N�g���̍쐬�i�����K�w�̃f�B���N�g�����쐬�\�j
// �t���p�X�̂ݑΉ�
// MakeSureDirectoryPathExists() ���g���΂������ۂ��H
bool CreateDirectoryEx(LPCTSTR path);

// �p�X����t�@�C��������菜��
// �L���ȃp�X�ł��邩�͊m���߂Ȃ�
bool RemoveFileName(String& path);

// �p�X����t�@�C��������菜��
// �L���ȁi���݂���j�p�X�ɂ��Ă̂ݗL��
bool RemoveFileNameEx(String& path);

// �p�X����g���q����菜��
// �L���ȃp�X�ł��邩�͊m���߂Ȃ�
bool RemoveExtension(String& path);

// �t�@�C��������g���q�𓾂�
void GetExtention(LPCTSTR filename, String& ext);

// �J�����g�f�B���N�g���ݒ�
// ��΃p�X�ł��A���΃p�X�i�󕶎���ł��j�ł��w��\
// "\\data" �Ƃ� "C:\\data" �Ƃ������悤�Ɏw�肷��
// ��΃p�X�łȂ��ꍇexe�t�@�C�����܂ރt�H���_����̑��΃p�X�Ƃ݂Ȃ�
bool SetCurrentDirectoryEx(LPCTSTR pszDir);

// �p�X�̒ǉ�
//void CatPath(char* pszPath1, const char* pszPath2);
void CatPath(String& String, LPCTSTR pszPath);

// �t�H���_���󂩂ǂ������ׂ�
bool IsEmptyFolder(LPCTSTR pszPath);

// 2�̃p�X�̐擪���狤�ʂ���p�X�̒�����Ԃ�
int PathCommonPrefixLen(const char* psz1, const char* psz2);

// �w�肵���p�X�𒷂��p�X�ɕϊ�����
//bool GetLongPathName(char* pszPath);

// �w�肳�ꂽ�����񂪃t�@�C�����Ƃ��Đ�������
// �i�t�@�C�����Ɏg���Ȃ��������g���Ă��Ȃ����j�`�F�b�N
// �t�@�C���Ɏg���Ȃ�������
// WinXP���ƁuTAB�v�u\�v�u/�v�u:�v�u*�v�u?�v�u"�v�u<�v�u>�v�u|�v
// Win98SE���Ƃ���Ɂu,�v�u;�v�������
// �i�ǂ���烍���O�t�@�C���l�[���ɂ����Ắu,�v���u;�v���g����͗l�j
// ��̕������擪�Ƀs���I�h������ꍇ�s���Ƃ���
bool IsFileName(LPCTSTR pszFileName);

// �p�X��\�������񂩂�X�y�[�X��������
// �X�y�[�X���������������ꍇ
// ������S�̂��_�u���N�H�[�e�[�V�����}�[�N�ň͂�
void QuotePath(String& Path);

// �T�C�Y�����Ȃ��Ō��ʂ� String �ɕԂ� GetModuleFileName()
void GetModuleFileName(HMODULE hModule, String& filename);

// �t���p�X����t�@�C�����𓾂�
void GetFileName(LPCTSTR path, String& filename);

LPCTSTR GetFileName(LPCTSTR Path);

// �t�@�C����t�H���_�̍폜
// ���ݔ��ɑ��邱�Ƃ��ł���
// �t�H���_�͂��̃t�H���_�Ɋ܂܂��t�@�C�����ƍ폜����
bool DeleteFileOrFolder(LPCTSTR filename, bool usesRecycleBin = false);

// �s���S�Ȏ���
// �ȗ����ꂽ�p�X��삯�オ��p�X�ɂ��Ή�����K�v������
int ComparePath(LPCTSTR path1, LPCTSTR path2);

// �t�@�C�����������łȂ��t�@�C�����ɕς���
// �g���q��ۂ��ăt�@�C�����̏I���Ɂu(1)�v����t����
bool EvacuateFileName(String& rFilename);

// �t�H���_���������łȂ��t�H���_���ɕς���
bool EvacuateFolderName(String& folderName);

int PathGetDepth(LPCTSTR path);

int StripPath(String& path, int n=1);

int GetCommonPrefix(LPCTSTR path1, LPCTSTR path2, String& commonPrefix);

bool ComparePath(LPCTSTR path1, LPCTSTR path2, bool isFolder1, bool isFolder2);

bool expandPath(LPCTSTR srcPath, String& dstPath);

class FileEnumerator {
public:
	// path �̓t���p�X�̂ݑΉ�
	// pattern �� FindFirstFile() �ւ̈������l
	// �u*�v�܂��́u?�v�̃��C���h�J�[�h�������܂߂邱�Ƃ��ł���
	// TODO ���[�g�f�B���N�g���̌����ɑΉ����Ă��Ȃ� �ȉ����Q�l�ɂ��邱��
	// http://msdn.microsoft.com/library/ja/default.asp?url=/library/ja/jpfileio/html/_win32_findfirstfile.asp
	// TODO �u*�v�݂̂łȂ��p�^�[�����w�肵���ꍇ�T�u�t�H���_�ȉ��̌������ł��Ȃ�
	static bool enumerate(
		LPCTSTR path, 
		LPCTSTR pattern, 
		bool searchesSubfolders,
		std::vector<String>& v);
private:
	static bool enumerateSub(
		LPCTSTR path, LPCTSTR pattern, 
		bool searchesSubfolders, 
		std::vector<String>& v);
};

} // end of namespace KSDK

#endif //__PATHFUNC_H__

