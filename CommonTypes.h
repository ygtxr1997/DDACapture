#include <Windows.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <memory.h>
#include <time.h>
#include <sal.h>
#include <iostream>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

//
// 异常处理
//
enum CA_RETURN
{
	CA_RETURN_SUCCESS = 0,
	CA_RETURN_ERROR_EXPECTED = 1,
	CA_RETURN_ERROR_UNEXPECTED = 2
};

//
// 捕获模式
// 概述 :	主要分为 DDA 和 BBT 两种方法
typedef enum CAPTURE_MODE
{
	CAPTURE_MODE_STOP = 0,
	CAPTURE_MODE_UNKNOWN_FULLSCREEN,
	CAPTURE_MODE_UNKNOWN_WIN,
	CAPTURE_MODE_BBT_FULLSCREEN,
	CAPTURE_MODE_BBT_WINHANDLE,
	CAPTURE_MODE_BBT_WINRECT,
	CAPTURE_MODE_DDA_FULLSCREEN,
	CAPTURE_MODE_DDA_WINHANDLE,
	CAPTURE_MODE_DDA_WINRECT
} _CAPTURE_MODE;

//
// 帧信息
// 包括 :	位图缓冲区首地址、位图缓冲区大小、BytesPerLine、鼠标位置、时间戳
typedef struct CAPTURE_FRAMEDATA
{
	BYTE*		pData = nullptr;
	UINT		uSize = 0;
	UINT		BytesPerLine;
	POINT*		CursorPos = new POINT;
	UINT64		TimeStamp = 0;
} _CAPTURE_FRAMEDATA;

//
// 捕获设置  
// 包括 :	时间戳、FPS、WinHandle、IsFollowCursor、目标矩形、是否显示鼠标
typedef struct CAPTURE_SETTING
{
	UINT64	TimeStamp;
	UINT64	OffsetTimeStamp = 0;
	UINT		FPS = 0;
	HWND		WinHandle;
	RECT		TargetRect;
	POINT		Anchor;
	bool			IsDisplay;
	bool			IsFollowCursor;
} _WINCAPTURE_SETTING;