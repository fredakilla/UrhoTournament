
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

#include "SnowCrate.h"

#include <Urho3D/DebugNew.h>



SnowCrate::SnowCrate(Context* context) : GameObject(context)
{
    // Initialize variables to sensible defaults
	health = maxHealth = snowcrateHealth;
}


SnowCrate::~SnowCrate()
{
}

void SnowCrate::Start()
{
	
    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(SnowCrate, HandleNodeCollision));
}


void SnowCrate::RegisterObject(Context* context)
{
	context->RegisterFactory<SnowCrate>();
}

void SnowCrate::FixedUpdate(float timeStep)
{
	if (health <= 0)
	{
// 		{
// 			VariantMap &eventData = GetEventDataMap();
// 			eventData[GameObjectDie::P_NODE] = node_;
// 			eventData[GameObjectDie::P_GAMEOBJECT] = this;
// 			SendEvent(GE_DIE, eventData);
// 		}

		SpawnParticleEffect(node_, node_->GetPosition(), "Particle/SnowExplosionBig.xml", 2);
		SpawnObject(node_, node_->GetPosition(), Quaternion(), "Potion");

		{
			VariantMap &eventData = GetEventDataMap();
			eventData[GameObjectPoints::P_POINTS] = snowcratePoints;
			eventData[GameObjectPoints::P_RECIVER] = lastDamageCreatorID;
			eventData[GameObjectPoints::P_DAMAGESIDE] = lastDamageSide;
			SendEvent(GE_POINTS, eventData);
		}
		node_->Remove();
	}
}








