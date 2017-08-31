#include "Main.h"

#include <iostream>

int main() {
	Main* main = new Main();
	while (main->run()) {}
	return 0;
}

static bool running = false;

static HCURSOR cursor;
static HCURSOR defaultCursor;
static HCURSOR textCursor;

LRESULT CALLBACK BBIDEWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_DESTROY:
			running = false;
			return 0;
		break;
        case WM_SETCURSOR:
            SetCursor(cursor);
        break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Main::Main() {
	running = true;

	hInstance = GetModuleHandle(0);

    defaultCursor = LoadCursor(NULL,IDC_ARROW);
    textCursor = LoadCursor(NULL,IDC_IBEAM);
    cursor = defaultCursor;

	LONG_PTR style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX;
	const wchar_t* className = L"BBIDE";

	// Register Class
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= BBIDEWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= defaultCursor;
	wcex.hbrBackground	= RGB(0,0,0);//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= className;
	wcex.hIconSm		= 0;

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

    SetClassLong(HWnd,
        GCL_HCURSOR,
        (LONG) cursor);
    
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

	eventReceiver = new MainEventReceiver();

	params.EventReceiver = eventReceiver;

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

	font = irr::gui::CGUITTFont::create(device,"cfg/consola.ttf",14);

	videodata = irr::video::SExposedVideoData(HWnd);

	flipShaderCallback = new FlipShaderCallback();

	quad2dMaterialType = (irr::video::E_MATERIAL_TYPE)driver->getGPUProgrammingServices()->addHighLevelShaderMaterial(
		QUAD2D_SHADER_CODE,"vertexMain",irr::video::EVST_VS_4_0,
		QUAD2D_SHADER_CODE,"pixelMain",irr::video::EPST_PS_4_0,flipShaderCallback);

	irr::video::IVertexDescriptor* vDesc = driver->getVertexDescriptor(0);

	irr::scene::SMesh* mesh = new irr::scene::SMesh();
	irr::scene::CMeshBuffer<irr::video::S3DVertex>* buf = new irr::scene::CMeshBuffer<irr::video::S3DVertex>(vDesc);

	irr::video::S3DVertex verts[4];
	verts[0]=irr::video::S3DVertex(irr::core::vector3df(-1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,(float)windowDims.Height/(float)windowDimsPOT.Height)); //top left
	verts[1]=irr::video::S3DVertex(irr::core::vector3df(1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df((float)windowDims.Width/(float)windowDimsPOT.Width,(float)windowDims.Height/(float)windowDimsPOT.Height)); //top right
	verts[2]=irr::video::S3DVertex(irr::core::vector3df(-1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,0.f)); //bottom left
	verts[3]=irr::video::S3DVertex(irr::core::vector3df(1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df((float)windowDims.Width/(float)windowDimsPOT.Width,0.f)); //bottom right

	irr::scene::CVertexBuffer<irr::video::S3DVertex>* vertexBuffer = new irr::scene::CVertexBuffer<irr::video::S3DVertex>();
	irr::scene::CIndexBuffer* indexBuffer = new irr::scene::CIndexBuffer(irr::video::EIT_16BIT);

	for (unsigned int j = 0; j<4; ++j) {
		vertexBuffer->addVertex(verts[j]);
	}

	indexBuffer->addIndex(1);
	indexBuffer->addIndex(0);
	indexBuffer->addIndex(2);
	indexBuffer->addIndex(2);
	indexBuffer->addIndex(3);
	indexBuffer->addIndex(1);

	buf->setVertexBuffer(vertexBuffer, 0);
	buf->setIndexBuffer(indexBuffer);

	buf->getMaterial().MaterialType = quad2dMaterialType;
	buf->getMaterial().BackfaceCulling = true;
	buf->getMaterial().setTexture(0,rtt);

	mesh->addMeshBuffer(buf);
	buf->drop();

	flipMesh = mesh;
	flipQuad = device->getSceneManager()->addMeshSceneNode(mesh);

	std::string keywordDump = execProc("blitzcc +k")+"\n";
	
	std::wstring keyword = L"";
    
    int pos=0; int n;
    while( (n=keywordDump.find( '\n',pos ))!=std::string::npos ){
        std::string t=keywordDump.substr( pos,n-pos-1 );
        for( int q=0;(q=t.find('\r',q))!=std::string::npos; ) t=t.replace( q,1,"" );

        std::string help=t;
        int i=t.find(' ');
        if( i!=std::string::npos ){
            t=t.substr(0,i);if( !t.size() ){
                std::cout<<"Error in keywords\n";
                ExitProcess(0);
            }
            if( !iswalnum(t[t.size()-1]) ) t=t.substr(0,t.size()-1);
        }

        std::wstring keyword = irr::core::stringw(t.c_str()).c_str();

        keywords.keywords.emplace(keyword);
        pos=n+1;
    }

	loadFile(L"Main.bb");
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

			irr::video::S3DVertex verts[4];
			verts[0]=irr::video::S3DVertex(irr::core::vector3df(-1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,(float)windowDims.Height/(float)windowDimsPOT.Height)); //top left
			verts[1]=irr::video::S3DVertex(irr::core::vector3df(1.f,-1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df((float)windowDims.Width/(float)windowDimsPOT.Width,(float)windowDims.Height/(float)windowDimsPOT.Height)); //top right
			verts[2]=irr::video::S3DVertex(irr::core::vector3df(-1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df(0.f,0.f)); //bottom left
			verts[3]=irr::video::S3DVertex(irr::core::vector3df(1.f,1.f,0.f),irr::core::vector3df(0.f,0.f,1.f),irr::video::SColor(255,255,255,255),irr::core::vector2df((float)windowDims.Width/(float)windowDimsPOT.Width,0.f)); //bottom right
			flipQuad->getMesh()->getMeshBuffer(0)->getVertexBuffer(0)->setVertex(0,&verts[0]);
			flipQuad->getMesh()->getMeshBuffer(0)->getVertexBuffer(0)->setVertex(1,&verts[1]);
			flipQuad->getMesh()->getMeshBuffer(0)->getVertexBuffer(0)->setVertex(2,&verts[2]);
			flipQuad->getMesh()->getMeshBuffer(0)->getVertexBuffer(0)->setVertex(3,&verts[3]);

			flipQuad->getMesh()->getMeshBuffer(0)->getMaterial().setTexture(0,rtt);
			flipQuad->setMaterialTexture(0,rtt);
		}
	}

	irr::video::SMaterial mat; mat.MaterialType = irr::video::EMT_SOLID;
	
	driver->beginScene(true,true,irr::video::SColor(255,0,0,0),videodata);
	driver->setRenderTarget(rtt,true,true,irr::video::SColor(255,255,0,255));
	driver->setMaterial(mat);

	int fontHeight = font->getCharDimension(L'W').Height;

	int lineBarWidth = 62;

	irr::core::recti textBoxRect(lineBarWidth-3,32,windowDims.Width,windowDims.Height-20);
	irr::core::recti lineNumRect(irr::core::recti(0, 32, lineBarWidth - 3, textBoxRect.LowerRightCorner.Y));

	bool mouseHit = eventReceiver->getMouseHit(0);

	if (selectedFile >= 0 && selectedFile<files.size() && files.size()>0) {
		irr::core::vector2di& scrollPos = files[selectedFile]->scrollPos;
        irr::core::vector2di& caretPos = files[selectedFile]->caretPos;
        irr::core::vector2di& selectionStart = files[selectedFile]->selectionStart;
        int& selecting = files[selectedFile]->selecting;
		std::vector<Line*>& text = files[selectedFile]->text;

        bool pasting = false;
        bool copying = false;
        if ((eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_RCONTROL)) && eventReceiver->getKeyHit(irr::KEY_KEY_V)) {
            pasting = true;
        }
        if ((eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_RCONTROL)) && eventReceiver->getKeyHit(irr::KEY_KEY_C)) {
            copying = true;
            pasting = false;
        }

		int fileWidth = font->getDimension(files[selectedFile]->text[files[selectedFile]->longestLine]->getText()).Width;
        if (fileWidth<1) {
            fileWidth = 1;
        }

		textBoxRect.LowerRightCorner.X = windowDims.Width;
		textBoxRect.LowerRightCorner.Y = windowDims.Height-20;

		bool verticalScrollEnabled = text.size()*14+2 > textBoxRect.getHeight();
		bool horizontalScrollEnabled = fileWidth > textBoxRect.getWidth();

		if (verticalScrollEnabled) {
			textBoxRect.LowerRightCorner.X = windowDims.Width-20;
		}

		if (horizontalScrollEnabled) {
			textBoxRect.LowerRightCorner.Y = windowDims.Height-40;
		}

        lineNumRect.LowerRightCorner.Y = textBoxRect.LowerRightCorner.Y;

        driver->draw2DRectangle(irr::video::SColor(255, 12, 12, 15), irr::core::recti(45, 32, windowDims.Width - 20, textBoxRect.LowerRightCorner.Y));

        driver->draw2DRectangle(irr::video::SColor(255, 30, 30, 35), irr::core::recti(0, 32, lineBarWidth-3, textBoxRect.LowerRightCorner.Y));
        driver->draw2DLine(irr::core::vector2di(lineBarWidth-12, 32), irr::core::vector2di(lineBarWidth-12, textBoxRect.LowerRightCorner.Y), irr::video::SColor(255, 150, 150, 150));

		int renderStart = scrollPos.Y/14;
		if (renderStart < 0) { renderStart = 0; }

		int renderEnd = (windowDims.Height-52)/14 + renderStart + 2;
		
        std::wstring part1 = text[caretPos.Y]->getText().substr(0,min(caretPos.X,text[caretPos.Y]->getText().size()));
        std::wstring part2 = text[caretPos.Y]->getText().substr(min(caretPos.X,text[caretPos.Y]->getText().size()));

        irr::core::vector2di startSelectRender;
        irr::core::vector2di endSelectRender;

        int part1size = part1.size();

        if (selecting!=2) {
            std::wstring charQueue = eventReceiver->getCharQueue(L"",true).c_str();

            if (pasting) {
                charQueue+=getClipboardText();
            }

            if (charQueue.size()>0) {
                bool appending = true;
                std::wstring appendStr = part1;
                std::wstring remainStr = L"";
                for (int i=0;i<charQueue.size();i++) {
                    if (charQueue[i]!=8) {
                        if ((charQueue[i]==13 || charQueue[i]==10) && appending) {
                            appending = false;
                        } else if (charQueue[i]=='\t') {
                            if (appending) {
                                appendStr+=L"    ";
                            } else {
                                remainStr+=L"    ";
                            }
                        } else {
                            if (appending) {
                                appendStr.push_back(charQueue[i]);
                            } else {
                                remainStr.push_back(charQueue[i]);
                            }
                        }
                    } else if (!appending) {
                        if (remainStr.size()>0) {
                            remainStr.pop_back();
                        } else {
                            appending = true;
                        }
                    } else if (appendStr.size()>0) {
                        appendStr.pop_back();
                    } else if (text.size()>1) {
                        delete text[caretPos.Y]; text.erase(text.begin()+caretPos.Y);
                        caretPos.Y--; caretPos.X = text[caretPos.Y]->getText().size();
                        appendStr = text[caretPos.Y]->getText();
                    }
                }

                part1 = appendStr;
                if (appending) {
                    caretPos.X = part1.size();
                    text[caretPos.Y]->setText(part1+part2);
                    text[caretPos.Y]->formatText(keywords);
                } else {
                    text[caretPos.Y]->setText(part1);
                    text[caretPos.Y]->formatText(keywords);

                    int lastLine = caretPos.Y+1;
                    Line* newLine = new Line();
                    std::wstring lineText = L"";
                    for (int i=0;i<remainStr.size();i++) {
                        if (remainStr[i]==13 || remainStr[i]==10) {
                            if (i<remainStr.size()-1) {
                                if (remainStr[i]==13 && remainStr[i+1]==10) {
                                    i++;
                                }
                            }

                            newLine->setText(lineText);
                            newLine->formatText(keywords);
                            text.insert(text.begin()+lastLine,newLine);
                            newLine = new Line();
                            lastLine++;
                            lineText = L"";
                        } else {
                            lineText.push_back(remainStr[i]);
                        }
                    }
                    newLine->setText(lineText+part2);
                    newLine->formatText(keywords);
                    text.insert(text.begin()+lastLine,newLine);

                    caretPos.Y = lastLine;
                    caretPos.X = lineText.size();
                }
                files[selectedFile]->recalculateLongestLine();
                selectionStart = caretPos;
                selecting = 0;
            }
        } else {
            if (caretPos.Y>selectionStart.Y) {
                startSelectRender = selectionStart;
                endSelectRender = caretPos;
            } else if (caretPos.Y<selectionStart.Y) {
                startSelectRender = caretPos;
                endSelectRender = selectionStart;
            } else if (caretPos.X>selectionStart.X) {
                startSelectRender = selectionStart;
                endSelectRender = caretPos;
            } else {
                startSelectRender = caretPos;
                endSelectRender = selectionStart;
            }

            if (copying) {
                std::wstring textToCopy = L"";
                if (startSelectRender.Y < endSelectRender.Y) {
                    textToCopy = text[startSelectRender.Y]->getText().substr(min(startSelectRender.X,text[startSelectRender.Y]->getText().size()))+L"\n";
                    for (int i=startSelectRender.Y+1;i<endSelectRender.Y;i++) {
                        textToCopy += text[i]->getText()+L"\n";
                    }
                    textToCopy += text[endSelectRender.Y]->getText().substr(0,min(endSelectRender.X,text[endSelectRender.Y]->getText().size()));
                } else {
                    int start = min(startSelectRender.X,text[startSelectRender.Y]->getText().size());
                    int end = min(endSelectRender.X,text[endSelectRender.Y]->getText().size());
                    textToCopy = text[startSelectRender.Y]->getText().substr(start,end-start);
                }
                if (textToCopy.size()>0) { setClipboardText(textToCopy); }
            }

            std::wstring charQueue = eventReceiver->getCharQueue(L"",true).c_str();

            if (pasting) {
                charQueue+=getClipboardText();
            }

            if (charQueue.size()>0) {
                bool appending = true;
                std::wstring appendStr = L"";
                std::wstring remainStr = L"";
                for (int i=0;i<charQueue.size();i++) {
                    if (charQueue[i]!=8) {
                        if ((charQueue[i]==13 || charQueue[i]==10) && appending) {
                            appending = false;
                        } else if (charQueue[i]=='\t') {
                            if (appending) {
                                appendStr+=L"    ";
                            } else {
                                remainStr+=L"    ";
                            }
                        } else {
                            if (appending) {
                                appendStr.push_back(charQueue[i]);
                            } else {
                                remainStr.push_back(charQueue[i]);
                            }
                        }
                    } else if (!appending) {
                        if (remainStr.size()>0) {
                            remainStr.pop_back();
                        } else {
                            appending = true;
                        }
                    } else if (appendStr.size()>0) {
                        appendStr.pop_back();
                    }
                }

                std::wstring firstLinePart1 = text[startSelectRender.Y]->getText().substr(0,min(startSelectRender.X,text[startSelectRender.Y]->getText().size()))+appendStr;
                std::wstring lastLinePart2 = text[endSelectRender.Y]->getText().substr(min(endSelectRender.X,text[endSelectRender.Y]->getText().size()));
                
                if (startSelectRender.Y < endSelectRender.Y) {
                    for (int i=endSelectRender.Y;i>startSelectRender.Y;i--) {
                        delete text[i];
                        text.erase(text.begin()+i);
                    }
                }

                if (appending) {
                    text[startSelectRender.Y]->setText(firstLinePart1+lastLinePart2);

                    caretPos = startSelectRender;
                    caretPos.X = firstLinePart1.size();
                } else {
                    text[startSelectRender.Y]->setText(firstLinePart1);
                    int lastLine = startSelectRender.Y+1;
                    Line* newLine = new Line();
                    std::wstring lineText = L"";
                    for (int i=0;i<remainStr.size();i++) {
                        if (remainStr[i]==13 || remainStr[i]==10) {
                            if (i<remainStr.size()-1) {
                                if (remainStr[i]==13 && remainStr[i+1]==10) {
                                    i++;
                                }
                            }

                            newLine->setText(lineText);
                            newLine->formatText(keywords);
                            text.insert(text.begin()+lastLine,newLine);
                            newLine = new Line();
                            lastLine++;
                            lineText = L"";
                        } else {
                            lineText.push_back(remainStr[i]);
                        }
                    }
                    newLine->setText(lineText+lastLinePart2);
                    newLine->formatText(keywords);
                    text.insert(text.begin()+lastLine,newLine);

                    caretPos.Y = lastLine;
                    caretPos.X = lineText.size();
                }
                text[startSelectRender.Y]->formatText(keywords);
                files[selectedFile]->recalculateLongestLine();

                selectionStart = caretPos;
                startSelectRender = caretPos;
                endSelectRender = caretPos;

                selecting = 0;
            }
        }

        if (renderEnd > text.size()) { renderEnd = text.size(); }

		for (int i = renderStart; i<renderEnd; i++) {
            if (selecting==2) {
                if (i==endSelectRender.Y && i==startSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                        irr::core::recti(
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,startSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,endSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)));
                } else if (i>startSelectRender.Y && i<endSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                                            irr::core::recti(
                                            irr::core::vector2di(lineBarWidth - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText()).Width + 8 - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)));
                } else if (i==startSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                        irr::core::recti(
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,startSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText()).Width + 8 - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)));
                } else if (i==endSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                        irr::core::recti(
                            irr::core::vector2di(lineBarWidth - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,endSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)));
                }
            }

			int lineNumW = font->getDimension(std::to_string(i+1).c_str()).Width;

            if (caretPos.Y == i && device->getTimer()->getTime()%1000<500) {
                int caretX = font->getDimension(part1).Width;
                driver->draw2DLine(irr::core::vector2di(lineBarWidth + caretX - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                                    irr::core::vector2di(lineBarWidth + caretX - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y),
                                    irr::video::SColor(255,255,255,255));
            }

			font->draw(std::to_string(i+1).c_str(),
				irr::core::recti(lineBarWidth-18-lineNumW, 32 - fontHeight + 14 + 14 * i - scrollPos.Y, lineBarWidth, 32 - fontHeight + 28 + 14 * i - scrollPos.Y),
				irr::video::SColor(255,200,200,255),false,true,&lineNumRect
			);
			int x = 0;
			for (int j = 0; j<text[i]->parts.size(); j++) {
				int w = font->getDimension(text[i]->parts[j].getText()).Width;

                if (j>0) {
                    std::wstring prevPart = text[i]->parts[j-1].getText();
                    std::wstring currPart = text[i]->parts[j].getText();
                    if (prevPart.size()>0 && currPart.size()>0) {
                        x+=font->getKerningWidth(prevPart[prevPart.size()-1],currPart[0]);
                    }
                }

				font->draw(text[i]->parts[j].getText().c_str(),
					irr::core::recti(lineBarWidth + x - scrollPos.X, 32 - fontHeight + 14 + 14 * i - scrollPos.Y, lineBarWidth + x + w - scrollPos.X, 32 - fontHeight + 28 + 14 * i - scrollPos.Y),
					text[i]->parts[j].color, false, true,&textBoxRect);
				x += w;
			}
		}
        
		wchar_t tempCStr[1];

		//vertical scroll bar

		int vRealScrollBarHalfHeight = (textBoxRect.getHeight())*(textBoxRect.getHeight()) / (text.size() * 14) / 2;
		int vScrollBarHalfHeight = vRealScrollBarHalfHeight;
		if (vScrollBarHalfHeight < 12) { vScrollBarHalfHeight = 12; }

		int vStartY = 52 + vScrollBarHalfHeight;
		int vEndY = textBoxRect.LowerRightCorner.Y-20-vScrollBarHalfHeight;

		int maxVScrollPos = (text.size() * 14) - (textBoxRect.getHeight()) + 2;

		int vScrollBarCenterY = ((vStartY*(maxVScrollPos-scrollPos.Y))+(vEndY*scrollPos.Y))/maxVScrollPos;

		if (verticalScrollEnabled) {
			driver->draw2DRectangle(irr::video::SColor(255, 60, 60, 60),
									irr::core::recti(textBoxRect.LowerRightCorner.X, 32, textBoxRect.LowerRightCorner.X+20, textBoxRect.LowerRightCorner.Y));

			driver->draw2DRectangle(irr::video::SColor(255, 105, 105, 105),
									irr::core::recti(textBoxRect.LowerRightCorner.X+2, vScrollBarCenterY-vScrollBarHalfHeight+2,textBoxRect.LowerRightCorner.X+18, vScrollBarCenterY+vScrollBarHalfHeight-2));

			tempCStr[0] = 0x25B2;
			font->draw(irr::core::stringw(tempCStr), irr::core::recti(textBoxRect.LowerRightCorner.X, 32, textBoxRect.LowerRightCorner.X+20, 52),irr::video::SColor(255,155,155,155),true,true);
			tempCStr[0] = 0x25BC;
			font->draw(irr::core::stringw(tempCStr), irr::core::recti(textBoxRect.LowerRightCorner.X, textBoxRect.LowerRightCorner.Y-20, textBoxRect.LowerRightCorner.X+20, textBoxRect.LowerRightCorner.Y), irr::video::SColor(255, 155, 155, 155), true, true);
		}

		//horizontal scroll bar

		int hRealScrollBarHalfWidth = (textBoxRect.getWidth())*(textBoxRect.getWidth()) / (fileWidth) / 2;
		int hScrollBarHalfWidth = hRealScrollBarHalfWidth;
		if (hScrollBarHalfWidth < 12) { hScrollBarHalfWidth = 12; }

		int hStartX = lineBarWidth+17+hScrollBarHalfWidth;
		int hEndX = textBoxRect.LowerRightCorner.X-20-hScrollBarHalfWidth;

		int maxHScrollPos = fileWidth - textBoxRect.getWidth() + 2;

		int hScrollBarCenterX = ((hStartX*(maxHScrollPos-scrollPos.X))+(hEndX*scrollPos.X))/maxHScrollPos;


		if (horizontalScrollEnabled) {
			driver->draw2DRectangle(irr::video::SColor(255, 60, 60, 60),
									irr::core::recti(lineBarWidth-3, textBoxRect.LowerRightCorner.Y, textBoxRect.LowerRightCorner.X, textBoxRect.LowerRightCorner.Y+20));

			driver->draw2DRectangle(irr::video::SColor(255, 105, 105, 105),
									irr::core::recti(hScrollBarCenterX-hScrollBarHalfWidth+2, textBoxRect.LowerRightCorner.Y+2, hScrollBarCenterX+hScrollBarHalfWidth-2, textBoxRect.LowerRightCorner.Y+18));

			tempCStr[0] = 0x25C4;
			font->draw(irr::core::stringw(tempCStr), irr::core::recti(lineBarWidth-3, textBoxRect.LowerRightCorner.Y, lineBarWidth+17, textBoxRect.LowerRightCorner.Y+20),irr::video::SColor(255,155,155,155),true,true);
			tempCStr[0] = 0x25BA;
			font->draw(irr::core::stringw(tempCStr), irr::core::recti(textBoxRect.LowerRightCorner.X-20, textBoxRect.LowerRightCorner.Y, textBoxRect.LowerRightCorner.X, textBoxRect.LowerRightCorner.Y+20), irr::video::SColor(255, 155, 155, 155), true, true);
		}

		if (mouseHit) {
			if (verticalScrollEnabled && eventReceiver->getMousePos().X > textBoxRect.LowerRightCorner.X) {
				if (eventReceiver->getMousePos().Y > 32 && eventReceiver->getMousePos().Y < 52) {
					scrollPos.Y -= 14;
				}
				if (eventReceiver->getMousePos().Y > textBoxRect.LowerRightCorner.Y-20 && eventReceiver->getMousePos().Y < textBoxRect.LowerRightCorner.Y) {
					scrollPos.Y += 14;
				}
			}

			if (horizontalScrollEnabled && eventReceiver->getMousePos().Y > textBoxRect.LowerRightCorner.Y && eventReceiver->getMousePos().Y < textBoxRect.LowerRightCorner.Y+20) {
				if (eventReceiver->getMousePos().X > lineBarWidth-3 && eventReceiver->getMousePos().X < lineBarWidth+17) {
					scrollPos.X -= 70;
				}
				if (eventReceiver->getMousePos().X > textBoxRect.LowerRightCorner.X-20 && eventReceiver->getMousePos().X < textBoxRect.LowerRightCorner.X) {
					scrollPos.X += 70;
				}
			}
		}

        irr::core::vector2di oldCaretPos = caretPos;

        if (eventReceiver->getMouseDown(0) && isScrolling==SCROLL::NONE) {
            if (textBoxRect.isPointInside(eventReceiver->getMousePos())) {
                caretPos.Y = max(0,min(text.size()-1,(eventReceiver->getMousePos().Y-textBoxRect.UpperLeftCorner.Y+scrollPos.Y)/14));

                if (text[caretPos.Y]->getText().size()>0) {
                    caretPos.X = max(0,min(text[caretPos.Y]->getText().size(),
                        (eventReceiver->getMousePos().X-textBoxRect.UpperLeftCorner.X+scrollPos.X)*text[caretPos.Y]->getText().size()/font->getDimension(text[caretPos.Y]->getText()).Width
                    ));
                } else {
                    caretPos.X = 0;
                }

                if (mouseHit) {
                    if (eventReceiver->getKeyDown(irr::KEY_LSHIFT) || eventReceiver->getKeyDown(irr::KEY_RSHIFT)) {
                        if (selecting == 0) {
                            selectionStart = oldCaretPos;
                        }
                        selecting = 2;
                    } else {
                        selecting = 1;
                        selectionStart = caretPos;
                    }
                } else if (selecting==1 && selectionStart != caretPos) {
                    selecting = 2;
                }
            }
        }

        oldCaretPos = caretPos;

        if (eventReceiver->getKeyHit(irr::KEY_LEFT)) {
            if (caretPos.X>text[caretPos.Y]->getText().size()) {
                caretPos.X=text[caretPos.Y]->getText().size();
            }
            caretPos.X--;
            if (caretPos.X<0) {
                caretPos.Y--;
                if (caretPos.Y<0) {
                    caretPos.X = 0;
                    caretPos.Y = 0;
                } else {
                    caretPos.X = text[caretPos.Y]->getText().size();
                }
            }

            if (!eventReceiver->getKeyDown(irr::KEY_LSHIFT) && !eventReceiver->getKeyDown(irr::KEY_RSHIFT)) {
                selecting = 0;
            } else if (selecting!=2) {
                selectionStart = oldCaretPos;
                selecting = 2;
            }
        }

        if (eventReceiver->getKeyHit(irr::KEY_RIGHT)) {
            caretPos.X++;
            if (caretPos.X>text[caretPos.Y]->getText().size()) {
                caretPos.Y++;
                if (caretPos.Y>text.size()-1) {
                    caretPos.Y = text.size()-1;
                    caretPos.X = text[caretPos.Y]->getText().size();
                } else {
                    caretPos.X = 0;
                }
            }

            if (!eventReceiver->getKeyDown(irr::KEY_LSHIFT) && !eventReceiver->getKeyDown(irr::KEY_RSHIFT)) {
                selecting = 0;
            } else if (selecting!=2) {
                selectionStart = oldCaretPos;
                selecting = 2;
            }
        }
        if (eventReceiver->getKeyHit(irr::KEY_UP)) {
            caretPos.Y--;
            if (caretPos.Y<0) {
                caretPos.Y = 0;
            }

            if (!eventReceiver->getKeyDown(irr::KEY_LSHIFT) && !eventReceiver->getKeyDown(irr::KEY_RSHIFT)) {
                selecting = 0;
            } else if (selecting!=2) {
                selectionStart = oldCaretPos;
                selecting = 2;
            }
        }
        if (eventReceiver->getKeyHit(irr::KEY_DOWN)) {
            caretPos.Y++;
            if (caretPos.Y>text.size()-1) {
                caretPos.Y = text.size()-1;
            }

            if (!eventReceiver->getKeyDown(irr::KEY_LSHIFT) && !eventReceiver->getKeyDown(irr::KEY_RSHIFT)) {
                selecting = 0;
            } else if (selecting!=2) {
                selectionStart = oldCaretPos;
                selecting = 2;
            }
        }

        HCURSOR oldCursor = cursor;
        if (textBoxRect.isPointInside(eventReceiver->getMousePos())) {
            cursor = textCursor;
        } else {
            cursor = defaultCursor;
        }

        if (oldCursor != cursor) {
            SetClassLong(HWnd,
                GCL_HCURSOR,
                (LONG) cursor);
        }

		float mouseWheel = eventReceiver->getMouseWheel();
		scrollPos.Y -= mouseWheel*42;

		if (eventReceiver->getMouseDown(0)) {
			int newVScrollPos = (eventReceiver->getMousePos().Y-vStartY)*maxVScrollPos/(vEndY-vStartY);

			int newHScrollPos = (eventReceiver->getMousePos().X-hStartX)*maxHScrollPos/(hEndX-hStartX);

			if (verticalScrollEnabled && eventReceiver->getMousePos().X > textBoxRect.LowerRightCorner.X) {
				if (eventReceiver->getMousePos().Y > 52 && eventReceiver->getMousePos().Y < textBoxRect.LowerRightCorner.Y-20) {
					if (mouseHit) {
						if (eventReceiver->getMousePos().Y > vScrollBarCenterY - vScrollBarHalfHeight && eventReceiver->getMousePos().Y < vScrollBarCenterY + vScrollBarHalfHeight) {
							if (isScrolling == SCROLL::NONE) {
								scrollOffset = newVScrollPos - scrollPos.Y;
								isScrolling = SCROLL::VERTICAL;
							}
						} else {
							scrollPos.Y = (scrollPos.Y+scrollPos.Y+newVScrollPos)/3;
						}
					}
				}
			}

			if (horizontalScrollEnabled && eventReceiver->getMousePos().Y > textBoxRect.LowerRightCorner.Y && eventReceiver->getMousePos().Y < textBoxRect.LowerRightCorner.Y+20) {
				if (eventReceiver->getMousePos().X > lineBarWidth+17 && eventReceiver->getMousePos().X < textBoxRect.LowerRightCorner.X) {
					if (mouseHit) {
						if (eventReceiver->getMousePos().X > hScrollBarCenterX - hScrollBarHalfWidth && eventReceiver->getMousePos().X < hScrollBarCenterX + hScrollBarHalfWidth) {
							if (isScrolling == SCROLL::NONE) {
								scrollOffset = newHScrollPos - scrollPos.X;
								isScrolling = SCROLL::HORIZONTAL;
							}
						} else {
							scrollPos.X = (scrollPos.X+scrollPos.X+newHScrollPos)/3;
						}
					}
				}
			}
			if (isScrolling == SCROLL::VERTICAL) {
				scrollPos.Y = newVScrollPos - scrollOffset;
			} else if (isScrolling == SCROLL::HORIZONTAL) {
				scrollPos.X = newHScrollPos - scrollOffset;
			}
		} else {
			isScrolling = SCROLL::NONE;
		}

		if (scrollPos.Y > maxVScrollPos) {
			scrollPos.Y = maxVScrollPos;
		}
		if (scrollPos.Y < 0) {
			scrollPos.Y = 0;
		}


		if (scrollPos.X > maxHScrollPos) {
			scrollPos.X = maxHScrollPos;
		}
		if (scrollPos.X < 0) {
			scrollPos.X = 0;
		}
	}

	driver->setRenderTarget(0,true,true,irr::video::SColor(255,0,255,0));
	driver->setMaterial(mat);
	flipQuad->render();
	driver->endScene();

	//std::cout << driver->getFPS() << "\n";

	return device->run();
}

