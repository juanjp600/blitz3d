#ifndef GXSCENE_H
#define GXSCENE_H

class gxScene {
public:
	enum{
		FX_FULLBRIGHT=	0x0001,
		FX_VERTEXCOLOR=	0x0002,
		FX_FLATSHADED=	0x0004,
		FX_NOFOG=		0x0008,
		FX_DOUBLESIDED=	0x0010,
		FX_VERTEXALPHA=	0x0020,

		FX_ALPHATEST=	0x2000,
		FX_CONDLIGHT=	0x4000,
		FX_EMISSIVE=	0x8000
	};
	enum{
		BLEND_REPLACE=	0,
		BLEND_ALPHA=	1,
		BLEND_MULTIPLY=	2,
		BLEND_ADD=		3,
		BLEND_DOT3=		4,
		BLEND_MULTIPLY2=5,
		BLEND_BUMPENVMAP=6,
	};
	enum{
		ZMODE_NORMAL=	0,
		ZMODE_DISABLE=	1,
		ZMODE_CMPONLY=	2
	};
	enum{
		FOG_NONE=		0,
		FOG_LINEAR=		1
	};
	enum{
		TEX_COORDS2=	0x0001
	};	

	struct Matrix{
		float elements[4][3];
	};

	static const int MAX_TEXTURES=8;
	struct RenderState{
		float color[3];
		float shininess,alpha;
		int blend,fx;
		struct TexState{
			class gxCanvas *canvas;
			const Matrix *matrix;
			int blend,flags;
			float bumpEnvMat[2][2];
			float bumpEnvScale;
			float bumpEnvOffset;
		}tex_states[MAX_TEXTURES];
	};

	void setAmbient( const float rgb[3] );
	void setAmbient2( const float rgb[3] );
	void setFogColor( const float rgb[3] );
	void setFogRange( float nr,float fr );
	void setFogMode( int mode );
	void setZMode( int mode );
	void setViewport( int x,int y,int w,int h );
	void setOrthoProj( float nr,float fr,float nr_w,float nr_h );
	void setPerspProj( float nr,float fr,float nr_w,float nr_h );
	void setViewMatrix( const Matrix *matrix );
	void setWorldMatrix( const Matrix *matrix );
	void setRenderState( const RenderState &state );

	//rendering
	//bool begin( const std::vector<gxLight*> &lights );
	void clear( const float rgb[3],float alpha,float z,bool clear_argb,bool clear_z );
	//void render( gxMesh *mesh,int first_vert,int vert_cnt,int first_tri,int tri_cnt );
	void end();

	void setZMode();
	void setAmbient();
	void setFogMode();
};

#endif
