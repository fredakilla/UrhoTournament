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
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Scene/Serializable.h>
#include <Urho3D/IO/Deserializer.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Graphics/ParticleEffect.h>


#include "AIController.h"
#include "Ninja.h"

#include <Urho3D/DebugNew.h>


float AIController::aiAimSpeed= initialAimSpeed;
float AIController::aiPrediction = initialPrediction;
float AIController::aiAggression = initialAggression;

void AIController::ResetAI()
{
	aiAggression = initialAggression;
	aiPrediction = initialPrediction;
	aiAimSpeed = initialAimSpeed;
}

void AIController::MakeAIHarder()
{
	aiAggression += deltaAggression;
	if (aiAggression > maxAggression)
		aiAggression = maxAggression;

	aiPrediction += deltaPrediction;
	if (aiPrediction < maxPrediction)
		aiPrediction = maxPrediction;

	aiAimSpeed += deltaAimSpeed;
	if (aiAimSpeed > maxAimSpeed)
		aiAimSpeed = maxAimSpeed;
}

AIController::AIController(Context* context) : GameObject(context)
{
    // Initialize variables to sensible defaults
	newTargetTimer = 0;
}


AIController::~AIController()
{
}

void AIController::Start()
{
	
    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(AIController, HandleNodeCollision));
}

void AIController::ObjectCollision(GameObject* otherObject, VariantMap& eventData)
{

}

void AIController::RegisterObject(Context* context)
{
	context->RegisterFactory<AIController>();
}

void AIController::GetNewTarget(Node* ownNode)
{
	if (!ownNode)
		return;

	newTargetTimer = 0;
	PODVector<Node*> nodes;

	ownNode->GetScene()->GetChildrenWithComponent<Ninja>(nodes, true);

	float closestDistance = M_INFINITY;

	for (PODVector<Node*>::Iterator it = nodes.Begin(); it != nodes.End(); it++)
	{
		Node* otherNode = (*it);

		Ninja* otherNinja = otherNode->GetComponent<Ninja>();
		if (otherNinja)
			if (otherNinja->GetSide() == SIDE_PLAYER && otherNinja->GetHealth() > 0)
			{
				float distance = (ownNode->GetPosition() - otherNode->GetPosition()).LengthSquared();
				if (distance < closestDistance)
				{
					currentTarget = otherNode;
					closestDistance = distance;
				}
			}
	}
}

void AIController::Control(Ninja* ownNinja, Node* ownNode, float timeStep)
{
	if (!ownNinja || !ownNode)
		return;

	// Get new target if none. Do not constantly scan for new targets to conserve CPU time
	if (currentTarget.Get() == NULL)
	{
		newTargetTimer += timeStep;
		if (newTargetTimer > 0.5)
			GetNewTarget(ownNode);
	}

	Node* targetNode = currentTarget.Get();

	if (targetNode != NULL)
	{
		// Check that current target is still alive. Otherwise choose new
		Ninja* targetNinja = targetNode->GetComponent<Ninja>();

		if (targetNinja == NULL || targetNinja->GetHealth() <= 0)
		{
			currentTarget = NULL;
			return;
		}

		RigidBody* targetBody = targetNode->GetComponent<RigidBody>();

		ownNinja->controls.Set(CTRL_FIRE, false);
		ownNinja->controls.Set(CTRL_JUMP, false);

		float deltaX = 0.0f;
		float deltaY = 0.0f;

		// Aim from own head to target's feet
		Vector3 ownPos(ownNode->GetPosition() + Vector3(0.0f, 0.9f, 0.0f));
		Vector3 targetPos(targetNode->GetPosition() + Vector3(0.0f, -0.9f, 0.0f));
		float distance = (targetPos - ownPos).Length();

		// Use prediction according to target distance & estimated snowball speed
		Vector3 currentAim(ownNinja->GetAim() * Vector3(0.0f, 0.0f, 1.0f));
		float predictDistance = distance;
		if (predictDistance > 50.0f) predictDistance = 50.0f;
		Vector3 predictedPos = targetPos + targetBody->GetLinearVelocity() * predictDistance / aiPrediction;
		Vector3 targetAim = (predictedPos - ownPos);

		// Add distance/height compensation
		float compensation = Max(targetAim.Length() - 15, 0.0);
		targetAim += Vector3(0, 0.6, 0) * compensation;

		// X-aiming
		targetAim.Normalize();
		Vector3 currentYaw(currentAim.x_, 0, currentAim.z_);
		Vector3 targetYaw(targetAim.x_, 0, targetAim.z_);
		currentYaw.Normalize();
		targetYaw.Normalize();
		deltaX = Clamp(Quaternion(currentYaw, targetYaw).YawAngle(), -aiAimSpeed, aiAimSpeed);

		// Y-aiming
		Vector3 currentPitch(0, currentAim.y_, 1);
		Vector3 targetPitch(0, targetAim.y_, 1);
		currentPitch.Normalize();
		targetPitch.Normalize();
		deltaY = Clamp(Quaternion(currentPitch, targetPitch).PitchAngle(), -aiAimSpeed, aiAimSpeed);

		ownNinja->controls.yaw_ += 0.1 * deltaX;
		ownNinja->controls.pitch_ += 0.1 * deltaY;

		// Firing? if close enough and relatively correct aim
		if ((distance < 25) && (currentAim.DotProduct(targetAim) > 0.75))
		{
			if (Random(1.0f) < aiAggression)
				ownNinja->controls.Set(CTRL_FIRE, true);
		}

		// Movement
		ownNinja->dirChangeTime -= timeStep;
		if (ownNinja->dirChangeTime <= 0)
		{
			ownNinja->dirChangeTime = 0.5 + Random(1.0f);
			ownNinja->controls.Set(CTRL_UP | CTRL_DOWN | CTRL_LEFT | CTRL_RIGHT, false);

			// Far distance: go forward
			if (distance > 30)
				ownNinja->controls.Set(CTRL_UP, true);
			else if (distance > 6)
			{
				// Medium distance: random strafing, predominantly forward
				float v = Random(1.0f);
				if (v < 0.8)
					ownNinja->controls.Set(CTRL_UP, true);
				float h = Random(1.0f);
				if (h < 0.3)
					ownNinja->controls.Set(CTRL_LEFT, true);
				if (h > 0.7)
					ownNinja->controls.Set(CTRL_RIGHT, true);
			}
			else
			{
				// Close distance: random strafing backwards
				float v = Random(1.0f);
				if (v < 0.8)
					ownNinja->controls.Set(CTRL_DOWN, true);
				float h = Random(1.0f);
				if (h < 0.4)
					ownNinja->controls.Set(CTRL_LEFT, true);
				if (h > 0.6)
					ownNinja->controls.Set(CTRL_RIGHT, true);
			}
		}

		// Random jump, if going forward
		if ((ownNinja->controls.IsDown(CTRL_UP)) && (distance < 1000.0f))
		{
			if (Random(1.0f) < (aiAggression / 5.0f))
				ownNinja->controls.Set(CTRL_JUMP, true);
		}
	}
	else
	{
		// If no target, walk idly
		ownNinja->controls.Set(CTRL_ALL, false);
		ownNinja->controls.Set(CTRL_UP, true);
		ownNinja->dirChangeTime -= timeStep;
		if (ownNinja->dirChangeTime <= 0.0f)
		{
			ownNinja->dirChangeTime = 1.0f + Random(2.0f);
			ownNinja->controls.yaw_ += 0.1f * (Random(600.0f) - 300.0f);
		}
		if (ownNinja->IsSliding())
			ownNinja->controls.yaw_ += 0.2f;
	}
}