std::wstring Main::Line::getText() {
	return text;
}

static bool isfmt( int ch,int nxt ){
	return ch==L';' || ch==L'\"' || iswalpha(ch) || isdigit(ch) || (ch==L'$' && isxdigit(nxt));
}

static bool isid( int c ){
	return iswalnum(c)||c=='_';
}

const irr::video::SColor Main::Line::Part::colors[6] = {
	irr::video::SColor(255,200,200,200),
	irr::video::SColor(255,255,200,100),
	irr::video::SColor(255,102,187,238),
	irr::video::SColor(255,0,204,102),
	irr::video::SColor(255,200,255,200)
};

void Main::Line::setText(std::wstring inText) {
    text = inText;
    parts.clear();

    Part newPart; newPart.text = inText; newPart.color = Part::colors[0];
    parts.push_back(newPart);
}

void Main::Line::setTextUTF8(std::string inText) {
    text = utf8ToWChar(inText);
    parts.clear();

    Part newPart; newPart.text = text; newPart.color = Part::colors[0];
    parts.push_back(newPart);
}

void Main::Line::formatText(Main::Keywords& keywords) {
	parts.clear();
	std::wstring out;
	int cf=0;
	for( int k=0;k<(int)text.size(); ){
		int from=k;
		int pf=cf;
		int c=text[k],is_sz=text.size();
		if( !isgraph( c ) ){
			for( ++k;k<is_sz && !isgraph(text[k]);++k ){}
		}else if( !isfmt( c,k+1<is_sz?text[k+1]:0 ) ){
			for( ++k;k<is_sz && !isfmt( text[k],k+1<is_sz?text[k+1]:0 );++k ){}
			cf=0;
		}else if( c==L';' ){					//comment?
			k=is_sz;
			cf=3;
		}else if( c==L'\"' ){		//string const?
			for( ++k;k<is_sz && text[k]!='\"';++k ){}
			if( k<is_sz ) ++k;
			cf=1;
		}else if( iswalpha( c ) ){		//ident?
			for( ++k;k<is_sz && isid(text[k]);++k ){}
			if( keywords.findKeyword(text.substr( from,k-from )) ) cf=0;
			else cf=2;
		}else if( c==L'$' ){
			for( ++k;k<is_sz && isxdigit(text[k]);++k ){}
			cf=4;
		}else if( isdigit( c ) ){	//numeric const?
			for( ++k;k<is_sz && isdigit(text[k]);++k ){}
			cf=4;
		}
		Part newPart;
		newPart.color = Part::colors[cf];
        cf = 0;
		newPart.text = text.substr( from,k-from );
		parts.push_back(newPart);
	}

	isFormatted = true;

	/*if( text[0]=='F' && text.find( L"Function" )==0 ){
		for( int k=8;k<(int)text.size();++k ){
			if( iswalpha( text[k] ) ){
				int start=k;
				for( ++k;k<(int)text.size() && isid(text[k]);++k ){}
				//funcList.insert( textnum,text.substr( start,k-start ) );
				break;
			}
		}
	}else if( text[0]==L'T' && text.find( L"Type" )==0 ){
		for( int k=4;k<(int)text.size();++k ){
			if( iswalpha( text[k] ) ){
				int start=k;
				for( ++k;k<(int)text.size() && isid(text[k]);++k ){}
				//typeList.insert( textnum,text.substr( start,k-start ) );
				break;
			}
		}
	}else if( text[0]==L'.' ){
		for( int k=1;k<(int)text.size();++k ){
			if( iswalpha( text[k] ) ){
				int start=k;
				for( ++k;k<(int)text.size() && isid(text[k]);++k ){}
				//labsList.insert( textnum,text.substr( start,k-start ) );
				break;
			}
		}
	}*/
}

