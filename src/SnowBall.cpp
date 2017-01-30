#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>

#include "SnowBall.h"

#include <Urho3D/DebugNew.h>


#include "MagicParticleEffect.h"
#include "MagicParticleEmitter.h"


SnowBall::SnowBall(Context* context) : GameObject(context)
{
    // Initialize variables to sensible defaults
    duration = snowballDuration;
    hitDamage = snowballDamage;



}


SnowBall::~SnowBall()
{
}

void SnowBall::Start()
{
    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(SnowBall, HandleNodeCollision));


}


void SnowBall::DelayedStart()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    MagicParticleEffect* _magicEffects = cache->GetResource<MagicParticleEffect>("MagicParticles/particles3d/fire.ptc");
    MagicParticleEmitter* em = node_->CreateComponent<MagicParticleEmitter>();
    em->SetEffect(_magicEffects, 3);             // use emitter in file at specified index
    em->SetOverrideEmitterRotation(true);                   // override any emitter built-in rotation, and prefer urho node based rotation.
    em->SetParticlesMoveWithEmitter(false);                 // new emitted particles will be independents from node movements
    em->SetParticlesRotateWithEmitter(false);               // new emitted particles will be independents from node rotations
    em->SetEmitterPosition(Vector3(0,0,0));                 // set initial emitter position (offset from node pos)
    //em->SetEmitterPosition();
    node_->SetScale(0.5);

}

void SnowBall::ObjectCollision(GameObject* otherObject, VariantMap& eventData)
{
    if (hitDamage > 0)
    {
        RigidBody* body = node_->GetComponent<RigidBody>();
        float l = body->GetLinearVelocity().Length();
        if ((body->GetLinearVelocity().Length() >= snowballMinHitSpeed))
        {
            if (side != otherObject->GetSide())
            {
                otherObject->Damage(this, hitDamage);
                // Create a temporary node for the hit sound
                SpawnSound(node_, node_->GetPosition(), "Sounds/PlayerFistHit.wav", 0.2);
            }
            hitDamage = 0;
        }
    }
    if (duration > snowballObjectHitDuration)
        duration = snowballObjectHitDuration;
}

void SnowBall::RegisterObject(Context* context)
{
    context->RegisterFactory<SnowBall>();
}

void SnowBall::FixedUpdate(float timeStep)
{
    // Apply damping when rolling on the ground, or near disappearing
    RigidBody* body = node_->GetComponent<RigidBody>();
    if ((onGround) || (duration < snowballGroundHitDuration))
    {
        Vector3 vel = body->GetLinearVelocity();
        body->ApplyForce(Vector3(-snowballDampingForce * vel.x_, 0, -snowballDampingForce * vel.z_));
    }

    // Disappear when duration expired
    if (duration >= 0)
    {
        duration -= timeStep;
        if (duration <= 0)
        {
            SpawnParticleEffect(node_, node_->GetPosition(), "Particle/SnowExplosion.xml", 1);
            node_->Remove();
        }
    }
}

void SnowBall::WorldCollision(VariantMap& eventData)
{
    GameObject::WorldCollision(eventData);

    // If hit the ground, disappear after a short while
    if (duration > snowballGroundHitDuration)
        duration = snowballGroundHitDuration;
}








