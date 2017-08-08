#include "Main.h"

#include <iostream>

int main() {
	Main* main = new Main();
	while (main->run()) {}
	return 0;
}

static bool running = false;
LRESULT CALLBACK BBIDEWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_DESTROY:
			running = false;
			return 0;
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Main::Main() {
	running = true;

	hInstance = GetModuleHandle(0);

	LONG_PTR style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
	const wchar_t* className = L"BBIDE";

	// Register Class
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= BBIDEWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= className;
	wcex.hIconSm		= 0;

	/*WNDCLASSEX wcex;
	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = (WNDPROC)CustomWndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = DLGWINDOWEXTRA;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = NULL;
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName   = 0;
	wcex.lpszClassName  = Win32ClassName;
	wcex.hIconSm        = 0;*/

	if (!RegisterClassEx(&wcex)) {
		std::cout<<"Failed to register window class\n";
	}

	RECT clientSize;
	clientSize.top = 0;
	clientSize.left = 0;
	clientSize.right = 1280;
	clientSize.bottom = 720;

	AdjustWindowRect(&clientSize, style, FALSE);

	const irr::s32 realWidth = clientSize.right - clientSize.left;
	const irr::s32 realHeight = clientSize.bottom - clientSize.top;

	const irr::s32 windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	const irr::s32 windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

	HWnd = CreateWindowEx(
		0,
		className,
		L"Blitz3D-600",
		style,
		windowLeft,
		windowTop,
		realWidth,
		realHeight,
		NULL,
		NULL,
		hInstance,
		NULL);
	
	if (HWnd == NULL) {
		std::cout<<"Failed to create window\n";
		std::cout<<GetLastError()<<"\n";
	}

	ShowWindow(HWnd,SW_SHOW);
	UpdateWindow(HWnd);

	MoveWindow(HWnd, windowLeft, windowTop, realWidth, realHeight, TRUE);

	std::cout<<"WINDOW CREATED\n";

	irr::SIrrlichtCreationParameters params;
	params.DriverType = irr::video::EDT_DIRECT3D11;
	params.WindowId = HWnd;

	device = irr::createDeviceEx(params);

	smgr = device->getSceneManager();
	driver = device->getVideoDriver();

	windowDims.Width = 1280;
	windowDims.Height = 720;

	driver->OnResize(windowDims);

	irr::core::dimension2du potDims(128,128);
	while (potDims.Width < windowDims.Width) {
		potDims.Width <<= 1;
	}
	while (potDims.Height < windowDims.Height) {
		potDims.Height <<= 1;
	}

	std::cout<<"CHANGING TEXTURE "<<potDims.Width<<" "<<potDims.Height<<"\n";

	windowDimsPOT = potDims;
	rtt = driver->addRenderTargetTexture(windowDimsPOT,"rt",irr::video::ECF_R8G8B8);

	driver->beginScene();
	driver->setRenderTarget(rtt);
	driver->setRenderTarget(0);
	driver->endScene();

	videodata = irr::video::SExposedVideoData(HWnd);
}

bool Main::run() {
	if (!running) {
		device->closeDevice();
	}

	if (GetActiveWindow() == HWnd) {
		RECT wRect; GetClientRect(HWnd,&wRect);
		
		irr::core::dimension2du tDims;
		tDims.Width = wRect.right-wRect.left;
		tDims.Height = wRect.bottom-wRect.top;

		if (tDims != windowDims) {
			driver->removeTexture(rtt);

			driver->OnResize(tDims);

			windowDims = tDims;

			irr::core::dimension2du potDims(128,128);
			while (potDims.Width < windowDims.Width) {
				potDims.Width <<= 1;
			}
			while (potDims.Height < windowDims.Height) {
				potDims.Height <<= 1;
			}

			std::cout<<"CHANGING TEXTURE "<<potDims.Width<<" "<<potDims.Height<<"\n";

			windowDimsPOT = potDims;
			rtt = driver->addRenderTargetTexture(windowDimsPOT,"rt",irr::video::ECF_R8G8B8);

			driver->beginScene();
			driver->setRenderTarget(rtt);
			driver->setRenderTarget(0);
			driver->endScene();
		}
	}

	irr::video::SMaterial mat; mat.MaterialType = irr::video::EMT_SOLID;
	
	driver->beginScene(true,true,irr::video::SColor(255,0,0,0),videodata);
	driver->setRenderTarget(rtt,true,true,irr::video::SColor(255,30,30,30));
	driver->setMaterial(mat);

	driver->draw2DRectangle(irr::video::SColor(255,12,12,15),irr::core::recti(45,32,windowDims.Width-20,windowDims.Height-20));
	driver->draw2DRectangle(irr::video::SColor(255,30,30,35),irr::core::recti(0,32,45,windowDims.Height-20));
	driver->draw2DLine(irr::core::vector2di(36,32),irr::core::vector2di(36,windowDims.Height-20),irr::video::SColor(255,150,150,150));

	driver->setRenderTarget(0,true,true,irr::video::SColor(255,0,255,0));
	driver->setMaterial(mat);
	driver->draw2DImage(rtt,irr::core::recti(0,0,windowDims.Width,windowDims.Height),irr::core::recti(0,0,windowDims.Width,windowDims.Height));
	driver->endScene();
	return device->run();
}
