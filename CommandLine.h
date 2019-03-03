#ifndef __COMMANDLINE_H__
#define __COMMANDLINE_H__

#include <windows.h>
#include "String.h"

namespace KSDK {

// �R�}���h���C���֌W�̊֐��Q
void* AllocCommandLine(void);
void FreeCommandLine(void* pvCommandLine);
UINT GetStringFromCommandLine(void* pvCommandLine, UINT iParam, String& Param);

} // end of namespace KSDK

#endif //__COMMANDLINE_H__