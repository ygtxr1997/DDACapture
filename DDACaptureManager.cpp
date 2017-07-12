#include "DDACaptureManager.h"

#include <string>

using namespace::std;

//
// ��ʼ���б���
//
DDACAPTUREMANAGER::DDACAPTUREMANAGER() :
m_Device(nullptr),
m_DeviceContext(nullptr),
m_GDIImage(nullptr),
m_FeatureLevel(),
m_OutputDesc(),
m_DeskDupl(nullptr),
m_OutDuplDesc(),
m_CPUImage(nullptr),
m_DesktopResource(nullptr),
m_FrameInfo(),
m_AcquiredDesktopImage(nullptr),
m_CursorInfo(),
m_FrameData(new CAPTURE_FRAMEDATA),
m_CaptureSetting(new CAPTURE_SETTING)
{

}

//
// ����
//
DDACAPTUREMANAGER::~DDACAPTUREMANAGER()
{
	Clean();
}

//
// ��ʼ�� D3D ��Դ
//
CA_RETURN DDACAPTUREMANAGER::Init()
{
	CA_RETURN Ret = CA_RETURN_SUCCESS;
	HRESULT hr = S_FALSE;

	// D3D_DRIVER_TYPE
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// D3D_FEATURE_LEVEL
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	// ���� D3D �豸
	D3D_FEATURE_LEVEL FeatureLevel;

	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &m_Device, &FeatureLevel, &m_DeviceContext);

		if (SUCCEEDED(hr))
		{
			// �Ѿ��������ʵ��豸
			m_Device->AddRef();
			m_DeviceContext->AddRef();
			break;
		}

		m_Device->Release();
		m_Device = nullptr;
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	return CA_RETURN_SUCCESS;
}

