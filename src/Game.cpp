//
// Copyright (c) 2008-2014 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <Urho3D/Urho3D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Engine/Console.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Urho2D/Drawable2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Math/Color.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/AudioDefs.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Deserializer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Physics/RigidBody.h>

#include "Game.h"
#include "GameObject.h"
#include "AIController.h"
#include "FootSteps.h"
#include "LightFlash.h"
#include "Ninja.h"
#include "Potion.h"
#include "SnowBall.h"
#include "SnowCrate.h"
#include "Player.h"

#include <Urho3D/DebugNew.h>


#include <Urho3D/Graphics/Animation.h>
#include "MagicParticleEffect.h"
#include "MagicParticleEmitter.h"

URHO3D_DEFINE_APPLICATION_MAIN(Game)

Game::Game(Context* context) :
    Application(context),
    yaw_(0.0f),
    pitch_(0.0f),
    touchEnabled_(false),
    screenJoystickIndex_(M_MAX_UNSIGNED),
    screenJoystickSettingsIndex_(M_MAX_UNSIGNED)

{
    singlePlayer = true;
    gameOn = false;
    drawDebug = false;
    drawOctreeDebug = false;
    maxEnemies = 0;
    incrementCounter = 0;
    enemySpawnTimer = 0;
    powerupSpawnTimer = 0;
    clientNodeID = 0;
    clientScore = 0;

    runServer = false;
    runClient = false;
    serverPort = 1234;
    nobgm = false;

    GameObject::RegisterObject(context);
    AIController::RegisterObject(context);
    FootSteps::RegisterObject(context);
    LightFlash::RegisterObject(context);
    Ninja::RegisterObject(context);
    Potion::RegisterObject(context);
    SnowBall::RegisterObject(context);
    SnowCrate::RegisterObject(context);

    MagicParticleEffect::RegisterObject(context);
    MagicParticleEmitter::RegisterObject(context);
}
void Game::Setup()
{
    // Modify engine startup parameters
    engineParameters_["WindowTitle"] = "Urho Tournament";
    engineParameters_["FullScreen"] = false;
    engineParameters_["Headless"] = false;
    engineParameters_["ResourcePaths"] = "Data;CoreData;GameData";
}

void Game::Stop()
{
}

void Game::Start()
{
    engine_ = GetSubsystem<Engine>();
    if (engine_->IsHeadless())
        OpenConsoleWindow();

    ParseNetworkArguments();
    if (runServer || runClient)
        singlePlayer = false;

    InitAudio();
    InitConsole();
    InitScene();
    InitNetworking();
    CreateCamera();
    CreateOverlays();

    // set mouse visible during debug to avoid no mouse pointer on breakpoint.
    //GetSubsystem<Input>()->SetMouseVisible(true);

    SubscribeToEvent(scene_, E_SCENEUPDATE, URHO3D_HANDLER(Game, HandleUpdate));
    if (scene_->GetComponent<PhysicsWorld>() != NULL)
        SubscribeToEvent(scene_->GetComponent<PhysicsWorld>(), E_PHYSICSPRESTEP, URHO3D_HANDLER(Game, HandleFixedUpdate));
    SubscribeToEvent(scene_, E_SCENEPOSTUPDATE, URHO3D_HANDLER(Game, HandlePostUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Game, HandlePostRenderUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Game, HandleKeyDown));

    SubscribeToEvent(GE_POINTS, URHO3D_HANDLER(Game, HandlePoints));
    SubscribeToEvent(GE_KILL, URHO3D_HANDLER(Game, HandleKill));
    SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(Game, HandleScreenMode));

    if (singlePlayer)
    {
        StartGame(NULL);
        engine_->SetPauseMinimized(true);
    }
}

void Game::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    UpdateControls();
    CheckEndAndRestart();

    if (engine_->IsHeadless())
    {
        String command = GetConsoleInput();
        if (command.Length() > 0)
            GetSubsystem<Script>()->Execute(command);
    }
    else
    {
        DebugHud* debugHud = GetSubsystem<DebugHud>();

        if (debugHud && debugHud->GetMode() != DEBUGHUD_SHOW_NONE)
        {
            Node* playerNode = FindOwnNode();
            if (playerNode != NULL)
            {
                debugHud->SetAppStats("Player Pos", playerNode->GetWorldPosition().ToString());
                debugHud->SetAppStats("Player Yaw", Variant(playerNode->GetWorldRotation().YawAngle()));
            }
            else
                debugHud->ClearAppStats();
        }
    }
}

void Game::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    if (engine_->IsHeadless())
        return;

    // If draw debug mode is enabled, draw navigation mesh debug geometry
    if (drawDebug)
        scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    if (drawOctreeDebug)
        scene_->GetComponent<Octree>()->DrawDebugGeometry(true);
}

void Game::ParseNetworkArguments()
{
    const Vector<String>& arguments = GetArguments();

    for (unsigned i = 0; i < arguments.Size(); ++i)
    {
        String argument = arguments[i].ToLower();
        if (argument[0] == '-')
        {
            argument = argument.Substring(1);
            if (argument == "server")
            {
                runServer = true;
                runClient = false;
            }
            else if (argument == "address")
            {
                runClient = true;
                runServer = false;
                serverAddress = arguments[i + 1];
                ++i;
            }
            else if (argument == "username")
            {
                userName = arguments[i + 1];
                ++i;
            }
            else if (argument == "nobgm")
                nobgm = true;
        }
    }
}

