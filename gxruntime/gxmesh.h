
#ifndef GXMESH_H
#define GXMESH_H

#include "std.h"

class gxGraphics;

class gxMesh{
public:

	//gxMesh( gxGraphics *graphics,IDirect3DVertexBuffer7 *verts,WORD *indicies,int max_verts,int max_tris );
	~gxMesh();

	int maxVerts()const{ return max_verts; }
	int maxTris()const{ return max_tris; }

	bool dirty()const{ return mesh_dirty; }

	void render( int first_vert,int vert_cnt,int first_tri,int tri_cnt );

	void backup();
	void restore();

private:
	struct gxVertex{
		float coords[3];
		float normal[3];
		unsigned argb;
		float tex_coords[4];
	};

	int *tri_indices;

	int max_verts,max_tris;
	bool mesh_dirty;

	/***** GX INTERFACE *****/
public:
	bool lock( bool all );
	void unlock();

	//VERY NAUGHTY!!!!!
	void setVertex( int n,const void *v ){
		
	}
	void setVertex( int n,const float coords[3],const float normal[3],const float tex_coords[2][2] ){
		
	}
	void setVertex( int n,const float coords[3],const float normal[3],unsigned argb,const float tex_coords[2][2] ){
		
	}
	void setTriangle( int n,int v0,int v1,int v2 ){
		tri_indices[n*3]=v0;
		tri_indices[n*3+1]=v1;
		tri_indices[n*3+2]=v2;
	}
};

#endif