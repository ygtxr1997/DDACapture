#include "DDACaptureManager.h"

int main()
{
	DDACAPTUREMANAGER Mgr;
	Mgr.Init();

	CAPTURE_SETTING Setting;
	Setting.FPS = 1000;
	Setting.IsDisplay = true;
	// Setting.WinHandle = FindWindow(NULL, "������ʾ��");
	Setting.WinHandle = FindWindow(NULL, "�±�ǩҳ - Google Chrome");

	CAPTURE_FRAMEDATA FrameData;

	Mgr.SetCaptureSetting(&Setting);

	int a = 0;
	BYTE* pAlloc = nullptr;
	UINT uSize;
	while (500)
	{
		a++;
		Mgr.GetOneFrame(CAPTURE_MODE_DDA_WINHANDLE);
		Mgr.GetFrameData(&FrameData);
		printf("%d\tʱ���Ϊ%ld\t", a, FrameData.TimeStamp);
		printf("[%d, %d]\n", FrameData.CursorPos->x, FrameData.CursorPos->y);
	}

	return 0;

}