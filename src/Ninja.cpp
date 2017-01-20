#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include "Ninja.h"
#include "AIController.h"

#include <Urho3D/DebugNew.h>




Ninja::Ninja(Context* context) : GameObject(context)
{
    // Initialize variables to sensible defaults
    health = maxHealth = 2;
    okToJump = false;
    smoke = false;
    onGround = false;
    isSliding = false;
    inAirTime = 1;
    onGroundTime = 0;
    throwTime = 0;
    deathTime = 0;
    deathDir = 0;
    dirChangeTime = 0;
    aimX = 0;
    aimY = 0;
}

Ninja::~Ninja()
{
}

void Ninja::RegisterObject(Context* context)
{
    context->RegisterFactory<Ninja>();
}

void Ninja::DelayedStart()
{
    SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(Ninja, HandleNodeCollision));

    // Get horizontal aim from initial rotation
    aimX = controls.yaw_ = node_->GetRotation().YawAngle();


    // Start playing the idle animation immediately, even before the first physics update
    AnimationController* animCtrl = node_->GetChildren()[0]->GetComponent<AnimationController>();
    animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_Idle3.ani", LAYER_MOVE, true);
}

Urho3D::Quaternion Ninja::GetAim()
{
    Quaternion q = Quaternion(aimX, Vector3(0, 1, 0));
    q = q * Quaternion(aimY, Vector3(1, 0, 0));
    return q;
}

void Ninja::SetControls(const Controls& newControls)
{
    controls = newControls;
}

