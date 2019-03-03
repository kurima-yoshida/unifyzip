// Zip�t�@�C������
// ���݂͖����k�̂ݑΉ�
// USE_ZIP_DETAIL_INFO ���`����� local file �̏ڍ׏�񂪓�����
#ifndef __ZIP_H__
#define __ZIP_H__

#include <vector>
#include <list>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "MemoryMappedFile.h"
#include "String.h"

namespace KSDK {

bool Compress(LPCTSTR srcPath, LPCTSTR dstPath, int compressLevel);
int Decompress(LPCTSTR zipFileName, LPCTSTR destPath);

struct LocalFileInfo {
	String fileName;	// ���O
	UINT32 fileDataPos;	// �A�[�J�C�u���ł̃t�@�C���f�[�^�̈ʒu
	UINT32 fileSize;	// �T�C�Y

	UINT32 externalFileAttributes;

//	#ifdef USE_ZIP_DETAIL_INFO
		UINT32 uncompressedSize;
		FILETIME fileTime;
		UINT16 versionMadeBy;
		UINT16 versionNeededToExtract;
		UINT16 generalPurposeBitFlag;
		UINT16 compressionMethod;
		UINT16 internalFileAttributes;
		std::vector<UINT8> comment;
//	#endif
};


// Zip���̌X�̃t�@�C����ǂވׂ̃N���X
class LocalFile : boost::noncopyable {
public:
	LocalFile(ReadOnlyMemoryMappedFileEx* pMemoryMappedFile, LocalFileInfo* pInfo) {
		pMemoryMappedFile_ = pMemoryMappedFile;
		pInfo_ = pInfo;
		pFileData_ = NULL;
		p_ = NULL;
	}

	int read(void* pBuffer, UINT32 len);
	bool seek(int offset, int origin);
	UINT32 tell() {
		return (UINT32)(p_ - pFileData_);
	}
	LPCTSTR getFileName() {
		return pInfo_->fileName.c_str();
	}
	UINT32 getFileSize() {
		return pInfo_->fileSize;
	}

	UINT32 getExternalFileAttributes() {
		return pInfo_->externalFileAttributes;
	}

	// Zip �t�@�C���̉����Ƀt�@�C���f�[�^������̂�
	UINT32 getFileOffset() {
		return pInfo_->fileDataPos;
	}


//#ifdef USE_ZIP_DETAIL_INFO
	UINT32 getUncompressedSize() {
		return pInfo_->uncompressedSize;
	}
	FILETIME getFileTime() {
		return pInfo_->fileTime;
	}
	UINT16 getVersionMadeBy() {
		return pInfo_->versionMadeBy;
	}
	UINT16 getVersionNeededToExtract() {
		return pInfo_->versionNeededToExtract;
	}
	UINT16 getGeneralPurposeBitFlag() {
		return pInfo_->generalPurposeBitFlag;
	}
	UINT16 getCompressionMethod() {
		return pInfo_->compressionMethod;
	}
	UINT16 getInternalFileAttributes() {
		return pInfo_->internalFileAttributes;
	}

	UINT16 obtainInternalFileAttributes();

	UINT16 getCommentLength() {
		return (UINT16)(pInfo_->comment.size());
	}

	void getComment(void* pComment) {
		memcpy(pComment, &(*(pInfo_->comment.begin())), getCommentLength());
	}
//#endif

	// �|�C���^�͎擾�ł��Ȃ������������ۂ����c
	// �o�b�t�@�p�ӂ��Ă�����read()�����đ�p���ׂ�����
	// �����k�łȂ��ƃ|�C���^�[�擾���Ă����k�ς݃f�[�^�Q�Ƃ��邱�ƂɂȂ邵
	// LocalFile �P�̃T�C�Y���}�b�s���O����
	void* getPointer() {
		if (!pView_) {
			pView_ = pMemoryMappedFile_->map(pInfo_->fileDataPos, pInfo_->fileSize);
			if (!pView_) return NULL;
			pFileData_ = (UINT8*)(pView_->getPointer());
			p_ = pFileData_;
		}
		return p_;
	}

private:
	LocalFile();
	ReadOnlyMemoryMappedFileEx* pMemoryMappedFile_;
	LocalFileInfo* pInfo_;
	UINT8* pFileData_;
	UINT8* p_;
	boost::shared_ptr<ViewOfFile> pView_;

	// �|�C���^�擾�̂��߃������}�b�v���ꂽ����
	UINT32 mappedSize_;
};


class ZipReader {
public:
	~ZipReader() {
		close();
	}

	// �A�[�J�C�u�t�@�C���I�[�v��
	// �����t�@�C���̏��擾
	bool open(LPCTSTR fileName);
	void close();

	// �����t�@�C���ւ̃A�N�Z�b�T�𓾂�
	boost::shared_ptr<LocalFile> getLocalFile(LPCTSTR fileName);

	boost::shared_ptr<LocalFile> getLocalFile(int index);

	int getLocalFileNumber() {
		return (int)fileInfo_.size();
	}

	LPCTSTR getName() {
		return name_.c_str();
	}
private:
	String name_;
	std::vector<LocalFileInfo> fileInfo_;
	ReadOnlyMemoryMappedFileEx mmf_;
};

// singleton
class ZipReaderManager {
public:
	static ZipReaderManager& instance();
	ZipReader* getZipReader(LPCTSTR zipFileName);
private:
	ZipReaderManager() {}
	ZipReaderManager(const ZipReaderManager&);
	const ZipReaderManager& operator=(const ZipReaderManager&);

	// ���ɂ̐��͂��������Ȃ����낤���珑�ɂ̌��������`�T���ŏ\��
	std::vector< boost::shared_ptr<ZipReader> > archives_;
};

class ZipWriter {
public:
	struct LocalFileData {
		String dstFileName;
		String srcFileName;
		DWORD fileAttributes;
		FILETIME lastWriteTime;

		// �t�@�C���T�C�Y�̓t�H���_�̏ꍇ 0 �ɂ��邱��
		UINT32 fileSize;
	};

	ZipWriter();
	~ZipWriter();

	bool open(LPCTSTR zipFileName) {
		zipFileName_ = zipFileName;
		isOpen_ = true;
		return true;
	}

	int close();

	void add(LPCTSTR dstFileName, LPCTSTR srcFileName);

	bool isOpen() {
		return isOpen_;
	}

	// -1: �f�t�H���g
	//  0: �����k
	//  1: ����
	//  9: �����k
	bool setCompressLevel(int compressLevel);

	bool addLackFolder();

	UINT16 getInternalFileAttributes(LPCTSTR fileName, UINT32 fileAttributes, UINT32 fileSize);

private:
	static const int WRITEBUFFERSIZE = 16384;

	int closeSub();
	UINT32 getCRC32(LPCTSTR fileName);

	bool correctLocalFileData(LocalFileData& localFileData);

	bool isOpen_;
	int compressLevel_;

	String zipFileName_;

	std::list<LocalFileData> localFileData_;
};

} // end of namespace KSDK

#endif //__ZIP_H__
