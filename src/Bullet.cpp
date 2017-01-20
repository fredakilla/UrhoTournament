
#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Math/Vector2.h>
#include "Bullet.h"
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/AudioDefs.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Urho2D/SpriteSheet2D.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>





void Bullet::RegisterObject(Context* context)
{
	context->RegisterFactory<Bullet>();
}

Bullet::~Bullet()
{

}

Bullet::Bullet(Context* context) : LogicComponent(context),
isPlayer_(false)
{
	SetUpdateEventMask(USE_UPDATE);
}

void Bullet::Init(bool isPlayer, Vector2 spawnPosition)
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Graphics* graphics = GetSubsystem<Graphics>();

	halfHeight_ = graphics->GetHeight()  * PIXEL_SIZE * 0.5f;
	isPlayer_ = isPlayer;
	

	Sound* laserSound = cache->GetResource<Sound>(isPlayer_ ? "Sounds/laser01.wav" : "Sounds/laser02.wav");
	StaticSprite2D* sprite2D = node_->CreateComponent<StaticSprite2D>();

	if (isPlayer_)
		sprite2D->SetSprite( cache->GetResource<Sprite2D>("Sprites/blue_beam.png"));
	else
		sprite2D->SetSprite(cache->GetResource<Sprite2D>("Sprites/green_beam.png"));

	sprite2D->SetBlendMode(BLEND_ADDALPHA);

	SoundSource* soundSource = node_->CreateComponent<SoundSource>();
	soundSource->SetSoundType(SOUND_EFFECT);
	soundSource->SetGain(0.75f);
	soundSource->Play(laserSound);

	node_->SetPosition2D(spawnPosition);

	if (!isPlayer)
	{
		node_->Roll(180.0f);
	}
}

void Bullet::Update(float timeStep)
{
	float speed = isPlayer_ ? 8.0f : 5.0f;
	speed *= timeStep;
	node_->Translate2D(Vector2(0.0f, speed));

	if (isPlayer_) {

		Vector2 bpos = node_->GetPosition2D();

		// off the top of the screen
		if (bpos.y_ > halfHeight_)
		{
			
			node_->Remove();
			return;
		}
	}
}

void Space::RegisterObject(Context* context)
{
	context->RegisterFactory<Space>();
}
Space::Space(Context* context) :LogicComponent(context)
{

}

Space::~Space()
{

}
void Space::Update(float timeStep)
{
	float speed = 0.75f;
	node_->Translate(Vector3(0, -timeStep * speed, 0));
}

void Space::Start()
{

	node_->SetScale2D(1.5f, 1.5);
	node_->SetPosition2D(0.0f, 12.0f);

	ResourceCache* cache = GetSubsystem<ResourceCache>();
	Sprite2D* spaceSprite = cache->GetResource<Sprite2D>("Sprites/space_background.png");

	// add a sprite component to our node
	StaticSprite2D* sprite2D = node_->CreateComponent<StaticSprite2D>();

	sprite2D->SetOrderInLayer(-200);
	sprite2D->SetBlendMode(BLEND_ADDALPHA);
	sprite2D->SetSprite(spaceSprite);
}


void Player::RegisterObject(Context* context)
{
	context->RegisterFactory<Player>();
}

Player::Player(Context* context) : LogicComponent(context),
allowMove_(true),
allowShoot_(true),
shootDelta_(0.0f),
halfHeight_(0.0f),
halfWidth_(0.0f)
{

}

Player::~Player()
{

}
void Player::Start()
{
	input_ = GetSubsystem<Input>();
	Graphics* graphics = GetSubsystem<Graphics>();
	halfHeight_ = graphics->GetHeight()  * PIXEL_SIZE * 0.5f;
	halfWidth_ = graphics->GetWidth()  * PIXEL_SIZE * 0.5f;
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	SpriteSheet2D* spaceSheet = cache->GetResource<SpriteSheet2D>("Sprites/spacegame_sheet.xml");

	StaticSprite2D* sprite2D = node_->CreateComponent<StaticSprite2D>();

	sprite2D->SetSprite( spaceSheet->GetSprite("spaceship_mantis"));
	sprite2D->SetBlendMode(BLEND_ALPHA);

	node_->SetPosition2D(0, -halfHeight_ + 1.0f);
}

void Player::Update(float timeStep)
{
	if (allowShoot_)
		DoShooting(timeStep);

	if (allowMove_)
		MoveShip(timeStep);
}


