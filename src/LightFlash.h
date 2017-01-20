#ifndef LIGHTFLASH_H
#define LIGHTFLASH_H

#pragma once

#include "GameObject.h"

using namespace Urho3D;

class LightFlash : public GameObject
{
    URHO3D_OBJECT(LightFlash, GameObject)
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
	LightFlash(Context* context);
	~LightFlash();
	/// Register object factory.
	static void RegisterObject(Context* context);

	virtual void FixedUpdate(float timeStep);
	
protected:


private:
	
};

#endif // LIGHTFLASH_H
