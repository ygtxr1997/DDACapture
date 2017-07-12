#pragma once

#include "CommonTypes.h"

//
// DDA ���񹤾�
// ֧��: ����FPS��ʱ�������굱ǰλ�á��Ƿ�������
// ����: ����ͨ�� GetFrameData(_Out_) ��ȡһ֡ FrameData ���͵�����	
class DDACAPTUREMANAGER
{
public:
	DDACAPTUREMANAGER();
	~DDACAPTUREMANAGER();

	CA_RETURN Init();	// ��ʼ���ر���Դ
	CA_RETURN GetOneFrame(_In_ CAPTURE_MODE CaptureMode);	// ��ȡһ֡
	void GetFrameData(_Out_ CAPTURE_FRAMEDATA* pFrameData);			// �õ���ǰ FrameData ���͵�����
	void SetCaptureSetting(_In_ CAPTURE_SETTING* pCaptureSetting);	// ����

	void Clean(); // ���

private:
	// D3D �� DXGI ��Դ
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

	// ������ CommonTypes �е�����
	CAPTURE_FRAMEDATA* m_FrameData;
	CAPTURE_SETTING* m_CaptureSetting;

	// ����
	CA_RETURN ProcessWindow(); // ���ݴ��ھ��δ��� bitmap
};

