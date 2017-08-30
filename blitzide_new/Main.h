#ifndef BBIDE_MAIN_H
#define BBIDE_MAIN_H

#include <windows.h>
#include <irrlicht.h>
#include "CGUITTFont.h"
#include "MainEventReceiver.h"

#include <string>
#include <vector>
#include <set>

extern LRESULT CALLBACK BBIDEWndProc(HWND, UINT, WPARAM, LPARAM);

extern std::string execProc(const std::string& proc);

class Main {
	private:
		irr::IrrlichtDevice* device;

		irr::scene::ISceneManager* smgr;
		irr::video::IVideoDriver* driver;

		MainEventReceiver* eventReceiver;

		HINSTANCE hInstance;
		HWND HWnd;

		irr::core::dimension2du windowDims;
		irr::core::dimension2du windowDimsPOT;

		irr::video::ITexture* rtt;
		irr::video::ITexture* textBoxT;

		irr::video::SExposedVideoData videodata;

		irr::video::IShaderConstantSetCallBack* flipShaderCallback;
		irr::video::E_MATERIAL_TYPE quad2dMaterialType;
		irr::scene::IMeshSceneNode* flipQuad; irr::scene::IMesh* flipMesh;
		irr::scene::IMeshBuffer* flipBuf;

		const char* QUAD2D_SHADER_CODE =
			"cbuffer cbParams : register(b0)\n\
				{\n\
				\n\
				};\n\
				\n\
				struct VS_INPUT\n\
				{\n\
					float4 vPosition : POSITION;\n\
					float3 vNormal   : NORMAL;\n\
					float2 texCoord0 : TEXCOORD0;\n\
				};\n\
				\n\
				// Vertex shader output structure\n\
				struct VS_OUTPUT\n\
				{\n\
					float4 Position   : SV_Position;	// vertex position\n\
					float2 TexCoord0  : TEXTURE0;		// tex coords\n\
				};\n\
				\n\
				VS_OUTPUT vertexMain( VS_INPUT input )\n\
				{\n\
					VS_OUTPUT Output;\n\
					\n\
					Output.Position = input.vPosition;\n\
					\n\
					Output.TexCoord0 = input.texCoord0;\n\
					Output.TexCoord0.y = input.texCoord0.y;\n\
					\n\
					return Output;\n\
				}\n\
				\n\
				Texture2D imgTexture : register(t0);\n\
				\n\
				SamplerState st0 : register(s0);\n\
				\n\
				float4 pixelMain( VS_OUTPUT input ) : SV_Target\n\
				{\n\
					float4 Output;\n\
					\n\
					float4 col0 = imgTexture.Sample( st0, input.TexCoord0 ).bgra;\n\
					\n\
					Output = col0;\n\
					\n\
					return Output;\n\
				}\n";

		class FlipShaderCallback : public irr::video::IShaderConstantSetCallBack
		{
			public:
			virtual void OnSetConstants(irr::video::IMaterialRendererServices* services,
				irr::s32 userData) { }
		};

		irr::gui::CGUITTFont* font;

        struct Keywords {
            bool findKeyword(std::wstring keyword);
            std::set<std::wstring> keywords;
            std::vector<std::wstring> recentKeywords;
        } keywords;

		struct Line {
			struct Part {
				static const irr::video::SColor colors[6];

				irr::video::SColor color;
				std::wstring text;

				virtual std::wstring getText() { return text; }
			};

			std::wstring text;

			bool isFormatted = false;
			std::vector<Part> parts;

			std::string getTextUTF8();
			void setTextUTF8(std::string inText);
			std::wstring getText();
			void setText(std::wstring inText);
			void formatText(Main::Keywords& keywords);
			//void draw(irr::video::IVideoDriver* driver,irr::gui::CGUITTFont* font);
		};
		
		struct File {
			std::wstring name;
			bool changed;
			std::vector<Line*> text;

			int longestLine = 0;
			void recalculateLongestLine();

            int selecting = 0;
            irr::core::vector2di selectionStart;

			irr::core::vector2di caretPos;
			irr::core::vector2di scrollPos;
		};

		enum class SCROLL {
			NONE = 0,
			HORIZONTAL,
			VERTICAL
		};
		SCROLL isScrolling = SCROLL::NONE;
		int scrollOffset = 0;

		int selectedFile = 0;
		
		std::vector<File*> files;

		File* loadFile(std::wstring name);
	public:
		Main();

		bool run();
};

#endif //BBIDE_MAIN_H