void Game::InitAudio()
{
    if (engine_->IsHeadless())
        return;
    Audio* audio = GetSubsystem<Audio>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Lower mastervolumes slightly.
    audio->SetMasterGain(SOUND_MASTER, 0.75f);
    audio->SetMasterGain(SOUND_MUSIC, 0.9f);

    if (!nobgm)
    {
        Sound* musicFile = cache->GetResource<Sound>("Music/Ninja Gods.ogg");
        musicFile->SetLooped(true);

        // Note: the non-positional sound source component need to be attached to a node to become effective
        // Due to networked mode clearing the scene on connect, do not attach to the scene itself
        musicNode = new Node(context_);
        musicSource = musicNode->CreateComponent<SoundSource>();
        musicSource->SetSoundType(SOUND_MUSIC);
        musicSource->Play(musicFile);
    }
}

void Game::InitConsole()
{
    if (engine_->IsHeadless())
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    UI* ui = GetSubsystem<UI>();
    ui->GetRoot()->SetDefaultStyle(uiStyle);

    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(uiStyle);
    console->GetBackground()->SetOpacity(0.8f);

    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(uiStyle);
}

void Game::InitScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);
    scene_->SetName("NinjaSnowWar");

    // For the multiplayer client, do not load the scene, let it load from the server
    if (runClient)
        return;

    scene_->LoadXML(*cache->GetFile("Scenes/NinjaSnowWar.xml"));

    // On mobile devices render the shadowmap first for better performance, adjust the cascaded shadows
    String platform = GetPlatform();

    if (platform == "Android" || platform == "iOS" || platform == "Raspberry Pi")
    {
        Renderer* renderer = GetSubsystem<Renderer>();
        renderer->SetReuseShadowMaps(false);

        // Adjust the directional light shadow range slightly further, as only the first
        // cascade is supported
        Node* dirLightNode = scene_->GetChild("GlobalLight", true);
        if (dirLightNode != NULL)
        {
            Light* dirLight = dirLightNode->GetComponent<Light>();
            dirLight->SetShadowCascade(CascadeParameters(15.0f, 0.0f, 0.0f, 0.0f, 0.9f));
        }
    }
    Graphics* graphics = GetSubsystem<Graphics>();
    // Precache shaders if possible
    if (!engine_->IsHeadless() && cache->Exists("NinjaSnowWarShaders.xml"))
        graphics->PrecacheShaders(*cache->GetFile("NinjaSnowWarShaders.xml"));




    // TO DELETE : Test animated model
    Node* node = scene_->CreateChild("");
    AnimatedModel* object = node->CreateComponent<AnimatedModel>();
    object->SetModel(cache->GetResource<Model>("Models/Galtrilian/Galtrilian.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Models/Galtrilian/Materials/Galtrilian_Material.xml"));
    object->SetCastShadows(true);
    AnimationController* animCtrl = node->CreateComponent<AnimationController>();
    animCtrl->Play("Models/Galtrilian/Standing_Run_Forward.ani", 0, true, 0.2);
}

void Game::InitNetworking()
{
    Network * network = GetSubsystem<Network>();

    network->SetUpdateFps(25); // 1/4 of physics FPS
    network->RegisterRemoteEvent("PlayerSpawned");
    network->RegisterRemoteEvent("UpdateScore");
    network->RegisterRemoteEvent("UpdateHiscores");
    network->RegisterRemoteEvent("ParticleEffect");

    if (runServer)
    {
        network->StartServer(serverPort);

        // Disable physics interpolation to ensure clients get sent physically correct transforms
        scene_->GetComponent<PhysicsWorld>()->SetInterpolation(false);

        SubscribeToEvent(E_CLIENTIDENTITY, URHO3D_HANDLER(Game, HandleClientIdentity));
        SubscribeToEvent(E_CLIENTSCENELOADED, URHO3D_HANDLER(Game, HandleClientSceneLoaded));
        SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(Game, HandleClientDisconnected));
    }
    if (runClient)
    {
        VariantMap identity;
        identity["UserName"] = userName;
        network->SetUpdateFps(50); // Increase controls send rate for better responsiveness
        network->Connect(serverAddress, serverPort, scene_, identity);

        SubscribeToEvent(GE_PLAYERSPAWNED, URHO3D_HANDLER(Game, HandlePlayerSpawned));
        SubscribeToEvent(GE_UPDATESCORE, URHO3D_HANDLER(Game, HandleUpdateScore));
        SubscribeToEvent(GE_UPDATEHISCORE, URHO3D_HANDLER(Game, HandleUpdateHiscores));
        SubscribeToEvent(E_NETWORKUPDATESENT, URHO3D_HANDLER(Game, HandleNetworkUpdateSent));
    }
}

void Game::InitTouchInput()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    touchEnabled_ = true;
    screenJoystickIndex_ = GetSubsystem<Input>()->AddScreenJoystick(cache->GetResource<XMLFile>("UI/ScreenJoystick_NinjaSnowWar.xml"));
}

void Game::CreateCamera()
{
    // Note: the camera is not in the scene
    cameraNode_ = new Node(context_);
    cameraNode_->SetPosition(Vector3(0.0f, 2.0f, -10.0f));

    gameCamera = cameraNode_->CreateComponent<Camera>();
    gameCamera->SetNearClip(0.5f);
    gameCamera->SetFarClip(160.0f);

    if (!engine_->IsHeadless())
    {
        Renderer* renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, gameCamera));
        renderer->SetViewport(0, viewport);
        Audio* audio = GetSubsystem<Audio>();
        audio->SetListener(cameraNode_->CreateComponent<SoundListener>());
    }
}