void Ninja::FixedUpdate(float timeStep)
{
    // For multiplayer, replicate the health into the node user variables
    node_->SetVar("Health", health);

    if (health <= 0)
    {
        DeathUpdate(timeStep);
        return;
    }

    // AI control if controller exists
    if (controller != NULL)
        controller->Control(this, node_, timeStep);

    RigidBody* body = node_->GetComponent<RigidBody>();
    AnimationController* animCtrl = node_->GetChildren()[0]->GetComponent<AnimationController>();

    // Turning / horizontal aiming
    if (aimX != controls.yaw_)
        aimX = controls.yaw_;

    // Vertical aiming
    if (aimY != controls.pitch_)
        aimY = controls.pitch_;

    // Force the physics rotation
    Quaternion q(aimX, Vector3(0, 1, 0));
    body->SetRotation(q);

    // Movement ground/air
    Vector3 vel = body->GetLinearVelocity();
    if (onGround)
    {
        // If landed, play a particle effect at feet (use the AnimatedModel node)
        if (inAirTime > 0.5)
        {
            SpawnParticleEffect(node_->GetChildren()[0], node_->GetChildren()[0]->GetWorldPosition(), "Particle/SnowExplosion.xml", 1 );
        }

        inAirTime = 0;
        onGroundTime += timeStep;
    }
    else
    {
        onGroundTime = 0;
        inAirTime += timeStep;
    }

    if (inAirTime < 0.3f && !isSliding)
    {
        bool sideMove = false;

        // Movement in four directions
        if (controls.IsDown(CTRL_UP | CTRL_DOWN | CTRL_LEFT | CTRL_RIGHT))
        {
            float animDir = 1.0f;
            Vector3 force(0, 0, 0);
            if (controls.IsDown(CTRL_UP))
                force += q * Vector3(0, 0, 1);
            if (controls.IsDown(CTRL_DOWN))
            {
                animDir = -1.0f;
                force += q * Vector3(0, 0, -1);
            }
            if (controls.IsDown(CTRL_LEFT))
            {
                sideMove = true;
                force += q * Vector3(-1, 0, 0);
            }
            if (controls.IsDown(CTRL_RIGHT))
            {
                sideMove = true;
                force += q * Vector3(1, 0, 0);
            }
            // Normalize so that diagonal strafing isn't faster
            force.Normalize();
            force *= ninjaMoveForce;
            body->ApplyImpulse(force);

            // Walk or sidestep animation
            if (sideMove)
            {
                animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_Stealth.ani", LAYER_MOVE, true, 0.2);
                animCtrl->SetSpeed("Models/NinjaSnowWar/Ninja_Stealth.ani", animDir * 2.2);
            }
            else
            {
                animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_Walk.ani", LAYER_MOVE, true, 0.2);
                animCtrl->SetSpeed("Models/NinjaSnowWar/Ninja_Walk.ani", animDir * 1.6);
            }
        }
        else
        {
            // Idle animation
            animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_Idle3.ani", LAYER_MOVE, true, 0.2);
        }

        // Overall damping to cap maximum speed
        body->ApplyImpulse(Vector3(-ninjaDampingForce * vel.x_, 0, -ninjaDampingForce * vel.z_));

        // Jumping
        if (controls.IsDown(CTRL_JUMP))
        {
            if (okToJump && inAirTime < 0.1f)
            {
                // Lift slightly off the ground for better animation
                body->SetPosition(body->GetPosition() + Vector3(0, 0.03, 0));
                body->ApplyImpulse(Vector3(0, ninjaJumpForce, 0));
                inAirTime = 1.0f;
                animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_JumpNoHeight.ani", LAYER_MOVE, false, 0.1);
                animCtrl->SetTime("Models/NinjaSnowWar/Ninja_JumpNoHeight.ani", 0.0); // Always play from beginning
                okToJump = false;
            }
        }
        else okToJump = true;
    }
    else
    {
        // Motion in the air
        // Note: when sliding a steep slope, control (or damping) isn't allowed!
        if (inAirTime > 0.3f && !isSliding)
        {
            if (controls.IsDown(CTRL_UP | CTRL_DOWN | CTRL_LEFT | CTRL_RIGHT))
            {
                Vector3 force(0, 0, 0);
                if (controls.IsDown(CTRL_UP))
                    force += q * Vector3(0, 0, 1);
                if (controls.IsDown(CTRL_DOWN))
                    force += q * Vector3(0, 0, -1);
                if (controls.IsDown(CTRL_LEFT))
                    force += q * Vector3(-1, 0, 0);
                if (controls.IsDown(CTRL_RIGHT))
                    force += q * Vector3(1, 0, 0);
                // Normalize so that diagonal strafing isn't faster
                force.Normalize();
                force *= ninjaAirMoveForce;
                body->ApplyImpulse(force);
            }
        }

        // Falling/jumping/sliding animation
        if (inAirTime > 0.1f)
            animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_JumpNoHeight.ani", LAYER_MOVE, false, 0.1);
    }

    // Shooting
    if (throwTime >= 0)
        throwTime -= timeStep;

    // Start fading the attack animation after it has progressed past a certain point
    if (animCtrl->GetTime("Models/NinjaSnowWar/Ninja_Attack1.ani") > 0.1)
        animCtrl->Fade("Models/NinjaSnowWar/Ninja_Attack1.ani", 0.0, 0.5);

    if ((controls.IsPressed(CTRL_FIRE, prevControls)) && (throwTime <= 0))
    {
        Vector3 projectileVel = GetAim() * ninjaThrowVelocity;

        animCtrl->Play("Models/NinjaSnowWar/Ninja_Attack1.ani", LAYER_ATTACK, false, 0.0);
        animCtrl->SetTime("Models/NinjaSnowWar/Ninja_Attack1.ani", 0.0); // Always play from beginning

        Node* snowball = SpawnObject(node_, node_->GetPosition()+ vel * timeStep + q * ninjaThrowPosition, GetAim(), "SnowBall");
        RigidBody* snowballBody = snowball->GetComponent<RigidBody>();
        snowballBody->SetLinearVelocity(projectileVel);

        GameObject* snowballObject = snowball->GetComponent<GameObject>();
        if (snowballObject)
        {
            snowballObject->SetSide(side);
            snowballObject->SetCreatorID(node_->GetID());
        }

        PlaySound("Sounds/NutThrow.wav");
        throwTime = ninjaThrowDelay;
    }

    prevControls = controls;
    ResetWorldCollision();
}

