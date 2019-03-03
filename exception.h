#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "String.h"
#include <tchar.h>

namespace KSDK {

// ���ׂĂ̗�O�͂��ꂩ��h��������
class Exception {
	//Exception() {}
	virtual String getError() const { return _T(""); };
};

class RuntimeException : public Exception {
public:
	RuntimeException(const String& errorName=_T("")) : string_(errorName) {
	}
	virtual String getError() const {
		return string_;
	}
protected:
	String string_;
};

} // end of namespace KSDK

#endif //__EXCEPTION_H__