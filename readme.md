Blitz3D-600
=======================
**(~~MAVLess~~/Stacktrace/DX11/VS2017/OpenAL/whatever else)**

## Steps to build:
1) Install some secret modified copy of irrlicht-shader-pipeline, I'll release it later

2) Build libogg and libvorbis (for static linking, Release mode), and place them into same dir as Blitz3D: https://xiph.org/

### NOTE
You will need to modify `ogg/ogg.h` and `vorbis/vorbisfile.h` to manually set the calling convention of each function to `__cdecl`, as Blitz3D's runtime uses `__stdcall` and will not link with these libraries unless this is clarified in the source.

3) Open 'blitz3d.sln' in Visual Studio 2017 (2015 should still work, though you'll have to change the SDK version)

4) Build project 'bblaunch' using config 'Win32 Blitz3D Release'.

5) Output files should end up in '_release' subdir.

6) If necessary, copy OpenAL32.dll to '_release/bin'.

7) Done?

**Blitz3D is released under the zlib/libpng license.**

```The zlib/libpng License

Copyright (c) 2013 Blitz Research Ltd

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
```