void Player::DoShooting(float timeStep)
{
	if (shootDelta_ > 0) {

		shootDelta_ -= timeStep;
		if (shootDelta_ < 0)
			shootDelta_ = 0;

		return;
	}

	if (!input_->GetKeyDown(KEY_W))
		return;

	shootDelta_ = 0.15;

	Vector2 pos = node_->GetPosition2D();
	pos.y_ += .5;

	using namespace SpawnBullet;
	VariantMap& eventData = GetEventDataMap();
	eventData[P_POS] = pos;
	eventData[P_ISPLAYER] = true;
	SendEvent(GE_SPAWNBULLET, eventData);

}
void Player::MoveShip(float timeStep)
{
	float speed = 3.0f * timeStep;

	Vector2 pos = node_->GetPosition2D();

	bool left = false;
	bool right = false;


	if (input_->GetKeyDown(KEY_A) || input_->GetKeyDown(KEY_LEFT))
		pos.x_ -= speed;

	if (input_->GetKeyDown(KEY_D) || input_->GetKeyDown(KEY_RIGHT))
		pos.x_ += speed;

	if (pos.x_ < -halfWidth_ + 2)
		pos.x_ = -halfWidth_ + 2;

	if (pos.x_ > halfWidth_ - 2)
		pos.x_ = halfWidth_ - 2;


	node_->SetPosition2D(pos);
}

Enemy::Enemy(Context* context) : LogicComponent(context),
allowShoot_(true),
shootDelta_(0.0f),
moveDelta_(0.0f),
dir_(false)

{

}

Enemy::~Enemy()
{

}

void Enemy::RegisterObject(Context* context)
{
	context->RegisterFactory<Enemy>();
}

void Enemy::Update(float timeStep)
{
	if (player_.Expired())
		return; 
	Vector2 pos = node_->GetPosition2D();

	Vector2 ppos = player_->GetPosition2D();

	if (Abs(pos.x_- ppos.x_) < .25) {

		//aiShoot(timeStep);

	}

	if (Random() > 0.98f)
	{
		dir_ = !dir_;
	}


	moveDelta_ += (dir_ ? timeStep * 4 : -timeStep * 4);

	pos.x_ = spawnPosition_.x_;
	 pos.y_ = spawnPosition_.y_;
	pos.y_ += Sin(moveDelta_) * 0.1f;
	node_->SetPosition2D( pos);
}

void Enemy::DelayedStart()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	SpriteSheet2D* spaceSheet = cache->GetResource<SpriteSheet2D>("Sprites/spacegame_sheet.xml");

	StaticSprite2D* sprite2D = node_->CreateComponent<StaticSprite2D>();
	sprite2D->SetSprite(spaceSheet->GetSprite(spriteName_));
	sprite2D->SetBlendMode(BLEND_ALPHA);


	node_->SetPosition2D(spawnPosition_);
	node_->Roll(180.0f);
	node_->SetScale2D(0.65f, 0.65f);

	dir_ = (Random() > 0.5);
}

void Enemy::SetSpriteName(const String& spritename)
{
	spriteName_ = spritename;
}

void Enemy::SetSpawnPos(const Vector2& pos)
{
	spawnPosition_ = pos;
}

void Enemy::SetPlayerNode(Node* player)
{
	player_ = WeakPtr<Node>(player);
}

CapitalShip::CapitalShip(Context* context) : LogicComponent(context),
allowShoot_(true),
shootDelta_(0.0f)
{

}

CapitalShip::~CapitalShip()
{

}

void CapitalShip::RegisterObject(Context* context)
{
	context->RegisterFactory<CapitalShip>();
}

void CapitalShip::DelayedStart()
{
	Graphics* graphics = GetSubsystem<Graphics>();
	float halfHeight_ = graphics->GetHeight()  * PIXEL_SIZE * 0.5f;

	ResourceCache* cache = GetSubsystem<ResourceCache>();
	SpriteSheet2D* spaceSheet = cache->GetResource<SpriteSheet2D>("Sprites/spacegame_sheet.xml");

	StaticSprite2D* sprite2D = node_->CreateComponent<StaticSprite2D>();
	sprite2D->SetSprite(spaceSheet->GetSprite("spaceship_locust"));
	sprite2D->SetBlendMode(BLEND_ALPHA);

	node_->SetPosition2D(-4.0f, halfHeight_ - 1.0f);
	node_->Roll(180.0f);

}

void CapitalShip::Update(float timeStep)
{
	if (player_.Expired())
		return;
	Vector2 pos = node_->GetPosition2D();
	Vector2 ppos = player_->GetPosition2D();

	if (Abs(pos.x_ - ppos.x_) > .25) {
		if (pos.x_ < ppos.x_)
			pos.x_ += timeStep * .95;
		else
			pos.x_ -= timeStep * .95;

		node_->SetPosition2D(pos);
	}
	else
	{
		//aiShoot(timeStep);
	}

}

void CapitalShip::SetPlayerNode(Node* player)
{
	player_ = WeakPtr<Node>(player);
}