bool Main::Keywords::findKeyword(std::wstring keyword) {
    // This effectively fixes the really long load times for large files
    // since certain keywords are far more common than others
    for (int i=0;i<recentKeywords.size();i++) {
        if (recentKeywords[i]==keyword) {
            return true;
        }
    }

    bool retVal = keywords.find( keyword )==keywords.end();
    if (retVal) {
        recentKeywords.push_back(keyword);
        if (recentKeywords.size()>10) {
            recentKeywords.erase(recentKeywords.begin());
        }
    }
    return retVal;
}

void Main::File::recalculateLongestLine() {
    if (longestLine>=text.size()) {
        longestLine = 0;
    }
	int length = text[longestLine]->getText().size();
	for (int i=0;i<text.size();i++) {
		if (text[i]->getText().size()>length) {
			length = text[longestLine]->getText().size();
			longestLine = i;
		}
	}
}

Main::File* Main::loadFile(std::wstring name) {
	File* newFile = new File();
	newFile->caretPos = irr::core::vector2di(0,0);
	newFile->scrollPos = irr::core::vector2di(0,0);
	newFile->changed = false;
	newFile->name = name;

	std::string data = "";
	char buffer[1025];

	irr::io::IFileSystem* fs = device->getFileSystem();
	irr::io::IReadFile* file = fs->createAndOpenFile(name.c_str());
	int bytesRead = 0;

	while ((bytesRead = file->read(buffer, 1024))>0) {
		buffer[bytesRead] = 0;
		data+=buffer;
	}

	std::string currLine = "";
	for (int i=0; i<data.size(); i++) {
		if (data[i]==L'\r') {
			//skip
		} else if (data[i]==L'\n') {
			Line* newLine = new Line();
			newLine->setText(utf8ToWChar(currLine));
			newLine->formatText(keywords); //TODO: format text when scrolling, not after loading
			newFile->text.push_back(newLine);
			//std::cout<<currLine.c_str()<<"\n";
			currLine = "";
		} else if (data[i]==L'\t') {
			currLine += "    ";
		} else {
			currLine += data[i];
		}
	}

	newFile->recalculateLongestLine();

	files.push_back(newFile);
	return newFile;
}