void Game::CreateOverlays()
{
    if (engine_->IsHeadless() || runServer)
        return;
    Graphics* graphics = GetSubsystem<Graphics>();
    int height = graphics->GetHeight() / 22;
    if (height > 64)
        height = 64;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    Input* input = GetSubsystem<Input>();

    sight = ui->GetRoot()->CreateChild<BorderImage>();
    sight->SetTexture(cache->GetResource<Texture2D>("Textures/NinjaSnowWar/Sight.png"));
    sight->SetAlignment(HA_CENTER, VA_CENTER);
    sight->SetSize(height, height);

    Font* font = cache->GetResource<Font>("Fonts/BlueHighway.ttf");

    scoreText = ui->GetRoot()->CreateChild<Text>();
    scoreText->SetFont(font, 13);
    scoreText->SetAlignment(HA_LEFT, VA_TOP);
    scoreText->SetPosition(5, 5);
    scoreText->SetColor(C_BOTTOMLEFT, Color(1, 1, 0.25));
    scoreText->SetColor(C_BOTTOMRIGHT, Color(1, 1, 0.25));

    hiscoreText = ui->GetRoot()->CreateChild<Text>();
    hiscoreText->SetFont(font, 13);
    hiscoreText->SetAlignment(HA_RIGHT, VA_TOP);
    hiscoreText->SetPosition(-5, 5);
    hiscoreText->SetColor(C_BOTTOMLEFT, Color(1, 1, 0.25));
    hiscoreText->SetColor(C_BOTTOMRIGHT, Color(1, 1, 0.25));

    messageText = ui->GetRoot()->CreateChild<Text>();
    messageText->SetFont(font, 13);
    messageText->SetAlignment(HA_CENTER, VA_CENTER);
    messageText->SetPosition(0, -height * 2);
    messageText->SetColor(Color(1, 0, 0));

    BorderImage* healthBorder = ui->GetRoot()->CreateChild<BorderImage>();
    healthBorder->SetTexture(cache->GetResource<Texture2D>("Textures/NinjaSnowWar/HealthBarBorder.png"));
    healthBorder->SetAlignment(HA_CENTER, VA_TOP);
    healthBorder->SetPosition(0, 8);
    healthBorder->SetSize(120, 20);

    healthBar = ui->GetRoot()->CreateChild<BorderImage>();
    healthBar->SetTexture(cache->GetResource<Texture2D>("Textures/NinjaSnowWar/HealthBarInside.png"));
    healthBar->SetPosition(2, 2);
    healthBar->SetSize(116, 16);
    healthBorder->AddChild(healthBar);

    if (GetPlatform() == "Android" || GetPlatform() == "iOS")
        // On mobile platform, enable touch by adding a screen joystick
        InitTouchInput();
    else if (input->GetNumJoysticks() == 0)
        // On desktop platform, do not detect touch when we already got a joystick
        SubscribeToEvent(E_TOUCHBEGIN, URHO3D_HANDLER(Game, HandleTouchBegin));
}

void Game::SetMessage(const String& message)
{
    if (messageText != NULL)
        messageText->SetText(message);
}

void Game::SpawnPlayer(Connection* connection)
{
    Vector3 spawnPosition;
    if (singlePlayer)
        spawnPosition = Vector3(0.0f, 0.97f, 0.0f);
    else
        spawnPosition = Vector3(Random(spawnAreaSize) - spawnAreaSize * 0.5f, 0.97f, Random(spawnAreaSize) - spawnAreaSize);

    Node* playerNode = GameObject::SpawnObject(scene_, spawnPosition, Quaternion(), "Galtrilian");
    // Set owner connection. Owned nodes are always updated to the owner at full frequency
    playerNode->SetOwner(connection);
    playerNode->SetName("Player");

    // Initialize variables
    Ninja* playerNinja = playerNode->GetOrCreateComponent<Ninja>();
    playerNinja->SetHealth(playerHealth);
    playerNinja->SetMaxHealth(playerHealth);
    playerNinja->SetSide(SIDE_PLAYER);

    // Make sure the player can not shoot on first frame by holding the button down
    if (connection == NULL)
        playerNinja->controls = playerNinja->prevControls = playerControls;
    else
        playerNinja->controls = playerNinja->prevControls = connection->controls_;

    // Check if player entry already exists
    int playerIndex = -1;
    for (unsigned int i = 0; i < players.Size(); ++i)
    {
        if (players[i].connection == connection)
        {
            playerIndex = i;
            break;
        }
    }

    // Does not exist, create new
    if (playerIndex < 0)
    {
        playerIndex = players.Size();
        players.Resize(players.Size() + 1);
        players[playerIndex] = Player();
        players[playerIndex].connection = connection;

        if (connection != NULL)
        {
            players[playerIndex].name = connection->identity_["UserName"].GetString();
            // In multiplayer, send current hiscores to the new player
            SendHiscores(playerIndex);
        }
        else
        {
            players[playerIndex].name = "Player";
            // In singleplayer, create also the default hiscore entry immediately
            HiscoreEntry newHiscore;
            newHiscore.name = players[playerIndex].name;
            newHiscore.score = 0;
            hiscores.Push(newHiscore);
        }
    }

    players[playerIndex].nodeID = playerNode->GetID();
    players[playerIndex].score = 0;

    if (connection != NULL)
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        // In multiplayer, send initial score, then send a remote event that tells the spawned node's ID
        // It is important for the event to be in-order so that the node has been replicated first
        SendScore(playerIndex);
        VariantMap eventData;
        eventData["NodeID"] = playerNode->GetID();
        connection->SendRemoteEvent("PlayerSpawned", true, eventData);

        // Create name tag (Text3D component) for players in multiplayer
        Node* textNode = playerNode->CreateChild("NameTag");
        textNode->SetPosition(Vector3(0.0f, 1.2f, 0.0f));
        Text3D* text3D = textNode->CreateComponent<Text3D>();
        Font* font = cache->GetResource<Font>("Fonts/BlueHighway.ttf");
        text3D->SetFont(font, 19);
        text3D->SetColor(Color(1.0f, 1.0f, 0.0f));
        text3D->SetText(players[playerIndex].name);
        text3D->SetHorizontalAlignment(HA_CENTER);
        text3D->SetVerticalAlignment(VA_CENTER);
        text3D->SetFaceCameraMode(FC_ROTATE_XYZ);
    }
}

