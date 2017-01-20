#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#pragma once

#include "GameObject.h"

const float initialAggression = 0.0020f;
const float initialPrediction = 30.0f;
const float initialAimSpeed = 10.0f;
const float deltaAggression = 0.000025f;
const float deltaPrediction = -0.15f;
const float deltaAimSpeed = 0.30f;
const float maxAggression = 0.01f;
const float maxPrediction = 20.0f;
const float maxAimSpeed = 40.0f;

class Ninja;
using namespace Urho3D;

class AIController : public GameObject
{
    URHO3D_OBJECT(AIController, GameObject);

public:
    AIController(Context* context);
    ~AIController();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start();

    virtual void ObjectCollision(GameObject* otherObject, VariantMap& eventData);

    void GetNewTarget(Node* ownNode);

    void Control(Ninja* ownNinja, Node* ownNode, float timeStep);

    static float aiAggression;
    static float aiPrediction;
    static float aiAimSpeed;

    static void ResetAI();
    static void MakeAIHarder();

protected:
    // Use a weak handle instead of a normal handle to point to the current target
    // so that we don't mistakenly keep it alive.
    WeakPtr<Node> currentTarget;
    float newTargetTimer;

private:


};

#endif // AICONTROLLER_H