static std::string execProc( const std::string& proc ){
	//TODO: figure out if this is the best way to get the keywords, it's probably not
	HANDLE rd,wr;

	SECURITY_ATTRIBUTES sa={sizeof(sa),0,true};

	if( CreatePipe( &rd,&wr,&sa,0 ) ){
		STARTUPINFOA si={sizeof(si)};
		si.dwFlags=STARTF_USESTDHANDLES;
		si.hStdOutput=si.hStdError=wr;
		PROCESS_INFORMATION pi={0};
		if( CreateProcessA( 0,(char*)proc.c_str(),0,0,true,DETACHED_PROCESS,0,0,&si,&pi ) ){
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
			CloseHandle( wr );

			std::string t;
			char *buf=new char[1024];
			for(;;){
				unsigned long sz;
				int n=ReadFile( rd,buf,1024,&sz,0 );
				if( !n && GetLastError()==ERROR_BROKEN_PIPE ) break;
				if( !n ){ t="";break; }
				if( !sz ) break;
				t+=std::string( buf,sz );
			}
			delete[] buf;
			CloseHandle(rd);
			return t;
		}
		CloseHandle( rd );
		CloseHandle( wr );
	}
	std::cout<< (proc+" failed").c_str() << "\n";
	ExitProcess(0);
	return "";
}