//
// ��ȡһ֡
//
CA_RETURN DDACAPTUREMANAGER::GetOneFrame(_In_ CAPTURE_MODE CaptureMode)
{
	HRESULT hr;

	if (!m_Device)
	{
		OutputDebugString("�豸�쳣\n");
	}

	clock_t t1, t2, t3, t4;
	clock_t T[20];
	t3 = clock();

	UINT test;

	// ��ȡ��һ֡
	UINT uTryCount = 4;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	// ��ȡ DXGI �豸
	IDXGIDevice* DXGIDevice = nullptr;
	hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DXGIDevice));

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	// ��ȡ DXGI ������
	IDXGIAdapter* DXGIAdapter = nullptr;
	hr = DXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DXGIAdapter));

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	DXGIDevice->Release();
	DXGIDevice = nullptr;

	// ��ȡ ���
	UINT Output = 0;

	IDXGIOutput* DXGIOutput = nullptr;
	hr = DXGIAdapter->EnumOutputs(Output, &DXGIOutput);

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	DXGIAdapter->Release();
	DXGIAdapter = nullptr;

	// ��ȡ�������
	hr = DXGIOutput->GetDesc(&m_OutputDesc);

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	// ��ȡ Output1 �汾�����
	IDXGIOutput1* DXGIOutput1 = nullptr;

	hr = DXGIOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&DXGIOutput1));

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	DXGIOutput->Release();
	DXGIOutput = nullptr;

	// �������渴����
	// NOTE: DuplicateOutput����
	if (m_DeskDupl)
	{
		m_DeskDupl->Release();
		m_DeskDupl = nullptr;
	}
	hr = DXGIOutput1->DuplicateOutput(m_Device, &m_DeskDupl);

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	DXGIOutput1->Release();
	DXGIOutput1 = nullptr;

	// ������������ (ͨ��DESC)
	m_DeskDupl->GetDesc(&m_OutDuplDesc);

	// ����GDI����
	D3D11_TEXTURE2D_DESC TextureDesc;
	TextureDesc.Width = m_OutDuplDesc.ModeDesc.Width;
	TextureDesc.Height = m_OutDuplDesc.ModeDesc.Height;
	TextureDesc.Format = m_OutDuplDesc.ModeDesc.Format;
	TextureDesc.ArraySize = 1;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.MipLevels = 1;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = m_Device->CreateTexture2D(&TextureDesc, nullptr, &m_GDIImage);

	if (hr == 0x887a0005)
	{
		hr = m_Device->GetDeviceRemovedReason();
	}

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	if (m_GDIImage == nullptr)
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}


	// ���� CPU ���Է��ʵ�����
	D3D11_TEXTURE2D_DESC CPUDesc;
	CPUDesc.Width = m_OutDuplDesc.ModeDesc.Width;
	CPUDesc.Height = m_OutDuplDesc.ModeDesc.Height;
	CPUDesc.Format = m_OutDuplDesc.ModeDesc.Format;
	CPUDesc.ArraySize = 1;
	CPUDesc.BindFlags = 0;
	CPUDesc.MiscFlags = 0;
	CPUDesc.SampleDesc.Count = 1;
	CPUDesc.SampleDesc.Quality = 0;
	CPUDesc.MipLevels = 1;
	CPUDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	CPUDesc.Usage = D3D11_USAGE_STAGING;

	hr = m_Device->CreateTexture2D(&CPUDesc, nullptr, &m_CPUImage);

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	if (m_CPUImage == nullptr)
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	for (UINT TryIndex = 0; TryIndex < uTryCount; ++TryIndex)
	{
		T[0] = clock();
		hr = m_DeskDupl->AcquireNextFrame(500, &FrameInfo, &m_DesktopResource);
		test = TryIndex;
		T[1] = clock();

		if (SUCCEEDED(hr))
		{
			break;
		}

		if (hr == DXGI_ERROR_WAIT_TIMEOUT)
		{
			OutputDebugString("��ʱ\n");
			continue;
		}
		else if (FAILED(hr))
		{
			OutputDebugString("ʧ��\n");
			break;
		}
	}

	/*t2 = clock();
	Sleep(0);
	t1 = clock();*/


	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	// �� Resource ��ȡ Texture2D (m_AcquiredDesktopImage)
	hr = m_DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_AcquiredDesktopImage));

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	m_DesktopResource->Release();
	m_DesktopResource = nullptr;

	if (m_AcquiredDesktopImage == nullptr)
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	// ���� ��ȡ��Texture2D �� m_GDIImage
	m_DeviceContext->CopyResource(m_GDIImage, m_AcquiredDesktopImage);

	// ������굽 m_GDIImage 4
	IDXGISurface1* DXGISurface1 = nullptr;

	hr = m_GDIImage->QueryInterface(__uuidof(IDXGISurface1), reinterpret_cast<void**>(&DXGISurface1));

	if (FAILED(hr))
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	m_CursorInfo = { 0 };
	m_CursorInfo.cbSize = sizeof(m_CursorInfo);
	BOOL bRes = GetCursorInfo(&m_CursorInfo);
	m_FrameData->CursorPos->x = m_CursorInfo.ptScreenPos.x;
	m_FrameData->CursorPos->y = m_CursorInfo.ptScreenPos.y;


	if (bRes == TRUE)
	{
		// �Ƿ�������
		if (m_CursorInfo.flags == CURSOR_SHOWING && m_CaptureSetting->IsDisplay)
		{
			POINT CursorPos = m_CursorInfo.ptScreenPos;
			unsigned long CursorSize = m_CursorInfo.cbSize;

			HDC hDC;

			DXGISurface1->GetDC(FALSE, &hDC);

			DrawIconEx(hDC, CursorPos.x, CursorPos.y, m_CursorInfo.hCursor, 0, 0, 0, 0, DI_NORMAL | DI_DEFAULTSIZE);

			hr = DXGISurface1->ReleaseDC(nullptr);
			ReleaseDC(NULL, hDC);

		}
	}

	DXGISurface1->Release();
	DXGISurface1 = nullptr;


	/*t2 = clock();
	Sleep(0);
	t1 = clock();*/
	T[2] = clock();

	// �� m_GDIImage ��ͼƬ���Ƶ� m_CPUImage 6
	m_DeviceContext->CopyResource(m_CPUImage, m_GDIImage);

	// m_CPUImage ӳ��� bitmap
	UINT Subresource = D3D11CalcSubresource(0, 0, 0);
	hr = m_DeviceContext->Map(m_CPUImage, Subresource, D3D11_MAP_READ_WRITE, 0, &m_MappedRect);

	//t2 = clock();
	//Sleep(0);
	//t1 = clock();
	T[3] = clock();

	// �޸� m_FrameData
	m_FrameData->pData = reinterpret_cast<BYTE*>(m_MappedRect.pData);
	m_FrameData->uSize = m_MappedRect.DepthPitch;
	m_FrameData->BytesPerLine = m_MappedRect.RowPitch;
	m_FrameData->TimeStamp = GetTickCount64() + m_CaptureSetting->OffsetTimeStamp;

	// ����� DDA_WINHANDLE ���� DDA_WINRECT ģʽ, ��Ҫ��֡���ݽ���һЩ����
	if (CaptureMode == CAPTURE_MODE_DDA_WINHANDLE)
	{
		// һ�㲻�����е�����
		BOOL bRet;
		bRet = GetWindowRect(m_CaptureSetting->WinHandle, &m_CaptureSetting->TargetRect);
		if (!bRet)
		{
			return CA_RETURN_ERROR_UNEXPECTED;
		}
		else
		{
			ProcessWindow();
		}
	}
	else if (CaptureMode == CAPTURE_MODE_DDA_WINRECT)
	{
		BOOL bRet;
		bRet = GetWindowRect(m_CaptureSetting->WinHandle, &m_CaptureSetting->TargetRect);
		if (!bRet)
		{
			return CA_RETURN_ERROR_UNEXPECTED;
		}
		else
		{
			ProcessWindow();
		}
	}

	// �ͷ���Դ
	hr = m_DeskDupl->ReleaseFrame();

	if (m_DesktopResource)
	{
		m_DesktopResource->Release();
		m_DesktopResource = nullptr;
	}

	if (m_CPUImage)
	{
		m_CPUImage->Release();
		m_CPUImage = nullptr;
	}

	if (m_GDIImage)
	{
		m_GDIImage->Release();
		m_GDIImage = nullptr;
	}

	if (m_AcquiredDesktopImage)
	{
		m_AcquiredDesktopImage->Release();
		m_AcquiredDesktopImage = nullptr;
	}

	/*t2 = clock();*/
	t4 = clock();

	// ���� FPS
	//static UINT uSleepTime = 0;
	//static UINT uOldFPS = 1;
	//if (uOldFPS != m_CaptureSetting->FPS && m_CaptureSetting->FPS)
	//{
	//	uOldFPS = m_CaptureSetting->FPS;
	//	uSleepTime = (UINT)(1000 / uOldFPS - (t4 - t3) - 3);
	//	if (uSleepTime > 67)
	//	{
	//		uSleepTime = 0;
	//	}
	//}
	//Sleep(uSleepTime);

	clock_t t5 = clock();

	if (t5 - t3 > 200)
	{
		// Sleep(0);
	}

	// ��ʾ������Ϣ
	OutputDebugString("֡����ʱ��: ");
	char chTime[10];
	_itoa_s((int)(t5 - t3), chTime, 10);
	string strTime = string("");
	for (int j = 0; j < 10 && chTime[j]; j++)
	{
		strTime.push_back(chTime[j]);
	}
	OutputDebugStringA(strTime.data());

	OutputDebugString("\n");

	return CA_RETURN_SUCCESS;
}

