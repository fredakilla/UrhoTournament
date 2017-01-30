#ifndef SNOWBALL_H
#define SNOWBALL_H

#pragma once

#include "GameObject.h"

using namespace Urho3D;

const float snowballMinHitSpeed = 1.0f;
const float snowballDampingForce = 20.0f;
const float snowballDuration = 5.0f;
const float snowballGroundHitDuration = 1.0f;
const float snowballObjectHitDuration = 0.0f;
const int snowballDamage = 1;

class SnowBall : public GameObject
{
    URHO3D_OBJECT(SnowBall, GameObject);
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    SnowBall(Context* context);
    ~SnowBall();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start();

    virtual void ObjectCollision(GameObject* otherObject, VariantMap& eventData);

    virtual void FixedUpdate(float timeStep);

    virtual void WorldCollision(VariantMap& eventData);

    void DelayedStart();


protected:
    int hitDamage;

private:


};

#endif // SNOWBALL_H
