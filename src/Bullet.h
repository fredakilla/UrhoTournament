#pragma once

#include <Urho3D/Scene/LogicComponent.h>
namespace Urho3D
{
	class Input;
	class Node;
}
using namespace Urho3D;

///Game Events.
URHO3D_EVENT(GE_SPAWNBULLET, SpawnBullet)
{
    URHO3D_PARAM(P_POS, Button);                // Vec2
    URHO3D_PARAM(P_ISPLAYER, Buttons);              // bool
}



class Bullet : public LogicComponent
{
    URHO3D_OBJECT(Bullet, LogicComponent)
public:
	Bullet(Context* context);
	virtual ~Bullet();
	static void RegisterObject(Context* context);
	void Init(bool isPlayer,Vector2 spawnPosition);

	virtual void Update(float timeStep);

protected:
	bool isPlayer_;
	float halfHeight_; 
private:
};

class Space : public LogicComponent
{
    URHO3D_OBJECT(Space, LogicComponent)
public:
	Space(Context* context);
	virtual ~Space();
	static void RegisterObject(Context* context);

	virtual void Start();
	virtual void Update(float timeStep);
protected:

private:
};

class Player : public LogicComponent
{
    URHO3D_OBJECT(Player, LogicComponent)
public:
	Player(Context* context);
	virtual ~Player();
	static void RegisterObject(Context* context);

	virtual void Start();
	virtual void Update(float timeStep);

	void DoShooting(float timeStep);
	void MoveShip(float timeStep);

protected:
	Input* input_;
	float halfHeight_;
	float halfWidth_;
	bool allowMove_;
	bool allowShoot_;
	float shootDelta_;
private:
};


class Enemy : public LogicComponent
{
    URHO3D_OBJECT(Enemy, LogicComponent)
public:
	Enemy(Context* context);
	virtual ~Enemy();
	static void RegisterObject(Context* context);


	virtual void Update(float timeStep);

	virtual void DelayedStart();
	void SetSpriteName(const String& spritename);
	void SetSpawnPos(const Vector2& pos);
	void SetPlayerNode(Node* player);
protected:
	WeakPtr<Node> player_;
	bool dir_;
	String spriteName_;
	Vector2 spawnPosition_;
	bool allowShoot_;
	float shootDelta_;
	float moveDelta_;
private:
};

class CapitalShip : public LogicComponent
{
    URHO3D_OBJECT(CapitalShip, LogicComponent)
public:
	CapitalShip(Context* context);
	virtual ~CapitalShip();
	static void RegisterObject(Context* context);

	virtual void DelayedStart();

	virtual void Update(float timeStep);
	
	void SetPlayerNode(Node* player);

protected:
	WeakPtr<Node> player_;
	bool allowShoot_;
	float shootDelta_;
	int health_;
private:
};