void Game::StartGame(Connection* connection)
{
    // Clear the scene of all existing scripted objects
    {
        PODVector<Node*> dest;
        scene_->GetChildren(dest, true);
        for (PODVector<Node*>::ConstIterator i = dest.Begin(); i != dest.End(); ++i)
        {
            if ((*i))
                for (Vector<SharedPtr<Component> >::ConstIterator it = (*i)->GetComponents().Begin(); it != (*i)->GetComponents().End(); ++it)
                {
                    GameObject* component = dynamic_cast<GameObject*>(it->Get());
                    if (component)
                    {
                        (*i)->Remove();
                        break;
                    }
                }
        }
    }

    players.Clear();
    SpawnPlayer(connection);

    AIController::ResetAI();

    gameOn = true;
    maxEnemies = initialMaxEnemies;
    incrementCounter = 0;
    enemySpawnTimer = 0.0f;
    powerupSpawnTimer = 0.0f;

    if (singlePlayer)
    {
        playerControls.yaw_ = 0.0f;
        playerControls.pitch_ = 0.0f;
        SetMessage("single");
    }
}

void Game::SendHiscores(int playerIndex)
{
    if (!runServer)
        return;

    VectorBuffer data;
    data.WriteVLE(hiscores.Size());
    for (unsigned int i = 0; i < hiscores.Size(); ++i)
    {
        data.WriteString(hiscores[i].name);
        data.WriteInt(hiscores[i].score);
    }
    Network * network = GetSubsystem<Network>();

    using namespace UpdateHiscores;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_HISCORES] = data.GetBuffer();

    if (playerIndex >= 0 && playerIndex < int(players.Size()))
        players[playerIndex].connection->SendRemoteEvent(GE_UPDATEHISCORE, true, eventData);
    else
        network->BroadcastRemoteEvent(scene_, GE_UPDATEHISCORE, true, eventData); // Send to all in scene
}

void Game::SendScore(int playerIndex)
{
    if (!runServer || playerIndex < 0 || playerIndex >= int(players.Size()))
        return;
    using namespace UpdateScore;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_SCORE] = players[playerIndex].score;
    players[playerIndex].connection->SendRemoteEvent(GE_UPDATESCORE, true, eventData);
}

void Game::HandleFixedUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace PhysicsPreStep;
    float timeStep = eventData["TimeStep"].GetFloat();

    // Spawn new objects, singleplayer or server only
    if (singlePlayer || runServer)
        SpawnObjects(timeStep);
}

void Game::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    UpdateCamera();
    UpdateStatus();
}

void Game::HandlePoints(StringHash eventType, VariantMap& eventData)
{
    using namespace GameObjectPoints;

    if (eventData[P_DAMAGESIDE].GetInt() == SIDE_PLAYER)
    {
        // Get node ID of the object that should receive points -> use it to find player index
        int playerIndex = FindPlayerIndex(eventData[P_RECIVER].GetInt());
        if (playerIndex >= 0)
        {
            players[playerIndex].score += eventData[P_POINTS].GetInt();
            SendScore(playerIndex);

            bool newHiscore = CheckHiscore(playerIndex);
            if (newHiscore)
                SendHiscores(-1);
        }
    }
}

void Game::HandleKill(StringHash eventType, VariantMap& eventData)
{
    using namespace  GameObjectKill;

    if (eventData[P_DAMAGESIDE].GetInt() == SIDE_PLAYER)
    {
        AIController::MakeAIHarder();

        // Increment amount of simultaneous enemies after enough kills
        incrementCounter++;
        if (incrementCounter >= incrementEach)
        {
            incrementCounter = 0;
            if (maxEnemies < finalMaxEnemies)
                maxEnemies++;
        }
    }
}

void Game::HandleClientIdentity(StringHash eventType, VariantMap& eventData)
{
    using namespace ClientIdentity;
    /// \todo why use GetEventSender ?
    //	dynamic_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
    Connection* connection = dynamic_cast<Connection*>(GetEventSender());
    if (connection)
    {
        // If user has empty name, invent one
        if (connection->identity_["UserName"].GetString().Trimmed().Empty())
            connection->identity_["UserName"] = "user" + Random(1000);
        // Assign scene to begin replicating it to the client
        connection->SetScene(scene_);
    }
}

