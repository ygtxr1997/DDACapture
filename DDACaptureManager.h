#pragma once

#include "CommonTypes.h"

//
// DDA 捕获工具
// 支持: 设置FPS、时间戳、鼠标当前位置、是否绘制鼠标
// 功能: 可以通过 GetFrameData(_Out_) 获取一帧 FrameData 类型的数据	
class DDACAPTUREMANAGER
{
public:
	DDACAPTUREMANAGER();
	~DDACAPTUREMANAGER();

	CA_RETURN Init();	// 初始化必备资源
	CA_RETURN GetOneFrame(_In_ CAPTURE_MODE CaptureMode);	// 获取一帧
	void GetFrameData(_Out_ CAPTURE_FRAMEDATA* pFrameData);			// 得到当前 FrameData 类型的数据
	void SetCaptureSetting(_In_ CAPTURE_SETTING* pCaptureSetting);	// 设置

	void Clean(); // 清空

private:
	// D3D 和 DXGI 资源
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	ID3D11Texture2D* m_GDIImage;
	D3D_FEATURE_LEVEL	m_FeatureLevel;
	DXGI_OUTPUT_DESC m_OutputDesc;
	IDXGIOutputDuplication* m_DeskDupl;
	DXGI_OUTDUPL_DESC m_OutDuplDesc;
	ID3D11Texture2D* m_CPUImage;

	IDXGIResource* m_DesktopResource;
	DXGI_OUTDUPL_FRAME_INFO m_FrameInfo;
	ID3D11Texture2D* m_AcquiredDesktopImage;
	CURSORINFO m_CursorInfo;

	D3D11_MAPPED_SUBRESOURCE m_MappedRect;

	// 声明在 CommonTypes 中的类型
	CAPTURE_FRAMEDATA* m_FrameData;
	CAPTURE_SETTING* m_CaptureSetting;

	// 方法
	CA_RETURN ProcessWindow(); // 根据窗口矩形处理 bitmap
};

