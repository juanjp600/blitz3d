#ifndef BBIDE_MAIN_H
#define BBIDE_MAIN_H

#include <windows.h>
#include <irrlicht.h>

extern LRESULT CALLBACK BBIDEWndProc(HWND, UINT, WPARAM, LPARAM);

class Main {
	private:
		irr::IrrlichtDevice* device;

		irr::scene::ISceneManager* smgr;
		irr::video::IVideoDriver* driver;

		HINSTANCE hInstance;
		HWND HWnd;

		irr::core::dimension2du windowDims;
		irr::core::dimension2du windowDimsPOT;

		irr::video::ITexture* rtt;

		irr::video::SExposedVideoData videodata;
	public:
		Main();

		bool run();
};

#endif //BBIDE_MAIN_H