void Game::HandleClientSceneLoaded(StringHash eventType, VariantMap& eventData)
{
    using namespace ClientSceneLoaded;
    /// \todo why use GetEventSender ?
    //	dynamic_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

    // Now client is actually ready to begin. If first player, clear the scene and restart the game
    Connection* connection = dynamic_cast<Connection*>(GetEventSender());
    if (connection)
    {
        // If user has empty name, invent one
        if (players.Empty())
            StartGame(connection);
        else
            SpawnPlayer(connection);
    }
}

void Game::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
    /// \todo why use GetEventSender ?
    //	dynamic_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

    Connection* connection = dynamic_cast<Connection*>(GetEventSender());
    // Erase the player entry, and make the player's ninja commit seppuku (if exists)
    if (connection)
        for (unsigned int i = 0; i < players.Size(); ++i)
        {
            if (players[i].connection == connection)
            {
                players[i].connection = NULL;
                Node* playerNode = FindPlayerNode(i);
                if (playerNode != NULL)
                {
                    Ninja* playerNinja = playerNode->GetComponent<Ninja>();
                    playerNinja->SetHealth(0);
                    playerNinja->SetLastDamageSide(SIDE_NEUTRAL); // No-one scores from this
                }
                players.Erase(i);
                return;
            }
        }
}

void Game::HandlePlayerSpawned(StringHash eventType, VariantMap& eventData)
{
    using namespace PlayerSpawned;

    // Store our node ID and mark the game as started
    clientNodeID = eventData[P_NODEID].GetInt();
    gameOn = true;
    SetMessage("");

    // Copy initial yaw from the player node (we should have it replicated now)
    Node* playerNode = FindOwnNode();
    if (playerNode != NULL)
    {
        playerControls.yaw_ = playerNode->GetRotation().YawAngle();
        playerControls.pitch_ = 0.0f;

        // Disable the nametag from own character
        Node* nameTag = playerNode->GetChild("NameTag");
        nameTag->SetEnabled(false);
    }
}

void Game::HandleUpdateScore(StringHash eventType, VariantMap& eventData)
{
    using namespace UpdateScore;
    clientScore = eventData[P_SCORE].GetInt();
    scoreText->SetText("Score " + clientScore);
}

void Game::HandleUpdateHiscores(StringHash eventType, VariantMap& eventData)
{
    using namespace UpdateHiscores;

    VectorBuffer data(eventData[P_HISCORES].GetBuffer());

    hiscores.Resize(data.ReadVLE());
    for (unsigned int i = 0; i < hiscores.Size(); ++i)
    {
        hiscores[i].name = data.ReadString();
        hiscores[i].score = data.ReadInt();
    }

    String allHiscores;
    for (unsigned int i = 0; i < hiscores.Size(); ++i)
        allHiscores += hiscores[i].name + " " + String(hiscores[i].score) + "\n";
    hiscoreText->SetText(allHiscores);
}

void Game::HandleNetworkUpdateSent(StringHash eventType, VariantMap& eventData)
{
    Network * network = GetSubsystem<Network>();
    // Clear accumulated buttons from the network controls
    if (network->GetServerConnection() != NULL)
        network->GetServerConnection()->controls_.Set(CTRL_ALL, false);
}

void Game::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;


    Input* input = GetSubsystem<Input>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    int key = eventData[P_KEY].GetInt();

    if (key == KEY_ESCAPE)
    {
        if (!GetSubsystem<Console>()->IsVisible())
            engine_->Exit();
        else
            GetSubsystem<Console>()->SetVisible(false);
    }

    if (key == KEY_F1)
        GetSubsystem<Console>()->Toggle();

    if (key == KEY_F2)
        GetSubsystem<DebugHud>()->ToggleAll();

    if (key == KEY_F3)
        drawDebug = !drawDebug;

    if (key == KEY_F4)
        drawOctreeDebug = !drawOctreeDebug;

    if (key == KEY_F5)
        GetSubsystem<DebugHud>()->Toggle(DEBUGHUD_SHOW_EVENTPROFILER);

    // Take screenshot
    if (key == KEY_F6)
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        Image screenshot(context_);
        graphics->TakeScreenShot(screenshot);
        // Here we save in the Data folder with date and time appended
        screenshot.SavePNG(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Screenshot_" +
                           Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_') + ".png");
    }

    // Allow pause only in singleplayer
    if (key == KEY_P && singlePlayer && !GetSubsystem<Console>()->IsVisible() && gameOn)
    {
        scene_->SetUpdateEnabled(!scene_->IsUpdateEnabled());
        if (!scene_->IsUpdateEnabled())
        {
            SetMessage("PAUSED");
            GetSubsystem<Audio>()->PauseSoundType(SOUND_EFFECT);

            // Open the settings joystick only if the controls screen joystick was already open
            if (screenJoystickIndex_ >= 0)
            {
                if (screenJoystickSettingsIndex_ < 0)
                    screenJoystickSettingsIndex_ = input->AddScreenJoystick(cache->GetResource<XMLFile>("UI/ScreenJoystickSettings_NinjaSnowWar.xml"));
                else
                    input->SetScreenJoystickVisible(screenJoystickSettingsIndex_, true);
            }
        }
        else
        {
            SetMessage("");
            GetSubsystem<Audio>()->ResumeSoundType(SOUND_EFFECT);

            if (screenJoystickSettingsIndex_ >= 0)
            {
                input->SetScreenJoystickVisible(screenJoystickSettingsIndex_, false);
            }
        }
    }
}

