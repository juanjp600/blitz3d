Blitz3D open source release! (VS2010/2013 version)

You can grab the prebuilt free version of blitz3d from www.blitzbasic.com.

Steps to build:
1) Install directx7 sdk into same dir as blitz3d:, the foldername should be DX7_Sdk 

2) Install freeimage241 into same dir as blitz3d: http://monkeycoder.co.nz/downloads/freeimage241.zip

PLEASE NOTE - IMPORTANT
=======================
You will need to edit the freeimage.h file and modify the following line (normally should be line #206)

typedef FIBITMAP *(DLL_CALLCONV * FI_AllocateProc)(int width, int height, int bpp, unsigned red_mask FI_DEFAULT(0), unsigned green_mask FI_DEFAULT(0), unsigned blue_mask FI_DEFAULT(0));

to this...

typedef FIBITMAP *(DLL_CALLCONV * FI_AllocateProc)(int width, int height, int bpp, unsigned red_mask , unsigned green_mask , unsigned blue_mask);

You have to modify this line because the original line of code is not compatible with versions of visual studio that are newer than vs6.  Changing this line will remove any compile error concerning this bug

3) Install fmodapi375win into same dir as blitz3d: http://www.fmod.org/files/public/fmodapi375win.zip

4) Open blitz3d workspace in MSVC 6.0.

5) Build project 'bblaunch' using config 'Win32 Blitz3D Release'.

6) Output files should end up in _release subdir.

7) Also copy fmodapi375win/api/fmod.dll to _release/bin.

8) Done?

Blitz3d is released under the zlib/libpng license.

The zlib/libpng License

Copyright (c) 2013 Blitz Research Ltd

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
