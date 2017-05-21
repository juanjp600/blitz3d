
#ifndef LISTENER_H
#define LISTENER_H

#include "object.h"

class Listener : public Object{
public:
	Listener( float roll=1.f,float dopp=1.f,float dist=1.f );
	Listener( const Listener &t );
	~Listener();

	//Entity interface
	Entity *clone(){ return d_new Listener( *this ); }
	Listener *getListener(){ return this; }

	//Listener interface
	void renderListener();
    void set(float roll=1.f,float dopp=1.f,float dist=1.f);

private:
};

#endif