void Game::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
}

void Game::HandleTouchBegin(StringHash eventType, VariantMap& eventData)
{
    // On some platforms like Windows the presence of touch input can only be detected dynamically
    InitTouchInput();
    UnsubscribeFromEvent(E_TOUCHBEGIN);
}

void Game::HandleScreenMode(StringHash eventType, VariantMap& eventData)
{
    Graphics* graphics = GetSubsystem<Graphics>();
    int height = graphics->GetHeight() / 22;
    if (height > 64)
        height = 64;
    sight->SetSize(height, height);
    messageText->SetPosition(0, -height * 2);
}

void Game::UpdateControls()
{
    Input* input = GetSubsystem<Input>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Network* network = GetSubsystem<Network>();

    if (singlePlayer || runClient)
    {
        prevPlayerControls = playerControls;
        playerControls.Set(CTRL_ALL, false);

        if (touchEnabled_)
        {
            for (unsigned int i = 0; i < input->GetNumTouches(); ++i)
            {
                TouchState* touch = input->GetTouch(i);
                if (touch->touchedElement_.Get() == NULL)
                {
                    // Touch on empty space
                    playerControls.yaw_ += touchSensitivity * gameCamera->GetFov() / graphics->GetHeight() * touch->delta_.x_;
                    playerControls.pitch_ += touchSensitivity * gameCamera->GetFov() / graphics->GetHeight() * touch->delta_.y_;
                }
            }
        }

        if (input->GetNumJoysticks() > 0)
        {
            JoystickState* joystick = touchEnabled_ ? input->GetJoystick(screenJoystickIndex_) : input->GetJoystickByIndex(0);
            if (joystick->GetNumButtons() > 0)
            {
                if (joystick->GetButtonDown(0))
                    playerControls.Set(CTRL_JUMP, true);
                if (joystick->GetButtonDown(1))
                    playerControls.Set(CTRL_FIRE, true);
                if (joystick->GetNumButtons() >= 6)
                {
                    if (joystick->GetButtonDown(4))
                        playerControls.Set(CTRL_JUMP, true);
                    if (joystick->GetButtonDown(5))
                        playerControls.Set(CTRL_FIRE, true);
                }
                if (joystick->GetNumHats() > 0)
                {
                    if ((joystick->GetHatPosition(0) & HAT_LEFT) != 0)
                        playerControls.Set(CTRL_LEFT, true);
                    if ((joystick->GetHatPosition(0) & HAT_RIGHT) != 0)
                        playerControls.Set(CTRL_RIGHT, true);
                    if ((joystick->GetHatPosition(0) & HAT_UP) != 0)
                        playerControls.Set(CTRL_UP, true);
                    if ((joystick->GetHatPosition(0) & HAT_DOWN) != 0)
                        playerControls.Set(CTRL_DOWN, true);
                }
                if (joystick->GetNumAxes() >= 2)
                {
                    if (joystick->GetAxisPosition(0) < -joyMoveDeadZone)
                        playerControls.Set(CTRL_LEFT, true);
                    if (joystick->GetAxisPosition(0) > joyMoveDeadZone)
                        playerControls.Set(CTRL_RIGHT, true);
                    if (joystick->GetAxisPosition(1) < -joyMoveDeadZone)
                        playerControls.Set(CTRL_UP, true);
                    if (joystick->GetAxisPosition(1) > joyMoveDeadZone)
                        playerControls.Set(CTRL_DOWN, true);
                }
                if (joystick->GetNumAxes() >= 4)
                {
                    float lookX = joystick->GetAxisPosition(2);
                    float lookY = joystick->GetAxisPosition(3);

                    if (lookX < -joyLookDeadZone)
                        playerControls.yaw_ -= joySensitivity * lookX * lookX;
                    if (lookX > joyLookDeadZone)
                        playerControls.yaw_ += joySensitivity * lookX * lookX;
                    if (lookY < -joyLookDeadZone)
                        playerControls.pitch_ -= joySensitivity * lookY * lookY;
                    if (lookY > joyLookDeadZone)
                        playerControls.pitch_ += joySensitivity * lookY * lookY;
                }
            }
        }

        Console* console = GetSubsystem<Console>();
        // For the triggered actions (fire & jump) check also for press, in case the FPS is low
        // and the key was already released
        if (console == NULL || !console->IsVisible())
        {
            if (input->GetKeyDown(KEY_W))
                playerControls.Set(CTRL_UP, true);
            if (input->GetKeyDown(KEY_S))
                playerControls.Set(CTRL_DOWN, true);
            if (input->GetKeyDown(KEY_A))
                playerControls.Set(CTRL_LEFT, true);
            if (input->GetKeyDown(KEY_D))
                playerControls.Set(CTRL_RIGHT, true);
            if (input->GetKeyDown(KEY_LCTRL) || input->GetKeyPress(KEY_LCTRL))
                playerControls.Set(CTRL_FIRE, true);
            if (input->GetKeyDown(' ') || input->GetKeyPress(' '))
                playerControls.Set(CTRL_JUMP, true);

            if (input->GetMouseButtonDown(MOUSEB_LEFT) || input->GetMouseButtonDown(MOUSEB_LEFT))
                playerControls.Set(CTRL_FIRE, true);
            if (input->GetMouseButtonDown(MOUSEB_RIGHT) || input->GetMouseButtonDown(MOUSEB_RIGHT))
                playerControls.Set(CTRL_JUMP, true);

            playerControls.yaw_ += mouseSensitivity * input->GetMouseMoveX();
            playerControls.pitch_ += mouseSensitivity * input->GetMouseMoveY();
            playerControls.pitch_ = Clamp(playerControls.pitch_, -60.0f, 60.0f);
        }

        // In singleplayer, set controls directly on the player's ninja. In multiplayer, transmit to server
        if (singlePlayer)
        {
            Node* playerNode = scene_->GetChild("Player", true);
            if (playerNode != NULL)
            {
                Ninja* playerNinja = playerNode->GetComponent<Ninja>();
                playerNinja->controls = playerControls;
            }
        }
        else if (network->GetServerConnection() != NULL)
        {
            // Set the latest yaw & pitch to server controls, and accumulate the buttons so that we do not miss any presses
            network->GetServerConnection()->controls_.yaw_ = playerControls.yaw_;
            network->GetServerConnection()->controls_.pitch_ = playerControls.pitch_;
            network->GetServerConnection()->controls_.buttons_ |= playerControls.buttons_;

            // Tell the camera position to server for interest management
            network->GetServerConnection()->SetPosition(cameraNode_->GetWorldPosition());
        }
    }

    if (runServer)
    {
        // Apply each connection's controls to the ninja they control
        for (unsigned int i = 0; i < players.Size(); ++i)
        {
            Node* playerNode = FindPlayerNode(i);
            if (playerNode != NULL)
            {
                Ninja* playerNinja = playerNode->GetComponent<Ninja>();
                playerNinja->controls = players[i].connection->controls_;
            }
            else
            {
                // If player has no ninja, respawn if fire/jump is pressed
                if (players[i].connection->controls_.IsPressed(CTRL_FIRE | CTRL_JUMP, players[i].lastControls))
                    SpawnPlayer(players[i].connection);
            }
            players[i].lastControls = players[i].connection->controls_;
        }
    }
}

