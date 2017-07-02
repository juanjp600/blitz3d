#include "gxcanvas.h"
#include "gxgraphics.h"
#include "gxfont.h"

gxCanvas::gxCanvas(gxGraphics* gfx,int inW, int inH, int inFlags) {
	w = inW; h = inH; flags = inFlags;
	if (flags&CANVAS_TEX_VIDMEM) {
		irrTex = gfx->irrDriver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(w,h),"rtcanvas",irr::video::ECF_A8R8G8B8);
		renderTex = irrTex;
		isRenderTarget = true;
	} else {
		irrTex = gfx->irrDriver->addTexture(irr::core::dimension2d<irr::u32>(w,h),"canvas",irr::video::ECF_A8R8G8B8);
		renderTex = 0;
		isRenderTarget = false;
	}
	graphics = gfx;
}

gxCanvas::gxCanvas(gxGraphics* gfx,const std::string &filename, int inFlags) {
	irr::video::IImage* img = gfx->irrDriver->createImageFromFile(filename.c_str());
	flags = inFlags;
	if (flags&CANVAS_TEX_VIDMEM) {
		irrTex = gfx->irrDriver->addRenderTargetTexture(img->getDimension(),"rtcanvas",img->getColorFormat());
		renderTex = irrTex;

		//copy img to renderTex
		void* ptr = renderTex->lock();
		irr::video::ECOLOR_FORMAT format = renderTex->getColorFormat();
		irr::core::dimension2du size = renderTex->getOriginalSize();
		irr::video::IImage* holder = graphics->irrDriver->createImageFromData(format, size, ptr, true, false);

		img->copyTo(holder);
		
		holder->drop(); renderTex->unlock();

		isRenderTarget = true;
	} else {
		printf(filename.c_str());
		printf("\n");
		irr::video::IImage* tImage = gfx->irrDriver->createImage(img->getColorFormat(),img->getDimension());
		irr::u8* dest = (irr::u8*)tImage->lock();
		irr::u8* src = (irr::u8*)img->lock();
		memcpy(dest,src,tImage->getPitch()*img->getDimension().Height);
		irr::video::ITexture* tex = gfx->irrDriver->addTexture(filename.c_str(),tImage);

		irrTex = tex;
		renderTex = 0;
		isRenderTarget = false;

		tImage->unlock(); img->unlock();
		tImage->drop();
	}

	img->drop();

	w = irrTex->getOriginalSize().Width; h = irrTex->getOriginalSize().Height;
	graphics = gfx;
}

void gxCanvas::reset() {
	graphics->irrDriver->removeTexture(irrTex);
	if (isRenderTarget) {
		irrTex = graphics->irrDriver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(w,h),"rtcanvas",irr::video::ECF_A8R8G8B8);
		renderTex = irrTex;
	} else {
		if (renderTex) graphics->irrDriver->removeTexture(renderTex);
		irrTex = graphics->irrDriver->addTexture(irr::core::dimension2d<irr::u32>(w,h),"canvas",irr::video::ECF_A8R8G8B8);
		renderTex = 0;
	}
}

gxCanvas::~gxCanvas() {
	graphics->irrDriver->removeTexture(irrTex);
	if (!isRenderTarget && !!renderTex) graphics->irrDriver->removeTexture(renderTex);
}

irr::video::ITexture* gxCanvas::getIrrTex() {
	return irrTex;
}

irr::video::ITexture* gxCanvas::getRenderTex() {
	if (!isRenderTarget && !renderTex) {
		renderTex = graphics->irrDriver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(w,h),"temprtcanvas",irrTex->getColorFormat());

		//copy irrTex to renderTex
		irr::video::ITexture* destTex = renderTex;
		irr::video::ITexture* srcTex = irrTex;

		irr::u8* dest = (irr::u8*)destTex->lock();
		irr::u8* src = (irr::u8*)srcTex->lock();
		
		int srcP=0; int destP=0;
		for (int i=0;i<h;i++) {
			int min = srcTex->getPitch();
			if (destTex->getPitch()<min) min = destTex->getPitch();
			memcpy(dest+destP,src+srcP,srcTex->getPitch());
			srcP+=srcTex->getPitch();
			destP+=destTex->getPitch();
		}

		destTex->unlock(); srcTex->unlock();
	}
	return renderTex;
}

void gxCanvas::cleanupRenderTex() {
	if (!isRenderTarget && !!renderTex) {
		//copy renderTex to irrTex
		irr::video::ITexture* destTex = irrTex;
		irr::video::ITexture* srcTex = renderTex;

		irr::u8* dest = (irr::u8*)destTex->lock();
		irr::u8* src = (irr::u8*)srcTex->lock();
		
		int srcP=0; int destP=0;
		for (int i=0;i<h;i++) {
			int min = srcTex->getPitch();
			if (destTex->getPitch()<min) min = destTex->getPitch();
			memcpy(dest+destP,src+srcP,srcTex->getPitch());
			srcP+=srcTex->getPitch();
			destP+=destTex->getPitch();
		}

		destTex->unlock(); srcTex->unlock();

		graphics->irrDriver->removeTexture(renderTex); renderTex = 0;
	}
}

