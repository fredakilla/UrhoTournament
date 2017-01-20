#ifndef NINJA_H
#define NINJA_H

#pragma once

#include "GameObject.h"
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Input/Controls.h>

namespace Urho3D
{
	class Quaternion;
}

class AIController;

const int LAYER_MOVE = 0;
const int LAYER_ATTACK = 1;

const float ninjaMoveForce = 25.0f;
const float ninjaAirMoveForce = 1.0f;
const float ninjaDampingForce = 5.0f;
const float ninjaJumpForce = 450.0f;
const Vector3 ninjaThrowVelocity(0.0f, 4.25f, 20.0f);
const Vector3 ninjaThrowPosition(0.0f, 0.2f, 1.0f);
const float ninjaThrowDelay = 0.1f;
const float ninjaCorpseDuration = 3.0f;
const int ninjaPoints = 250;

using namespace Urho3D;


class Ninja : public GameObject
{
    URHO3D_OBJECT(Ninja, GameObject)
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    Ninja( Context* context );
    ~Ninja();
	/// Register object factory.
    static void RegisterObject(Context* context);

	 Quaternion GetAim();

	 void SetControls(const Controls& newControls);
	 void DeathUpdate(float timeStep);

	 virtual void DelayedStart();

	 virtual void FixedUpdate(float timeStep);

	 virtual bool Heal(int amount);

	 bool  okToJump;
	 bool  smoke;
	 float inAirTime;
	 float onGroundTime;
	 float throwTime;
	 float deathTime;
	 float deathDir;
	 float dirChangeTime;
	 float aimX;
	 float aimY;
	 /// Movement controls. Assigned by the main program each frame.
	 Controls controls;
	 Controls prevControls;

	 SharedPtr<AIController> controller;
protected:


private:


};

#endif // NINJA_H