void Game::CheckEndAndRestart()
{
    // Only check end of game if singleplayer or client
    if (runServer)
        return;

    // Check if player node has vanished
    Node* playerNode = FindOwnNode();
    if (gameOn && playerNode == NULL)
    {
        gameOn = false;
        SetMessage("Press Fire or Jump to restart!");
        return;
    }

    // Check for restart (singleplayer only)FindPlayerNode
    if (!gameOn && singlePlayer && playerControls.IsPressed(CTRL_FIRE | CTRL_JUMP, prevPlayerControls))
        StartGame(NULL);
}

Node* Game::FindPlayerNode(int playerIndex)
{
    if (playerIndex >= 0 && playerIndex < int(players.Size()))
        return scene_->GetNode(players[playerIndex].nodeID);
    else
        return NULL;
}

Node* Game::FindOwnNode()
{
    if (singlePlayer)
        return scene_->GetChild("Player", true);
    else
        return scene_->GetNode(clientNodeID);
}

void Game::SpawnObjects(float timeStep)
{
    // If game not running, run only the random generator
    if (!gameOn)
    {
        Random();
        return;
    }

    // Spawn powerups
    powerupSpawnTimer += timeStep;
    if (powerupSpawnTimer >= powerupSpawnRate)
    {
        powerupSpawnTimer = 0;
        PODVector<Node*> destSnowCrate;
        PODVector<Node*> destPotion;
        scene_->GetChildrenWithComponent<SnowCrate>(destSnowCrate, true);
        scene_->GetChildrenWithComponent<Potion>(destPotion, true);

        int numPowerups = destPotion.Size() + destSnowCrate.Size();

        if (numPowerups < maxPowerups)
        {
            const float maxOffset = 40.0f;
            float xOffset = Random(maxOffset * 2.0f) - maxOffset;
            float zOffset = Random(maxOffset * 2.0f) - maxOffset;

            GameObject::SpawnObject(scene_, Vector3(xOffset, 50.0f, zOffset), Quaternion(), "SnowCrate");
        }
    }

    // Spawn enemies
    enemySpawnTimer += timeStep;
    if (enemySpawnTimer > enemySpawnRate)
    {
        enemySpawnTimer = 0;
        int numEnemies = 0;
        PODVector<Node*> ninjaNodes;
        scene_->GetChildrenWithComponent<Ninja>(ninjaNodes, true);
        for (unsigned int i = 0; i < ninjaNodes.Size(); ++i)
        {
            Ninja* ninja = ninjaNodes[i]->GetComponent<Ninja>();
            if (ninja->GetSide() == SIDE_ENEMY)
                ++numEnemies;
        }

        if (numEnemies < maxEnemies)
        {
            const float maxOffset = 40.0f;
            float offset = Random(maxOffset * 2.0f) - maxOffset;
            // Random north/east/south/west direction
            int dir = Random(1, 4);
            dir *= 90;
            Quaternion rotation(0.0f,(float) dir, 0.0f);

            Node* enemyNode = GameObject::SpawnObject(scene_, rotation * Vector3(offset, 10, -120), rotation, "Galtrilian");

            // Initialize variables
            Ninja* enemyNinja = enemyNode->GetComponent<Ninja>();
            enemyNinja->SetSide(SIDE_ENEMY);
            enemyNinja->controller = new AIController(context_);
            RigidBody* enemyBody = enemyNode->GetOrCreateComponent<RigidBody>();
            enemyBody->SetLinearVelocity(rotation * Vector3(0, 10, 30));
        }
    }
}