//
// ��ȡ CAPTURE_FRAMEDATA ���͵�֡���� 
//
void DDACAPTUREMANAGER::GetFrameData(_Out_ CAPTURE_FRAMEDATA* pFrameData)
{
	pFrameData->BytesPerLine = m_FrameData->BytesPerLine;
	pFrameData->CursorPos->x = m_FrameData->CursorPos->x;
	pFrameData->CursorPos->y = m_FrameData->CursorPos->y;
	pFrameData->pData = m_FrameData->pData;
	pFrameData->TimeStamp = m_FrameData->TimeStamp;
	pFrameData->uSize = m_FrameData->uSize;
}

//
// ���ò�������
//
void DDACAPTUREMANAGER::SetCaptureSetting(_In_ CAPTURE_SETTING* pCaptureSetting)
{
	m_CaptureSetting->Anchor = pCaptureSetting->Anchor;
	m_CaptureSetting->FPS = pCaptureSetting->FPS;
	m_CaptureSetting->IsDisplay = pCaptureSetting->IsDisplay;
	m_CaptureSetting->IsFollowCursor = pCaptureSetting->IsFollowCursor;
	m_CaptureSetting->OffsetTimeStamp = pCaptureSetting->OffsetTimeStamp;
	m_CaptureSetting->TargetRect = pCaptureSetting->TargetRect;
	m_CaptureSetting->TimeStamp = pCaptureSetting->TimeStamp;
	m_CaptureSetting->WinHandle = pCaptureSetting->WinHandle;
}

