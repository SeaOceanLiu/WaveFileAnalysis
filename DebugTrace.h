//---------------------------------------------------------------------------

#ifndef DebugTraceH
#define DebugTraceH

#include <stdio.h>
#include <iostream>
#include <string>
#include <windows.h>
//#include "..\debug\debugformunit.h"
#define MAX_SYS_ERROR_MSG_LEN 500
#define PRINT_OUT_LAST_ERROR { char ErrorInfo[MAX_SYS_ERROR_MSG_LEN];           \
                               LPVOID   lpMsgBuf;                               \
                               FormatMessage(                                   \
                                   FORMAT_MESSAGE_ALLOCATE_BUFFER               \
                                       | FORMAT_MESSAGE_FROM_SYSTEM,            \
                                   NULL, GetLastError(),                        \
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   \
                                   (LPTSTR) &lpMsgBuf, 0, NULL);                \
                               sprintf_s(ErrorInfo, MAX_SYS_ERROR_MSG_LEN, "Error at File:%s, Line:%d, %s",__FILE__, __LINE__, (char *)lpMsgBuf);\
                               MessageBox( NULL, ( LPCTSTR)ErrorInfo,           \
								   TEXT("GetLastError"), MB_OK|MB_ICONINFORMATION );     \
                               LocalFree( lpMsgBuf );}
//---------------------------------------------------------------------------
/*
_CreateDebugForm
_DebugInfoForm
_DebugPrint
_DestoryDebugForm
_HideDebugForm
_ShowDebugForm
_SetColorSize
_GetColorSize
_OutPutMem*/

// extern DebugTrace *pDTInst;
#define DEBUG_PRINT(fmt, ...) DebugTrace::getInstance()->m_pfDebugPrint((fmt), ##__VA_ARGS__)
#define DEBUG_STRING(str) DebugTrace::getInstance()->m_pfDebugPrint((string(str)).c_str())

typedef void (*pFunc)(void);
typedef HWND (*pCreateFunc)(const char *pMainFrameTitle);
typedef void (*pDebugPrintFunc)(const char *Format, ...);
typedef void (*pSetColorSize)(COLORREF FColor, int FSize);
typedef void (*pGetColorSize)(COLORREF *FColor, int *FSize);
typedef void (*pOutPutMem)(void *pMemPointer, unsigned int Length);
typedef void (*pPrintToBufferFunc)(bool StyleFlag, COLORREF FColor, int FSize, char *Format, ...);
typedef void (*pSetPrintSpeed)(int Interval);
typedef void (*pOutPutMemToMem)(bool StyleFlag, COLORREF FColor, int FSize, void *pMemPointer, unsigned int Length);
typedef void (*pStartSaveToFile)(char *pFileName);

enum RUNNING_BIT_SIZE
{
	RBS_x86,
	RBS_x64,

	RBS_BUTT
};

class DebugTrace
{
private:
	HINSTANCE   hDebugDllInst;
	HWND 		m_hDebugForm;

	enum RUNNING_BIT_SIZE getRunningBitSize();
	DebugTrace();
    bool Initial(const char *pTitle);
public:
	pCreateFunc m_pfCreateDebugForm;
	pFunc m_pfDestoryDebugForm;
	pFunc m_pfCloseDebugForm;
	pFunc m_pfShowDebugForm;
	pFunc m_pfHideDebugForm;
	pDebugPrintFunc m_pfDebugPrint;

	pSetColorSize m_pfSetColorSize;
	pGetColorSize m_pfGetColorSize;
	pOutPutMem m_pfOutPutMem;

	pPrintToBufferFunc m_pPrintToBuffer;
	pSetPrintSpeed m_pfSetPrintSpeed;
	pOutPutMemToMem m_pfOutPutMemToBuffer;
	pStartSaveToFile m_pfStartSaveToFile;
    pFunc m_pfStopSaveToFile;

    ~DebugTrace();

    static DebugTrace* getInstance(void){
        static DebugTrace instance; // 静态局部变量，程序运行期间只会被初始化一次
        return &instance;
    }

};
#endif