void Game::UpdateCamera()
{
    if (engine_->IsHeadless())
        return;

    // On the server, use a simple freelook camera
    if (runServer)
    {
        UpdateFreelookCamera();
        return;
    }

    Node* playerNode = FindOwnNode();
    if (playerNode == NULL)
        return;

    Vector3 pos = playerNode->GetPosition();
    Quaternion dir;

    // Make controls seem more immediate by forcing the current mouse yaw to player ninja's Y-axis rotation
    if (playerNode->GetVar("Health").GetInt() > 0)
        playerNode->SetRotation(Quaternion(0, playerControls.yaw_, 0));

    dir = dir * Quaternion(playerNode->GetRotation().YawAngle(), Vector3(0, 1, 0));
    dir = dir * Quaternion(playerControls.pitch_, Vector3(1, 0, 0));

    Vector3 aimPoint = pos + Vector3(0, 1, 0);
    Vector3 minDist = aimPoint + dir * Vector3(0, 0, -cameraMinDist);
    Vector3 maxDist = aimPoint + dir * Vector3(0, 0, -cameraMaxDist);

    // Collide camera ray with static objects (collision mask 2)
    Vector3 rayDir = (maxDist - minDist).Normalized();
    float rayDistance = cameraMaxDist - cameraMinDist + cameraSafetyDist;
    PhysicsRaycastResult result;

    scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, Ray(minDist, rayDir), rayDistance, 2);
    if (result.body_ != NULL)
        rayDistance = Min(rayDistance, result.distance_ - cameraSafetyDist);

    cameraNode_->SetPosition(minDist + rayDir * rayDistance);
    cameraNode_->SetRotation(dir);
}

void Game::UpdateFreelookCamera()
{
    Input* input = GetSubsystem<Input>();
    Console* console = GetSubsystem<Console>();
    Time* time = GetSubsystem<Time>();
    if (console == NULL || !console->IsVisible())
    {
        float timeStep = time->GetTimeStep();
        float speedMultiplier = 1.0f;
        if (input->GetKeyDown(KEY_LSHIFT))
            speedMultiplier = 5.0f;
        if (input->GetKeyDown(KEY_LCTRL))
            speedMultiplier = 0.1f;

        if (input->GetKeyDown(KEY_W))
            cameraNode_->Translate(Vector3(0, 0, 10) * timeStep * speedMultiplier);
        if (input->GetKeyDown(KEY_S))
            cameraNode_->Translate(Vector3(0, 0, -10) * timeStep * speedMultiplier);
        if (input->GetKeyDown(KEY_A))
            cameraNode_->Translate(Vector3(-10, 0, 0) * timeStep * speedMultiplier);
        if (input->GetKeyDown(KEY_D))
            cameraNode_->Translate(Vector3(10, 0, 0) * timeStep * speedMultiplier);

        playerControls.yaw_ += mouseSensitivity * input->GetMouseMoveX();
        playerControls.pitch_ += mouseSensitivity * input->GetMouseMoveY();
        playerControls.pitch_ = Clamp(playerControls.pitch_, -90.0f, 90.0f);
        cameraNode_->SetRotation(Quaternion(playerControls.pitch_, playerControls.yaw_, 0));
    }
}

void Game::UpdateStatus()
{
    if (engine_->IsHeadless() || runServer)
        return;

    if (singlePlayer)
    {
        if (players.Size() > 0)
            scoreText->SetText("Score " + String(players[0].score));
        if (hiscores.Size() > 0)
            hiscoreText->SetText("Hiscore " + String(hiscores[0].score));
    }

    Node* playerNode = FindOwnNode();
    if (playerNode != NULL)
    {
        int health = 0;
        if (singlePlayer)
        {
            GameObject* object = playerNode->GetDerivedComponent<GameObject>();
            health = object->GetHealth();
        }
        else
        {
            // In multiplayer the client does not have script logic components, but health is replicated via node user variables
            health = playerNode->GetVar("Health").GetInt();
        }
        healthBar->SetWidth(116 * health / playerHealth);
    }
}

int Game::FindPlayerIndex(unsigned int nodeID)
{
    for (unsigned int i = 0; i < players.Size(); ++i)
    {
        if (players[i].nodeID == nodeID)
            return i;
    }
    return -1;
}

bool Game::CheckHiscore(int playerIndex)
{
    for (unsigned int i = 0; i < hiscores.Size(); ++i)
    {
        if (hiscores[i].name == players[playerIndex].name)
        {
            if (players[playerIndex].score > hiscores[i].score)
            {
                hiscores[i].score = players[playerIndex].score;
                SortHiscores();
                return true;
            }
            else
                return false; // No update to individual hiscore
        }
    }

    // Not found, create new hiscore entry
    HiscoreEntry newHiscore;
    newHiscore.name = players[playerIndex].name;
    newHiscore.score = players[playerIndex].score;
    hiscores.Push(newHiscore);
    SortHiscores();
    return true;
}

void Game::SortHiscores()
{
    for (int i = 1; i < int(hiscores.Size()); ++i)
    {
        HiscoreEntry& temp = hiscores[i];
        int j = i;
        while (j > 0 && temp.score > hiscores[j - 1].score)
        {
            hiscores[j] = hiscores[j - 1];
            --j;
        }
        hiscores[j] = temp;
    }
}