//
// ���ݴ��ھ��δ��� m_FrameData
//
CA_RETURN DDACAPTUREMANAGER::ProcessWindow()
{
	clock_t t1, t2;
	t1 = clock();

	if (!m_CaptureSetting)
	{
		return CA_RETURN_ERROR_UNEXPECTED;
	}

	static UINT OldRectHeight = 0, OldRectWidth = 0, OldRectTop = 0, OldRectLeft = 0;
	UINT RectHeight = m_CaptureSetting->TargetRect.bottom - m_CaptureSetting->TargetRect.top;
	UINT RectWidth = m_CaptureSetting->TargetRect.right - m_CaptureSetting->TargetRect.left;
	RectWidth <<= 2;

	BYTE* pScreen = m_FrameData->pData;
	BYTE* pWindow = pScreen + m_FrameData->BytesPerLine * (m_CaptureSetting->TargetRect.top) + m_CaptureSetting->TargetRect.left;

	// ������ڴ�����С��״̬, ��Ҫ�������⴦��
	if (IsIconic(m_CaptureSetting->WinHandle))
	{
		OutputDebugString("���ڴ�����С��״̬, �����ͺ�ɫ���ο�\n");
		if (!OldRectHeight)
		{
			OutputDebugString("�ɵľ��η�Χδ��ʼ��\n");
			return CA_RETURN_ERROR_EXPECTED;
		}
		memset(pScreen, 0, OldRectHeight * OldRectWidth);

		m_FrameData->uSize = OldRectHeight * OldRectWidth;
		m_FrameData->BytesPerLine = OldRectWidth;

		printf("{%d, %d, %d, %d}\t", OldRectLeft, OldRectTop, OldRectWidth, OldRectHeight);
	}
	else
	{
		OutputDebugString("���ڴ�������״̬\n");

		// �����ڳ�����Ļ��Χʱ����Ҫ��������
		if (m_CaptureSetting->TargetRect.bottom > GetSystemMetrics(SM_CYSCREEN))
		{
			RectHeight -= (m_CaptureSetting->TargetRect.bottom - GetSystemMetrics(SM_CYSCREEN));
		}

		if (m_CaptureSetting->TargetRect.top < 0)
		{
			RectHeight += m_CaptureSetting->TargetRect.top;
			m_CaptureSetting->TargetRect.top = 0;

		}

		if (m_CaptureSetting->TargetRect.left < 0)
		{
			RectWidth += m_CaptureSetting->TargetRect.left << 2;
			m_CaptureSetting->TargetRect.left = 0;
		}

		if (m_CaptureSetting->TargetRect.right > GetSystemMetrics(SM_CXSCREEN))
		{
			RectWidth -= (m_CaptureSetting->TargetRect.right - GetSystemMetrics(SM_CXSCREEN));
		}

		pWindow = pScreen + m_FrameData->BytesPerLine * (m_CaptureSetting->TargetRect.top) + m_CaptureSetting->TargetRect.left;

		// ��ʼ���� bitmap
		for (UINT i = 0; i < RectHeight; i++)
		{
			memcpy(pScreen, pWindow, RectWidth);
			pScreen += RectWidth;
			pWindow += m_FrameData->BytesPerLine;
		}
		m_FrameData->uSize = RectHeight * RectWidth;
		m_FrameData->BytesPerLine = RectWidth;

		// ����ɵ�����
		OldRectHeight = RectHeight;
		OldRectWidth = RectWidth;
		OldRectTop = m_CaptureSetting->TargetRect.top;
		OldRectLeft = m_CaptureSetting->TargetRect.left;

		printf("{%ld, %ld, %d, %d}\t", m_CaptureSetting->TargetRect.left, m_CaptureSetting->TargetRect.top, RectWidth, RectHeight);
	}

	t2 = clock();

	return CA_RETURN_SUCCESS;
}

//
// �ͷ�������Դ
//
void DDACAPTUREMANAGER::Clean()
{
	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}

	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}

	if (m_GDIImage)
	{
		m_GDIImage->Release();
		m_GDIImage = nullptr;
	}

	if (m_DeskDupl)
	{
		m_DeskDupl->Release();
		m_DeskDupl = nullptr;
	}

	if (m_CPUImage)
	{
		m_CPUImage->Release();
		m_CPUImage = nullptr;
	}

	if (m_DesktopResource)
	{
		m_DesktopResource->Release();
		m_DesktopResource = nullptr;
	}

	if (m_AcquiredDesktopImage)
	{
		m_AcquiredDesktopImage->Release();
		m_AcquiredDesktopImage = nullptr;
	}

	if (m_FrameData)
	{
		if (m_FrameData->pData)
		{
			delete m_FrameData->pData;
		}
		if (m_FrameData->CursorPos)
		{
			delete m_FrameData->CursorPos;
		}
	}

	if (m_CaptureSetting)
	{
		if (m_CaptureSetting)
		{
			delete m_CaptureSetting;
			m_CaptureSetting = nullptr;
		}
	}
}

