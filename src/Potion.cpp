#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Core/Context.h>

#include "Potion.h"

#include <Urho3D/DebugNew.h>

void Potion::RegisterObject(Context* context)
{
    context->RegisterFactory<Potion>();
}

Potion::Potion(Context* context) : GameObject(context)
{
    // Initialize variables to sensible defaults
    healAmount = potionHealAmount;
}

Potion::~Potion()
{
}

void Potion::Start()
{
    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(Potion, HandleNodeCollision));
}

void Potion::ObjectCollision(GameObject* otherObject, VariantMap& eventData)
{
    if (healAmount > 0)
    {
        if (otherObject->Heal(healAmount))
        {
            // Could also remove the potion directly, but this way it gets removed on next update
            healAmount = 0;
            duration = 0.0f;
        }
    }
}