bool Ninja::Heal(int amount)
{
    if (health == maxHealth)
        return false;

    health += amount;
    if (health > maxHealth)
        health = maxHealth;
    // If player, play the "powerup" sound
    if (side == SIDE_PLAYER)
        PlaySound("Sounds/Powerup.wav");
    return true;
}

void Ninja::DeathUpdate(float timeStep)
{
    RigidBody* body = node_->GetComponent<RigidBody>();
    CollisionShape* shape = node_->GetComponent<CollisionShape>();
    Node* modelNode = node_->GetChildren()[0];
    AnimationController* animCtrl = modelNode->GetComponent<AnimationController>();
    AnimatedModel* model = modelNode->GetComponent<AnimatedModel>();

    Vector3 vel = body->GetLinearVelocity();

    // Overall damping to cap maximum speed
    body->ApplyImpulse(Vector3(-ninjaDampingForce * vel.x_, 0, -ninjaDampingForce * vel.z_));

    // Collide only to world geometry
    body->SetCollisionMask(2);

    // Pick death animation on first death update
    if (deathDir == 0.0f)
    {
        if (Random(1.0f) < 0.5f)
            deathDir = -1.0f;
        else
            deathDir = 1.0f;

        PlaySound("Sounds/SmallExplosion.wav");

        VariantMap &eventData = GetEventDataMap();
        eventData[GameObjectPoints::P_POINTS] = ninjaPoints;
        eventData[GameObjectPoints::P_RECIVER] = lastDamageCreatorID;
        eventData[GameObjectPoints::P_DAMAGESIDE] = lastDamageSide;
        SendEvent(GE_POINTS, eventData);
        SendEvent(GE_KILL, eventData);
    }

    deathTime += timeStep;

    // Move the model node to center the corpse mostly within the physics cylinder
    // (because of the animation)
    if (deathDir < 0.0f)
    {
        // Backward death
        animCtrl->StopLayer(LAYER_ATTACK, 0.1f);
        animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_Death1.ani", LAYER_MOVE, false, 0.2f);
        animCtrl->SetSpeed("Models/NinjaSnowWar/Ninja_Death1.ani", 0.5f);
        if ((deathTime >= 0.3f) && (deathTime < 0.8f))
            modelNode->Translate(Vector3(0.0f, 0.0f, 4.25f * timeStep));
    }
    else if (deathDir > 0.0f)
    {
        // Forward death
        animCtrl->StopLayer(LAYER_ATTACK, 0.1f);
        animCtrl->PlayExclusive("Models/NinjaSnowWar/Ninja_Death2.ani", LAYER_MOVE, false, 0.2f);
        animCtrl->SetSpeed("Models/NinjaSnowWar/Ninja_Death2.ani", 0.5f);
        if ((deathTime >= 0.4f) && (deathTime < 0.8f))
            modelNode->Translate(Vector3(0.0f, 0.0f, -4.25f * timeStep));
    }

    // Create smokecloud just before vanishing
    if ((deathTime >(ninjaCorpseDuration - 1)) && (!smoke))
    {

        SpawnParticleEffect(node_, node_->GetPosition()+ Vector3(0, -0.4, 0), "Particle/Smoke.xml", 8);
        smoke = true;
    }

    if (deathTime > ninjaCorpseDuration)
    {
        /// \todo does not work ... some kind of bug dont know why ...
        //	SpawnObject(node_, node_->GetPosition() + Vector3(0, -0.5, 0), Quaternion(), "LightFlash");
        SpawnSound(node_, node_->GetPosition() + Vector3(0, -0.5, 0), "Sounds/BigExplosion.wav", 2);
        UnsubscribeFromAllEvents();
        node_->Remove();
    }
}
