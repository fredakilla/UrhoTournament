#ifndef FOOTSTEP_H
#define FOOTSTEP_H

#pragma once

#include "GameObject.h"

using namespace Urho3D;


class FootSteps : public GameObject
{
    URHO3D_OBJECT(FootSteps, GameObject);
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    FootSteps(Context* context);
    ~FootSteps();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start();

    virtual void HandleAnimationTrigger(StringHash eventType, VariantMap& eventData);
protected:


private:


};

#endif // FOOTSTEP_H

