#ifndef SNOWCRATE_H
#define SNOWCRATE_H

#pragma once

#include "GameObject.h"

using namespace Urho3D;

const int snowcrateHealth = 5;
const int snowcratePoints = 250;

class SnowCrate : public GameObject
{
    URHO3D_OBJECT(SnowCrate, GameObject);
public:

    SnowCrate(Context* context);
    ~SnowCrate();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start();

    virtual void FixedUpdate(float timeStep);



protected:


private:


};

#endif // SNOWCRATE_H