static std::wstring utf8ToWChar(std::string utf8Str) {
    std::wstring retVal = L"";
    for (int i=0;i<utf8Str.size();i++) {
        if ((utf8Str[i]&0x80)==0) { //regular ASCII: just copy the char over
            retVal.push_back((wchar_t)utf8Str[i]);
        } else { //most significant bit is 1 => check for codepoint validity
            char firstByte = utf8Str[i];
            char sBits = 0x00;
            int sBitCount = 0;
            if ((firstByte&0xC0)==0xC0) { //two most significant bits are 1 => unicode confirmed
                std::cout<<"UNICHAR\n";
                int character = 0x00;

                while ((firstByte&(1<<(7-sBitCount)))!=0) {
                    sBits |= (1<<(7-sBitCount));
                    sBitCount++;
                    if (sBitCount>7) {
                        std::cout<<"OOPS1\n";
                        break; //all 1's???
                    }
                }

                if (sBitCount<2 || sBitCount>7) {
                    std::cout<<"OOPS2\n";
                    continue;
                }

                character |= firstByte&((~sBits)&0xff);

                bool goodChar = true;
                for (int j=1;j<sBitCount;j++) {
                    int nextChar = utf8Str[i+j];

                    if ((nextChar&0x80)==0x80) { //next codepoint is good
                        character = (character<<6)|(nextChar&0x3f);
                    } else {
                        std::cout<<"OOPS3 "<<(int)(nextChar&0x80)<<"\n";
                        goodChar = false;
                        break; //codepoint is bad, stop
                    }
                }
                i+=sBitCount-1;

                if (goodChar) {
                    retVal.push_back((wchar_t)character);
                } else {
                    continue;
                }
            } else { continue; }
        }
    }
    return retVal;
}

static std::wstring getClipboardText() {
    if (!OpenClipboard(NULL))
        return 0;

    wchar_t * buffer = 0;

    HANDLE hData = GetClipboardData( CF_UNICODETEXT );
    buffer = (wchar_t*)GlobalLock( hData );
    GlobalUnlock( hData );
    CloseClipboard();
    return buffer;
}

static void setClipboardText(std::wstring text) {
    if (!OpenClipboard(NULL))
        return;

    std::cout<<"COPYING!\n";

    wchar_t * buffer = 0;
    size_t len = (text.size()+1)*sizeof(wchar_t);

    HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE,len);
    memcpy(GlobalLock(hMem), text.data(), len);
    GlobalUnlock(hMem);
    SetClipboardData( CF_UNICODETEXT,hMem );
    CloseClipboard();
}