//MANIPULATORS
void gxCanvas::resize(int inW, int inH) {
	if (isRenderTarget) {
		printf("Can't resize render target!\n");
	} else if (!!renderTex) {
		printf("Can't resize target canvas!\n");
	} else {
		irr::core::stringc name = irrTex->getName();
		irr::video::ECOLOR_FORMAT format = irrTex->getColorFormat();

		renderTex = graphics->irrDriver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(inW,inH),"temprtcanvas",format);
		graphics->resizeCanvas(this,inW,inH);
		graphics->irrDriver->removeTexture(irrTex); //remove old texture
		w = inW; h = inH;
		
		irr::video::IImage* img = graphics->irrDriver->createImage(format,irr::core::dimension2d<irr::u32>(inW,inH));
		irr::u8* dest = (irr::u8*)img->lock();
		irr::u8* src = (irr::u8*)renderTex->lock();
		//printf((irr::core::stringc("PITCH: ")+irr::core::stringc(renderTex->getPitch())+" "+irr::core::stringc(img->getPitch())+"\n").c_str());
		int srcP=0; int destP=0;
		for (int i=0;i<inH;i++) {
			memcpy(dest+destP,src+srcP,img->getPitch());
			srcP+=renderTex->getPitch();
			destP+=img->getPitch();
		}
		img->unlock(); renderTex->unlock();
		irrTex = graphics->irrDriver->addTexture(name,img); //create new texture
		img->drop();
		graphics->irrDriver->removeTexture(renderTex); renderTex = 0;
	}
}

void gxCanvas::setFont(gxFont *inFont) {
	font = inFont;
}
void gxCanvas::setMask(unsigned argb) {

}
void gxCanvas::setColor(unsigned argb) {
	color = irr::video::SColor(255,(argb>>16)&255,(argb>>8)&255,argb&255);
}
void gxCanvas::setClsColor(unsigned argb) {

}
void gxCanvas::setOrigin(int x, int y) {

}
void gxCanvas::setHandle(int x, int y) {

}
void gxCanvas::setViewport(int x, int y, int w, int h) {

}

void gxCanvas::cls() {

}
void gxCanvas::plot(int x, int y) {

}
void gxCanvas::line(int x, int y, int x2, int y2) {
	graphics->setDefaultMaterial();
	graphics->irrDriver->draw2DLine(irr::core::vector2di(x,y),irr::core::vector2di(x2,y2),color);
}
void gxCanvas::rect(int x, int y, int w, int h, bool solid) {
	graphics->setDefaultMaterial();
	if (solid) {
		graphics->irrDriver->draw2DRectangle(color,irr::core::recti(x,y,x+w,y+h));
	} else {
		graphics->irrDriver->draw2DLine(irr::core::vector2di(x,y),irr::core::vector2di(x+w,y),color);
		graphics->irrDriver->draw2DLine(irr::core::vector2di(x,y),irr::core::vector2di(x,y+h),color);
		graphics->irrDriver->draw2DLine(irr::core::vector2di(x+w,y),irr::core::vector2di(x+w,y+h),color);
		graphics->irrDriver->draw2DLine(irr::core::vector2di(x,y+h),irr::core::vector2di(x+w,y+h),color);
	}
}
void gxCanvas::oval(int x, int y, int w, int h, bool solid) {

}
void gxCanvas::text(int x, int y, const std::string &t) {
	graphics->setDefaultMaterial();
	font->render(this,color.getAlpha()<<24|color.getRed()<<16|color.getGreen()<<8|color.getBlue(),x,y,t);
}
void gxCanvas::blit(int x, int y, int w, int h, gxCanvas *src, int src_x, int src_y, int src_w, int src_h, bool solid) {
	graphics->setDefaultMaterial();
	
	graphics->irrDriver->draw2DImage(src->getIrrTex(),irr::core::recti(x,y,x+w,y+h),
		irr::core::recti(src_x,src_y,src_x+src_w,src_y+src_h));
}

bool gxCanvas::collide(int x, int y, const gxCanvas *src, int src_x, int src_y, bool solid)const {
	return false;
}
bool gxCanvas::rect_collide(int x, int y, int rect_x, int rect_y, int rect_w, int rect_h, bool solid)const {
	return false;
}

void gxCanvas::setPixel(int x, int y, unsigned argb) {
	
}
void gxCanvas::copyPixel(int x, int y, gxCanvas *src, int src_x, int src_y) {

}
unsigned gxCanvas::getPixel(int x, int y)const {
	return 0;
}

//ACCESSORS
int gxCanvas::getWidth()const {
	return w;
}
int gxCanvas::getHeight()const {
	return h;
}
int gxCanvas::getDepth()const {
	return 32;
}
void gxCanvas::getOrigin(int *x, int *y)const {
	
}
void gxCanvas::getHandle(int *x, int *y)const {

}
void gxCanvas::getViewport(int *x, int *y, int *w, int *h)const {

}
unsigned gxCanvas::getMask()const {
	return 0;
}
unsigned gxCanvas::getColor()const {
	return color.color;
}
unsigned gxCanvas::getClsColor()const {
	return 0;
}
