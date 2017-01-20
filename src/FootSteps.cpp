#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/DrawableEvents.h>

#include "FootSteps.h"

#include <Urho3D/DebugNew.h>





void FootSteps::RegisterObject(Context* context)
{
    context->RegisterFactory<FootSteps>();
}

FootSteps::FootSteps(Context* context) : GameObject(context)
{
    // Initialize variables to sensible defaults

}


FootSteps::~FootSteps()
{
}

void FootSteps::Start()
{
    // Subscribe to animation triggers, which are sent by the AnimatedModel's node (same as our node)
    SubscribeToEvent(node_, E_ANIMATIONTRIGGER, URHO3D_HANDLER(FootSteps, HandleAnimationTrigger));
}


void FootSteps::HandleAnimationTrigger(StringHash eventType, VariantMap& eventData)
{
    using namespace AnimationTrigger;
    AnimatedModel* model = node_->GetComponent<AnimatedModel>();
    if (model)
    {
        AnimationState* state = model->GetAnimationState(eventData[P_NAME].GetString());

        if (state == NULL)
            return;

        // If the animation is blended with sufficient weight, instantiate a local particle effect for the footstep.
        // The trigger data (string) tells the bone scenenode to use. Note: called on both client and server
        if (state->GetWeight() > 0.5f)
        {
            Node* bone = node_->GetChild(eventData[P_DATA].GetString(), true);

            if (bone != NULL )
                SpawnParticleEffect(bone, node_->GetWorldPosition(), "Particle/SnowExplosionFade.xml", 1, LOCAL);
        }
    }

}








