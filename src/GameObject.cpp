#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/AudioEvents.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Scene/Serializable.h>
#include <Urho3D/IO/Deserializer.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Graphics/ParticleEffect.h>

#include "GameObject.h"

#include <Urho3D/DebugNew.h>

const float mouseSensitivity = 0.125f;
const float touchSensitivity = 2.0f;
const float joySensitivity = 0.5f;
const float joyMoveDeadZone = 0.333f;
const float joyLookDeadZone = 0.05f;
const float cameraMinDist = 0.25f;
const float cameraMaxDist = 5.0f;
const float cameraSafetyDist = 0.3f;
const int initialMaxEnemies = 5;
const int finalMaxEnemies = 25;
const int maxPowerups = 5;
const int incrementEach = 10;
const int playerHealth = 20;
const float enemySpawnRate = 1.0f;
const float powerupSpawnRate = 15.0f;
const float spawnAreaSize = 5.0f;

void GameObject::RegisterObject(Context* context)
{
	context->RegisterFactory<GameObject>();
}

GameObject::GameObject(Context* context) : LogicComponent(context)
{
	// Initialize variables to sensible defaults
	onGround = false;
	isSliding = false;
	duration = -1; // Infinite
	health = 0;
	maxHealth = 0;
	side = SIDE_NEUTRAL;
	lastDamageSide = SIDE_NEUTRAL;
	lastDamageCreatorID = 0;
	creatorID = 0;
}

GameObject::~GameObject()
{
}

void GameObject::FixedUpdate(float timeStep)
{
	// Disappear when duration expired
	if (duration >= 0)
	{
		duration -= timeStep;
		if (duration <= 0)
			node_->Remove();
	}
}

bool GameObject::Damage(GameObject* origin, int amount)
{
	if ((origin->side == side) || (health == 0))
		return false;

	lastDamageSide = origin->side;
	lastDamageCreatorID = origin->creatorID;
	health -= amount;
	if (health < 0)
		health = 0;
	return true;
}

bool GameObject::Heal(int amount)
{
	// By default do not heal
	return false;
}

void GameObject::PlaySound(const String& soundName)
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Create the sound channel
    SoundSource3D* soundNode = node_->CreateComponent<SoundSource3D>();
	Sound* sound = cache->GetResource<Sound>(soundName);

    // Subscribe to sound finished for cleaning up the source
    SubscribeToEvent(soundNode, E_SOUNDFINISHED, URHO3D_HANDLER(GameObject, HandleSoundFinished)); //"SoundFinished", "HandleSoundFinished");

    soundNode->SetDistanceAttenuation(2, 50, 1);
    soundNode->Play(sound);
    //soundNode->SetAutoRemoveMode(REMOVE_NODE);
}

void GameObject::HandleSoundFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace SoundFinished;

    Node* soundNode = static_cast<Node*>(eventData[P_NODE].GetPtr());
    if (soundNode)
        soundNode->Remove();
}

void GameObject::HandleNodeCollision(StringHash eventType, VariantMap& eventData)
{
	using namespace NodeCollision;

	// Get the other colliding body, make sure it is moving (has nonzero mass)
	RigidBody* otherBody = (RigidBody*)eventData[P_OTHERBODY].GetPtr();
	Node* otherNode = (Node*)eventData[P_OTHERNODE].GetPtr();

	// If the other collision shape belongs to static geometry, perform world collision
	if (otherBody->GetCollisionLayer() == 2)
		WorldCollision(eventData);

	// If the other node is scripted, perform object-to-object collision
	GameObject* otherObject = otherNode->GetDerivedComponent<GameObject>();

	if (otherObject != NULL)
		ObjectCollision(otherObject, eventData);
}

void GameObject::WorldCollision(VariantMap& eventData)
{
	// Check collision contacts and see if character is standing on ground (look for a contact that has near vertical normal)
	using namespace NodeCollision;

	VectorBuffer contacts = eventData["Contacts"].GetBuffer();
	while (!contacts.IsEof())
	{
		Vector3 contactPosition = contacts.ReadVector3();
		Vector3 contactNormal = contacts.ReadVector3();
		float contactDistance = contacts.ReadFloat();
        float contactImpulse = contacts.ReadFloat();

        // If contact is below node center and pointing up, assume it's ground contact
        if (contactPosition.y_ < node_->GetPosition().y_)
		{
            float level = contactNormal.y_;
			if (level > 0.75)
				onGround = true;
			else
            {
                // If contact is somewhere between vertical/horizontal, is sliding a slope
				if (level > 0.1)
					isSliding = true;
			}
		}
	}

	// Ground contact has priority over sliding contact
	if (onGround == true)
		isSliding = false;
}

void GameObject::ObjectCollision(GameObject* otherObject, VariantMap& eventData)
{
}

void GameObject::ResetWorldCollision()
{
	RigidBody * body = node_->GetComponent<RigidBody>();
	if (body->IsActive())
	{
		onGround = false;
		isSliding = false;
	}
	else
	{
		// If body is not active, assume it rests on the ground
		onGround = true;
		isSliding = false;
	}
}

void GameObject::SetHealth(int hp)
{
	if (hp < 0) hp = 0;
	health = hp;
}


Node* GameObject::SpawnParticleEffect(const Node* node, const Vector3& pos, const String& effectName, float duration, CreateMode mode /*= REPLICATED*/)
{
	ResourceCache* cache = node->GetSubsystem<ResourceCache>();

	Node* newNode = node->GetScene()->CreateChild("Effect", mode);
	newNode->SetPosition( pos);
	
	// Create the particle emitter
	ParticleEmitter* emitter = newNode->CreateComponent<ParticleEmitter>();
	emitter->SetEffect(cache->GetResource<ParticleEffect>(effectName));

	// Create a GameObject for managing the effect lifetime
	GameObject* object = newNode->CreateComponent<GameObject>(LOCAL);
	object->duration = duration;

	return newNode;
}

Node* GameObject::SpawnObject(const Node* node, const Vector3& pos, const Quaternion& rot, const String& className)
{
	ResourceCache* cache = node->GetSubsystem<ResourceCache>();
	XMLFile* xml = cache->GetResource<XMLFile>("Objects/c++/" + className + ".xml");
	if (xml)
	{
		return node->GetScene()->InstantiateXML(xml->GetRoot(), pos, rot);
	}
	return NULL;
}

Node* GameObject::SpawnSound(const Node* node, const Vector3& pos, const String& soundName, float duration)
{
	ResourceCache* cache = node->GetSubsystem<ResourceCache>();
	Node* newNode = node->GetScene()->CreateChild("Sound");
	newNode->SetPosition( pos);

	// Create the sound source
	SoundSource3D* source = newNode->CreateComponent<SoundSource3D>();
	Sound* sound = cache->GetResource<Sound>(soundName);
	source->SetDistanceAttenuation(200, 5000, 1);
	source->Play(sound);

	// Create a GameObject for managing the sound lifetime
	GameObject* object = newNode->CreateComponent<GameObject>(LOCAL);
	object->duration = duration;

	return newNode;
}
