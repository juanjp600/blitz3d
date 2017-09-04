#include "Main.h"

#include <iostream>
#include <fstream>

int main() {
	Main* main = new Main();
	while (main->run()) {}
	return 0;
}

static bool running = false;
static bool requestClose = false;

static HCURSOR cursor;
static HCURSOR defaultCursor;
static HCURSOR textCursor;

LRESULT CALLBACK BBIDEWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
        case WM_CLOSE:
            requestClose = true;
            return 0;
        break;
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
    driver->setTextureCreationFlag(irr::video::E_TEXTURE_CREATION_FLAG::ETCF_NO_ALPHA_CHANNEL,false);
    toolbarTex = driver->getTexture("cfg/toolbar.png");
    
	driver->beginScene();
	driver->setRenderTarget(rtt);
	driver->setRenderTarget(0);
	driver->endScene();

    WCHAR winDir[MAX_PATH];
    GetWindowsDirectory(winDir, MAX_PATH);

    std::wstring fontPath = std::wstring(winDir)+L"\\Fonts\\consola.ttf";

    font = irr::gui::CGUITTFont::create(device,fontPath.c_str(),14);
    smallFont = irr::gui::CGUITTFont::create(device,fontPath.c_str(),10);

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
	driver->setRenderTarget(rtt,true,true,irr::video::SColor(255,0,0,0));
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
        
        File::ActionMem* tempMem = files[selectedFile]->tempMem;
        /*if (tempMem!=nullptr) {
            delete tempMem;
            files[selectedFile]->tempMem = nullptr;
            tempMem = nullptr;
        }*/

        bool caretScroll = false;

        bool pasting = false;
        bool copying = false;
        bool cutting = false;
        if ((eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_RCONTROL)) && eventReceiver->getKeyHit(irr::KEY_KEY_V)) {
            pasting = true;
        }
        if ((eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_RCONTROL)) && eventReceiver->getKeyHit(irr::KEY_KEY_C)) {
            copying = true;
            pasting = false;
        }
        if ((eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_RCONTROL)) && eventReceiver->getKeyHit(irr::KEY_KEY_X)) {
            copying = true;
            cutting = true;
            pasting = false;
        }
        if (tempMem!=nullptr) {
            for (int i=0;i<files[selectedFile]->redoMem.size();i++) {
                delete files[selectedFile]->redoMem[i];
            }
            files[selectedFile]->redoMem.clear();
            if (tempMem->startPos.Y!=tempMem->endPos.Y || tempMem->startPos.X>tempMem->endPos.X || tempMem->text.size()>0) {
                if (tempMem->startPos.Y>=tempMem->endPos.Y) {
                    if (tempMem->startPos.Y>tempMem->endPos.Y || tempMem->startPos.X>tempMem->endPos.X) {
                        irr::core::vector2di tempVec = tempMem->startPos;
                        tempMem->startPos = tempMem->endPos;
                        tempMem->endPos = tempVec;
                    }
                }

                files[selectedFile]->pushToUndoMem(tempMem);
                files[selectedFile]->tempMem = nullptr;
                tempMem = nullptr;
            }
        }

        if (focus == FOCUS::FILE) {
            if ((eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_RCONTROL)) && eventReceiver->getKeyHit(irr::KEY_KEY_Y)) {
                if (files[selectedFile]->redoMem.size()>0) {
                    files[selectedFile]->redo(keywords);
                }
            }
            if ((eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_RCONTROL)) && eventReceiver->getKeyHit(irr::KEY_KEY_Z)) {
                if (tempMem!=nullptr) {
                    if (tempMem->startPos.Y>=tempMem->endPos.Y) {
                        if (tempMem->startPos.Y>tempMem->endPos.Y || tempMem->startPos.X>tempMem->endPos.X) {
                            irr::core::vector2di tempVec = tempMem->startPos;
                            tempMem->startPos = tempMem->endPos;
                            tempMem->endPos = tempVec;
                        }
                    }

                    files[selectedFile]->pushToUndoMem(tempMem);
                    files[selectedFile]->tempMem = nullptr;
                    tempMem = nullptr;
                }
                if (files[selectedFile]->undoMem.size()>0) {
                    files[selectedFile]->undo(keywords);
                }
            }
        }

		int fileWidth = font->getDimension(files[selectedFile]->text[files[selectedFile]->longestLine]->getText()).Width+200;
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

        driver->draw2DRectangle(irr::video::SColor(255, 12, 12, 15), textBoxRect);

        driver->draw2DRectangle(irr::video::SColor(255, 25, 25, 31), irr::core::recti(textBoxRect.UpperLeftCorner.X,textBoxRect.UpperLeftCorner.Y+caretPos.Y*14-scrollPos.Y,textBoxRect.LowerRightCorner.X,textBoxRect.UpperLeftCorner.Y+caretPos.Y*14-scrollPos.Y+14), &textBoxRect);

        driver->draw2DRectangle(irr::video::SColor(255, 30, 30, 35), irr::core::recti(0, 32, lineBarWidth-3, textBoxRect.LowerRightCorner.Y));
        driver->draw2DLine(irr::core::vector2di(lineBarWidth-12, 32), irr::core::vector2di(lineBarWidth-12, textBoxRect.LowerRightCorner.Y), irr::video::SColor(255, 150, 150, 150));

        std::wstring part1 = text[caretPos.Y]->getText().substr(0,min(caretPos.X,text[caretPos.Y]->getText().size()));
        std::wstring part2 = text[caretPos.Y]->getText().substr(min(caretPos.X,text[caretPos.Y]->getText().size()));

        int caretX = font->getDimension(part1).Width;
        int caretY = 32 - fontHeight + 12 + 14 * caretPos.Y;

        int renderStart = scrollPos.Y/14;
        if (renderStart < 0) { renderStart = 0; }

        int renderEnd = (windowDims.Height-52)/14 + renderStart + 2;

        irr::core::vector2di startSelectRender;
        irr::core::vector2di endSelectRender;

        int part1size = part1.size();

        if (focus == FOCUS::FILE) {
            if (selecting<2) {
                std::wstring charQueue = eventReceiver->getCharQueue(L"",true).c_str();

                if (pasting) {
                    charQueue+=getClipboardText();
                }

                if (charQueue.size()>0) {
                    caretPos.X = min(caretPos.X,text[caretPos.Y]->getText().size());
                    irr::core::vector2di oldCaretPos = caretPos;

                    if (tempMem==nullptr) {
                        files[selectedFile]->tempMem = new File::ActionMem();
                        tempMem = files[selectedFile]->tempMem;
                        tempMem->startPos = caretPos;
                        tempMem->text = L"";
                    }

                    caretScroll = true;

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
                        } else if (part1.size()>0) {
                            tempMem->text = part1[part1.size()-1]+tempMem->text;
                            part1.pop_back();
                        } else if (text.size()>1) {
                            tempMem->text.push_back(L'\n');
                            delete text[caretPos.Y]; text.erase(text.begin()+caretPos.Y);
                            caretPos.Y--; caretPos.X = text[caretPos.Y]->getText().size();
                            part1 = text[caretPos.Y]->getText();
                        }
                    }

                    if (appending) {
                        caretPos.X = part1.size()+appendStr.size();
                        text[caretPos.Y]->setText(part1+appendStr+part2);
                        text[caretPos.Y]->formatText(keywords);
                    } else {
                        text[caretPos.Y]->setText(part1+appendStr);
                        text[caretPos.Y]->formatText(keywords);

                        int lastLine = caretPos.Y+1;
                        Line* newLine = new Line(files[selectedFile]);
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
                                newLine = new Line(files[selectedFile]);
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

                    tempMem->endPos = caretPos;
                    if (tempMem->text.size()>0) {
                        tempMem->startPos = caretPos;
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
                    charQueue=getClipboardText();
                }
                if (cutting) {
                    charQueue.clear();
                    charQueue.push_back(8);
                }

                if (charQueue.size()>0) {
                    caretScroll = true;

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
                
                    std::wstring oldStr = L"";

                    if (startSelectRender.Y < endSelectRender.Y) {
                        oldStr = text[startSelectRender.Y]->getText().substr(min(startSelectRender.X,text[startSelectRender.Y]->getText().size()))+L"\n";
                        for (int i=startSelectRender.Y+1;i<endSelectRender.Y;i++) {
                            oldStr += text[i]->getText()+L"\n";
                        }
                        oldStr+=text[endSelectRender.Y]->getText().substr(0,min(endSelectRender.X,text[endSelectRender.Y]->getText().size()));
                        for (int i=endSelectRender.Y;i>startSelectRender.Y;i--) {
                            delete text[i];
                            text.erase(text.begin()+i);
                        }
                    } else {
                        int startPos = min(startSelectRender.X,text[startSelectRender.Y]->getText().size());
                        int endPos = min(endSelectRender.X,text[endSelectRender.Y]->getText().size());
                        oldStr = text[startSelectRender.Y]->getText().substr(startPos,endPos-startPos);
                    }

                    if (appending) {
                        text[startSelectRender.Y]->setText(firstLinePart1+lastLinePart2);

                        caretPos = startSelectRender;
                        caretPos.X = firstLinePart1.size();
                    } else {
                        text[startSelectRender.Y]->setText(firstLinePart1);
                        int lastLine = startSelectRender.Y+1;
                        Line* newLine = new Line(files[selectedFile]);
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
                                newLine = new Line(files[selectedFile]);
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
                    endSelectRender = caretPos;

                    files[selectedFile]->pushToUndoMem(new File::ActionMem());
                    files[selectedFile]->undoMem[files[selectedFile]->undoMem.size()-1]->startPos = startSelectRender;
                    files[selectedFile]->undoMem[files[selectedFile]->undoMem.size()-1]->endPos = endSelectRender;
                    files[selectedFile]->undoMem[files[selectedFile]->undoMem.size()-1]->text = oldStr;

                    startSelectRender = caretPos;
                
                    selecting = 0;
                }
            }
        }

        if (renderEnd > text.size()) { renderEnd = text.size(); }

		for (int i = renderStart; i<renderEnd; i++) {
            if (selecting>=2) {
                if (i==endSelectRender.Y && i==startSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                        irr::core::recti(
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,startSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,endSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)),
                        &textBoxRect);
                } else if (i>startSelectRender.Y && i<endSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                                            irr::core::recti(
                                            irr::core::vector2di(lineBarWidth - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText()).Width + 8 - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)),
                        &textBoxRect);
                } else if (i==startSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                        irr::core::recti(
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,startSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText()).Width + 8 - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)),
                        &textBoxRect);
                } else if (i==endSelectRender.Y) {
                    driver->draw2DRectangle(irr::video::SColor(255,40,80,120),
                        irr::core::recti(
                            irr::core::vector2di(lineBarWidth - scrollPos.X,32 - fontHeight + 12 + 14 * i - scrollPos.Y),
                            irr::core::vector2di(lineBarWidth + font->getDimension(text[i]->getText().substr(0,endSelectRender.X)).Width - scrollPos.X,32 - fontHeight + 26 + 14 * i - scrollPos.Y)),
                        &textBoxRect);
                }
            }

			int lineNumW = font->getDimension(std::to_string(i+1).c_str()).Width;

            if (focus == FOCUS::FILE && caretPos.Y == i && device->getTimer()->getTime()%1000<500) {
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
        
		wchar_t tempCStr[2];
        tempCStr[1]=0;

		//vertical scroll bar

        int vScrollSpace = textBoxRect.getHeight()-20;
		int vRealScrollBarHalfHeight = vScrollSpace*vScrollSpace / (text.size() * 14) / 2;
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

        int hScrollSpace = textBoxRect.getWidth()-20;
		int hRealScrollBarHalfWidth = hScrollSpace*hScrollSpace / (fileWidth) / 2;
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

        bool forcePushUndoMem = false;

        if (eventReceiver->getMouseDown(0) && isScrolling==SCROLL::NONE && textBoxRect.isPointInside(eventReceiver->getMousePos())) {
            if (textBoxRect.isPointInside(eventReceiver->getMousePos()) || selecting>1) {
                focus = FOCUS::FILE;
                caretScroll = true;
                forcePushUndoMem = true;
                caretPos.Y = max(0,min(((int)text.size())-1,(eventReceiver->getMousePos().Y-textBoxRect.UpperLeftCorner.Y+scrollPos.Y)/14));

                if (text[caretPos.Y]->getText().size()>0) {
                    caretPos.X = max(0,min(((int)text[caretPos.Y]->getText().size()),
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

        if (focus == FOCUS::FILE) {
            if (eventReceiver->getKeyHit(irr::KEY_LEFT)) {
                caretScroll = true;
                forcePushUndoMem = true;
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
                } else if (selecting<2) {
                    selectionStart = oldCaretPos;
                    selecting = 2;
                }
            }

            if (eventReceiver->getKeyHit(irr::KEY_RIGHT)) {
                caretScroll = true;
                forcePushUndoMem = true;
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
                } else if (selecting<2) {
                    selectionStart = oldCaretPos;
                    selecting = 2;
                }
            }
            if (eventReceiver->getKeyHit(irr::KEY_UP)) {
                caretScroll = true;
                forcePushUndoMem = true;
                caretPos.Y--;
                if (caretPos.Y<0) {
                    caretPos.Y = 0;
                }

                if (!eventReceiver->getKeyDown(irr::KEY_LSHIFT) && !eventReceiver->getKeyDown(irr::KEY_RSHIFT)) {
                    selecting = 0;
                } else if (selecting<2) {
                    selectionStart = oldCaretPos;
                    selecting = 2;
                }
            }
            if (eventReceiver->getKeyHit(irr::KEY_DOWN)) {
                caretScroll = true;
                forcePushUndoMem = true;
                caretPos.Y++;
                if (caretPos.Y>text.size()-1) {
                    caretPos.Y = text.size()-1;
                }

                if (!eventReceiver->getKeyDown(irr::KEY_LSHIFT) && !eventReceiver->getKeyDown(irr::KEY_RSHIFT)) {
                    selecting = 0;
                } else if (selecting<2) {
                    selectionStart = oldCaretPos;
                    selecting = 2;
                }
            }
        }

        if (forcePushUndoMem) {
            if (tempMem!=nullptr) {
                if (tempMem->startPos.Y>=tempMem->endPos.Y) {
                    if (tempMem->startPos.Y>tempMem->endPos.Y || tempMem->startPos.X>tempMem->endPos.X) {
                        irr::core::vector2di tempVec = tempMem->startPos;
                        tempMem->startPos = tempMem->endPos;
                        tempMem->endPos = tempVec;
                    }
                }

                files[selectedFile]->pushToUndoMem(tempMem);
                files[selectedFile]->tempMem = nullptr;
                tempMem = nullptr;
            }
        }

        if (caretScroll) {
            caretX = font->getDimension(part1).Width;
            caretY = 32 - fontHeight + 12 + 14 * caretPos.Y;
            if (caretX>scrollPos.X+(int)textBoxRect.getWidth()-8) {
                scrollPos.X = caretX-(int)textBoxRect.getWidth()+8;
            } else if (caretX<scrollPos.X+8) {
                scrollPos.X = caretX-8;
            }
            scrollPos.X = max(0,min(fileWidth-(int)textBoxRect.getWidth()+100,scrollPos.X));
            if (caretY>scrollPos.Y+((int)textBoxRect.getHeight())+14) {
                scrollPos.Y = caretY-((int)textBoxRect.getHeight())-14;
            } else if (caretY<scrollPos.Y+textBoxRect.UpperLeftCorner.Y) {
                scrollPos.Y = caretY-textBoxRect.UpperLeftCorner.Y;
            }
            scrollPos.Y = max(0,min(((int)text.size())*14,scrollPos.Y));
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
    
    driver->draw2DRectangle(irr::video::SColor(255,255,0,0),irr::core::recti(-2,-2,-1,-1));
    driver->draw2DLine(irr::core::vector2di(0,32),irr::core::vector2di(windowDims.Width,32),irr::video::SColor(255,70,70,70));

    mat.MaterialType = irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    driver->setMaterial(mat);
    //new
    bool doNewFile = false;
    irr::core::recti newButtonRect(2,4,25,27);
    if (eventReceiver->getKeyHit(irr::KEY_KEY_N) && (eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_LCONTROL))) {
        doNewFile = true;
    }
    if (newButtonRect.isPointInside(eventReceiver->getMousePos())) {
        driver->draw2DRectangle(irr::video::SColor(255,60,60,60),newButtonRect);
        if (mouseHit) {
            doNewFile = true;
        }
    }
    if (doNewFile) {
        File* newFile = new File();
        newFile->path = L"";
        int num = 0;
        for (int i=0;i<files.size();i++) {
            if (files[i]->name.size()>=3 && files[i]->path.size()==0) {
                if (files[i]->name.substr(0,3)==L"New") {
                    num++;
                }
            }
        }
        newFile->name = L"New"+std::to_wstring(num)+L".bb";
        Line* newLine = new Line(newFile);
        newLine->setText(L"");
        newFile->text.push_back(newLine);
        files.push_back(newFile);
        selectedFile = files.size()-1;
    }
    driver->draw2DImage(toolbarTex,irr::core::recti(6,8,22,24),irr::core::recti(0,0,16,16),nullptr,nullptr,true);

    //load
    bool doLoadFile = false;
    irr::core::recti loadButtonRect(26,4,49,27);
    if (eventReceiver->getKeyHit(irr::KEY_KEY_O) && (eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_LCONTROL))) {
        doLoadFile = true;
    }
    if (loadButtonRect.isPointInside(eventReceiver->getMousePos())) {
        driver->draw2DRectangle(irr::video::SColor(255,60,60,60),loadButtonRect);
        if (mouseHit) {
            doLoadFile = true;
        }
    }
    if (doLoadFile) {
        OPENFILENAME ofn;
        wchar_t szFile[128];
        szFile[0] = L'\0';
        ZeroMemory( &ofn , sizeof( ofn));
        ofn.lStructSize = sizeof ( ofn );
        ofn.hwndOwner = NULL  ;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof( szFile );
        ofn.lpstrFilter = L"BlitzBasic Source Files (*.bb)\0*.bb\0All Files (*.*)\0*.*\0";
        ofn.lpstrInitialDir = NULL;
        ofn.nFilterIndex =1;
        ofn.lpstrFileTitle = NULL ;
        ofn.nMaxFileTitle = 0 ;
        ofn.Flags = OFN_PATHMUSTEXIST;
        if (GetOpenFileName( &ofn )) {
            loadFile(ofn.lpstrFile);
            selectedFile = files.size()-1;
        }
        eventReceiver->clearKeys();
        eventReceiver->clearMouse();
        mouseHit = false;
    }
    driver->draw2DImage(toolbarTex,irr::core::recti(30,8,46,24),irr::core::recti(16,0,32,16),nullptr,nullptr,true);

    //save
    bool doSaveFile = false;
    irr::core::recti saveButtonRect(50,4,73,27);
    if (eventReceiver->getKeyHit(irr::KEY_KEY_S) && (eventReceiver->getKeyDown(irr::KEY_LCONTROL) || eventReceiver->getKeyDown(irr::KEY_LCONTROL))) {
        doSaveFile = true;
    }
    if (saveButtonRect.isPointInside(eventReceiver->getMousePos())) {
        driver->draw2DRectangle(irr::video::SColor(255,60,60,60),saveButtonRect);
        if (mouseHit && selectedFile>=0 && selectedFile<files.size()) {
            doSaveFile = true;
        }
    }
    if (doSaveFile) {
        if (files[selectedFile]->path.size()==0) {
            OPENFILENAME ofn;
            wchar_t szFile[128];
            for (int i=0;i<files[selectedFile]->name.size();i++) {
                szFile[i] = files[selectedFile]->name[i];
            }
            szFile[files[selectedFile]->name.size()] = L'\0';
            ZeroMemory( &ofn , sizeof( ofn));
            ofn.lStructSize = sizeof ( ofn );
            ofn.hwndOwner = NULL  ;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof( szFile );
            ofn.lpstrFilter = L"BlitzBasic Source Files (*.bb)\0*.bb\0All Files (*.*)\0*.*\0";
            ofn.lpstrInitialDir = NULL;
            ofn.nFilterIndex =1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0 ;
            ofn.Flags = OFN_PATHMUSTEXIST;
            if (GetSaveFileName( &ofn )) {
                saveFile(files[selectedFile],ofn.lpstrFile);
            }
        } else {
            saveFile(files[selectedFile],files[selectedFile]->path+files[selectedFile]->name);
        }
        eventReceiver->clearKeys();
        eventReceiver->clearMouse();
        mouseHit = false;
    }
    driver->draw2DImage(toolbarTex,irr::core::recti(54,8,70,24),irr::core::recti(32,0,48,16),nullptr,nullptr,true);

    //save as
    irr::core::recti saveAsButtonRect(74,4,97,27);
    if (saveAsButtonRect.isPointInside(eventReceiver->getMousePos())) {
        driver->draw2DRectangle(irr::video::SColor(255,60,60,60),saveAsButtonRect);
        if (mouseHit && selectedFile>=0 && selectedFile<files.size()) {
            OPENFILENAME ofn;
            wchar_t szFile[128];
            for (int i=0;i<files[selectedFile]->name.size();i++) {
                szFile[i] = files[selectedFile]->name[i];
            }
            szFile[files[selectedFile]->name.size()] = L'\0';
            ZeroMemory( &ofn , sizeof( ofn));
            ofn.lStructSize = sizeof ( ofn );
            ofn.hwndOwner = NULL  ;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof( szFile );
            ofn.lpstrFilter = L"BlitzBasic Source Files (*.bb)\0*.bb\0All Files (*.*)\0*.*\0";
            ofn.lpstrInitialDir = NULL;
            ofn.nFilterIndex =1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0 ;
            ofn.Flags = OFN_PATHMUSTEXIST;
            if (GetSaveFileName( &ofn )) {
                saveFile(files[selectedFile],ofn.lpstrFile);
            }
            eventReceiver->clearKeys();
            eventReceiver->clearMouse();
            mouseHit = false;
        }
    }
    driver->draw2DImage(toolbarTex,irr::core::recti(78,8,94,24),irr::core::recti(48,0,64,16),nullptr,nullptr,true);

    int x = 400;
    for (int i=0;i<files.size();i++) {
        int y = 12;
        irr::video::SColor tabColor(255,50,50,50);
        if (i==selectedFile) {
            tabColor = irr::video::SColor(255,50,80,100);
            y-=4;
        }
        irr::core::recti tabRect(x-4,y-4,x+55,32);
        irr::core::recti closeRect(x+46,y-4,x+55,y+5);
        bool doClose = requestClose;
        if (closeRect.isPointInside(eventReceiver->getMousePos())) {
            if (mouseHit) {
                doClose = true;
            }
        } else if (tabRect.isPointInside(eventReceiver->getMousePos())) {
            if (i!=selectedFile) {
                tabColor = irr::video::SColor(255,70,70,70);
            }
            if (mouseHit) {
                selectedFile = i;
            }
        }

        if (doClose) {
            if (files[i]->changed) {
                std::wstring msgStr = L"Would you like to save changes to \""+files[i]->name+L"\" before closing?";
                int msgState = MessageBoxW(
                    HWnd,
                    msgStr.c_str(),
                    L"Confirm",
                    MB_YESNOCANCEL
                );

                if (msgState==IDYES) {
                    doClose = true;
                    if (files[i]->path.size()==0) {
                        OPENFILENAME ofn;
                        wchar_t szFile[128];
                        for (int j=0;j<files[i]->name.size();j++) {
                            szFile[j] = files[i]->name[j];
                        }
                        szFile[files[i]->name.size()] = L'\0';
                        ZeroMemory( &ofn , sizeof( ofn));
                        ofn.lStructSize = sizeof ( ofn );
                        ofn.hwndOwner = NULL;
                        ofn.lpstrFile = szFile;
                        ofn.nMaxFile = sizeof( szFile );
                        ofn.lpstrFilter = L"BlitzBasic Source Files (*.bb)\0*.bb\0All Files (*.*)\0*.*\0";
                        ofn.lpstrInitialDir = NULL;
                        ofn.nFilterIndex = 1;
                        ofn.lpstrFileTitle = NULL;
                        ofn.nMaxFileTitle = 0;
                        ofn.Flags = OFN_PATHMUSTEXIST;
                        if (GetSaveFileName( &ofn )) {
                            saveFile(files[i],ofn.lpstrFile);
                        }
                    } else {
                        saveFile(files[i],files[i]->path+files[i]->name);
                    }
                } else if (msgState==IDNO) {
                    doClose = true;
                } else {
                    doClose = false;
                }
            }
            if (doClose) {
                for (int j=0;j<files[i]->text.size();j++) {
                    delete files[i]->text[j];
                }
                for (int j=0;j<files[i]->undoMem.size();j++) {
                    delete files[i]->undoMem[j];
                }
                for (int j=0;j<files[i]->redoMem.size();j++) {
                    delete files[i]->redoMem[j];
                }
                delete files[i];
                files.erase(files.begin()+i);
                mouseHit = false;
                eventReceiver->clearMouse();
                eventReceiver->clearKeys();
                i--;
                continue;
            } else {
                requestClose = false;
            }
        }

        driver->draw2DRectangle(tabColor,tabRect);
        std::wstring name = files[i]->name;
        if (files[i]->changed) {
            name = L"*"+name;
        }

        smallFont->draw(name.c_str(),irr::core::recti(x+4,y,x+50,28),irr::video::SColor(255,255,255,255),false,true,&tabRect);

        if (closeRect.isPointInside(eventReceiver->getMousePos())) {
            driver->draw2DRectangle(irr::video::SColor(255,255,70,70),closeRect);
        }
        driver->draw2DImage(toolbarTex,irr::core::recti(x+47,y-3,x+54,y+4),irr::core::recti(80,0,87,7),nullptr,nullptr,true);
        x+=65;
    }

	driver->setRenderTarget(0,true,true,irr::video::SColor(255,0,255,0));
    mat.MaterialType = irr::video::EMT_SOLID;
	driver->setMaterial(mat);
	flipQuad->render();
	driver->endScene();

    if (requestClose) {
        DestroyWindow(HWnd);
    }

	//std::cout << driver->getFPS() << "\n";

	return device->run();
}

std::string Line::getTextUTF8() {
    return wCharToUtf8(text);
}

std::wstring Line::getText() {
	return text;
}

static bool isfmt( int ch,int nxt ){
	return ch==L';' || ch==L'\"' || iswalpha(ch) || isdigit(ch) || (ch==L'$' && isxdigit(nxt));
}

static bool isid( int c ){
	return iswalnum(c)||c=='_';
}

const irr::video::SColor Line::Part::colors[6] = {
	irr::video::SColor(255,200,200,200),
	irr::video::SColor(255,255,200,100),
	irr::video::SColor(255,102,187,238),
	irr::video::SColor(255,0,204,102),
	irr::video::SColor(255,200,255,200)
};

void Line::setText(std::wstring inText) {
    file->changed = true;

    text = inText;
    parts.clear();

    Part newPart; newPart.text = inText; newPart.color = Part::colors[0];
    parts.push_back(newPart);
}

void Line::setTextUTF8(std::string inText) {
    text = utf8ToWChar(inText);
    parts.clear();

    Part newPart; newPart.text = text; newPart.color = Part::colors[0];
    parts.push_back(newPart);
}

void Line::formatText(Main::Keywords& keywords) {
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

void File::recalculateLongestLine() {
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

void File::pushToUndoMem(File::ActionMem* mem) {
    undoMem.push_back(mem);
    for (int i=0;i<redoMem.size();i++) {
        delete redoMem[i];
    }
    redoMem.clear();
}

void File::undo(Main::Keywords& keywords) {
    if (undoMem.size()<=0) { return; }
    ActionMem* mem = undoMem[undoMem.size()-1];
    
    performAndReverse(mem,keywords);

    redoMem.push_back(mem);
    undoMem.pop_back();
}

void File::redo(Main::Keywords& keywords) {
    if (redoMem.size()<=0) { return; }
    ActionMem* mem = redoMem[redoMem.size()-1];

    performAndReverse(mem,keywords);

    undoMem.push_back(mem);
    redoMem.pop_back();
}

void File::performAndReverse(File::ActionMem* mem,Main::Keywords& keywords) {
    std::wstring charQueue = mem->text;
    
    std::wstring reverseText = L"";
    irr::core::vector2di reverseEndPos;

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

    std::wstring firstLinePart1 = text[mem->startPos.Y]->getText().substr(0,min(mem->startPos.X,text[mem->startPos.Y]->getText().size()))+appendStr;
    std::wstring lastLinePart2 = text[mem->endPos.Y]->getText().substr(min(mem->endPos.X,text[mem->endPos.Y]->getText().size()));

    if (mem->startPos.Y < mem->endPos.Y) {
        reverseText = text[mem->startPos.Y]->getText().substr(min(mem->startPos.X,text[mem->startPos.Y]->getText().size()))+L"\n";
        for (int i=mem->startPos.Y+1;i<mem->endPos.Y;i++) {
            reverseText += text[i]->getText()+L"\n";
        }
        reverseText += text[mem->endPos.Y]->getText().substr(0,min(mem->endPos.X,text[mem->endPos.Y]->getText().size()));
        for (int i=mem->endPos.Y;i>mem->startPos.Y;i--) {
            delete text[i];
            text.erase(text.begin()+i);
        }
    } else {
        int startPos = min(mem->startPos.X,text[mem->startPos.Y]->getText().size());
        int endPos = min(mem->endPos.X,text[mem->endPos.Y]->getText().size());
        reverseText = text[mem->startPos.Y]->getText().substr(startPos,endPos-startPos);
    }

    if (appending) {
        text[mem->startPos.Y]->setText(firstLinePart1+lastLinePart2);
        reverseEndPos.Y = mem->startPos.Y;
        reverseEndPos.X = firstLinePart1.size();
    } else {
        text[mem->startPos.Y]->setText(firstLinePart1);
        int lastLine = mem->startPos.Y+1;
        Line* newLine = new Line(this);
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
                newLine = new Line(this);
                lastLine++;
                lineText = L"";
            } else {
                lineText.push_back(remainStr[i]);
            }
        }
        newLine->setText(lineText+lastLinePart2);
        reverseEndPos.Y = lastLine;
        reverseEndPos.X = lineText.size();
        newLine->formatText(keywords);
        text.insert(text.begin()+lastLine,newLine);
    }
    text[mem->startPos.Y]->formatText(keywords);
    recalculateLongestLine();

    mem->text = reverseText;
    mem->endPos = reverseEndPos;

    caretPos = mem->endPos;

    selecting = 0;
}

void Main::saveFile(File* f,std::wstring absoluteFilename) {
    irr::io::IFileSystem* fs = device->getFileSystem();

    std::ofstream file;
    file.open(absoluteFilename.c_str(),std::ofstream::out|std::ofstream::binary);

    std::wstring name = L"File.bb";
    std::wstring path = irr::core::stringw(fs->getWorkingDirectory()+L"/").c_str();
    for (int i=absoluteFilename.size();i>=0;i--) {
        if (absoluteFilename[i]=='/' || absoluteFilename[i]=='\\') {
            path = absoluteFilename.substr(0,i+1);
            name = absoluteFilename.substr(i+1);
            break;
        }
    }
    f->name = name; f->path = path;

    std::string buffer = "";
    for (int i=0;i<f->text.size();i++) {
        buffer+=f->text[i]->getTextUTF8()+"\n";
    }
    file.write(buffer.data(),buffer.size());
    file.close();

    f->changed = false;
}

File* Main::loadFile(std::wstring name) {
    irr::io::IFileSystem* fs = device->getFileSystem();
    
    std::wstring path = irr::core::stringw(fs->getWorkingDirectory()+L"/").c_str();
    for (int i=name.size();i>=0;i--) {
        if (name[i]=='/' || name[i]=='\\') {
            path = name.substr(0,i+1);
            name = name.substr(i+1);
            break;
        }
    }

    for (int i=0;i<files.size();i++) {
        if (path==files[i]->path && name==files[i]->name) {
            return files[i];
        }
    }

	File* newFile = new File();
	newFile->caretPos = irr::core::vector2di(0,0);
	newFile->scrollPos = irr::core::vector2di(0,0);
	newFile->changed = false;
	newFile->name = name;
    newFile->path = path;

	std::string data = "";
	char buffer[1025];

	std::ifstream file;
    file.open((path+name).c_str(),std::ifstream::in|std::ifstream::binary);
	
	while (!file.eof()) {
        file.read(buffer, 1024);
        int bytesRead = file.gcount();
		buffer[bytesRead] = 0;
		data+=buffer;
	}

    file.close();

	std::string currLine = "";
	for (int i=0; i<data.size(); i++) {
		if (data[i]=='\r') {
			//skip
		} else if (data[i]=='\n') {
            Line* newLine = new Line(newFile);
            newLine->setText(utf8ToWChar(currLine));
            newLine->formatText(keywords);
            newFile->text.push_back(newLine);
			//std::cout<<currLine.c_str()<<"\n";
			currLine = "";
		} else if (data[i]=='\t') {
            int tabSize = 4-(currLine.size()%4);
            for (int i=0;i<tabSize;i++) {
                currLine+=" ";
            }
		} else {
			currLine += data[i];
		}
	}
    if (currLine.size()>0 || newFile->text.size()==0) {
        Line* newLine = new Line(newFile);
        newLine->setText(utf8ToWChar(currLine));
        newLine->formatText(keywords);
        newFile->text.push_back(newLine);
    }

	newFile->recalculateLongestLine();

    newFile->changed = false;
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

static std::string wCharToUtf8(std::wstring wCharStr) {
    std::string retVal = "";
    for (int i=0;i<wCharStr.size();i++) {
        if (wCharStr[i]<128) { //regular ASCII: just copy the char over
            retVal.push_back((char)wCharStr[i]);
        } else { //unicode
            int bitCount = 7;
            int character = wCharStr[i];
            while ((character>>bitCount)>0) {
                bitCount++;
            }
            std::string codepoints = "";
            char currChar = 0;
            for (int j=0;j<bitCount;j++) {
                currChar|=(character&(1<<j))>>((j/6)*6);
                if (j%6==5 || j==bitCount-1) {
                    codepoints.insert(codepoints.begin(),currChar);
                    currChar = 0;
                }
            }

            char header = 0;
            for (int j=0;j<codepoints.size();j++) {
                header|=1<<(7-j);
            }
            char headerSpaceCheck = header|(header>>1);

            if ((codepoints[0]&headerSpaceCheck)==0) {
                codepoints[0]|=header;
            } else {
                codepoints[0]|=0x80; codepoints[0]&=0xbf;
                codepoints.insert(codepoints.begin(),header);
            }

            for (int j=1;j<codepoints.size();j++) {
                codepoints[j]|=0x80; codepoints[j]&=0xbf;
            }

            retVal+=codepoints;
        }
    }
    return retVal;
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
                int character = 0x00;

                while ((firstByte&(1<<(7-sBitCount)))!=0) {
                    sBits |= (1<<(7-sBitCount));
                    sBitCount++;
                    if (sBitCount>7) {
                        break; //all 1's???
                    }
                }

                if (sBitCount<2 || sBitCount>7) {
                    continue;
                }

                character |= firstByte&((~sBits)&0xff);

                bool goodChar = true;
                for (int j=1;j<sBitCount;j++) {
                    int nextChar = utf8Str[i+j];

                    if ((nextChar&0x80)==0x80) { //next codepoint is good
                        character = (character<<6)|(nextChar&0x3f);
                    } else {
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

    std::wstring str = buffer;
    for (int i=0;i<str.size();i++) {
        if (str[i]==13) {
            if (i<str.size()-1) {
                if (str[i+1]==10) {
                    str.erase(str.begin()+i+1);
                }
            }
            str[i]=10;
        }
    }

    return str;
}

static void setClipboardText(std::wstring text) {
    if (!OpenClipboard(NULL))
        return;

    wchar_t * buffer = 0;
    size_t len = (text.size()+1)*sizeof(wchar_t);

    HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE,len);
    memcpy(GlobalLock(hMem), text.data(), len);
    GlobalUnlock(hMem);
    SetClipboardData( CF_UNICODETEXT,hMem );
    CloseClipboard();
}
