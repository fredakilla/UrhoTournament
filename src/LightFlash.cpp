
#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Light.h>
#include "LightFlash.h"



#include <Urho3D/DebugNew.h>



LightFlash::LightFlash(Context* context) : GameObject(context)
{
    // Initialize variables to sensible defaults
    duration = 5.0;
}


LightFlash::~LightFlash()
{
}

void LightFlash::RegisterObject(Context* context)
{
    context->RegisterFactory<LightFlash>();
}

void LightFlash::FixedUpdate(float timeStep)
{



    // Call superclass to handle lifetime
    GameObject::FixedUpdate(timeStep);

    Light* light = node_->GetComponent<Light>();
    if (light)
    {
        light->SetBrightness(light->GetBrightness() * Max(1.0f - timeStep * 10.0f, 0.0f));
    }
}








