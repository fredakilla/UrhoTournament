#ifndef GAME_H
#define GAME_H

#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Urho3D.h>
#include <Urho3D/Input/Controls.h>

namespace Urho3D
{
	class Sprite;
	class Drawable;
	class Node;
	class Scene;
	class SoundSource;
	class BorderImage;
	class Text;
	class Camera;
	class Connection;
	class Engine;
}

class Player;
class HiscoreEntry;

using namespace Urho3D;

///Game Events.
URHO3D_EVENT(GE_PLAYERSPAWNED, PlayerSpawned)
{
	
    URHO3D_PARAM(P_NODEID, NodeID); // int
}
URHO3D_EVENT(GE_UPDATESCORE, UpdateScore)
{
    URHO3D_PARAM(P_SCORE, Score); // int
}

URHO3D_EVENT(GE_UPDATEHISCORE, UpdateHiscores)
{
    URHO3D_PARAM(P_HISCORES, Hiscores);  // const PODVector<unsigned char>&
}

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

class Game : public Application
{
    URHO3D_OBJECT(Game, Application)

public:
	/// Construct.
	Game(Context* context);

	/// Setup after engine initialization and before running the main loop.
	virtual void Start();
	/// Setup before engine initialization. Modifies the engine parameters.
	virtual void Setup();
	/// Cleanup after the main loop. Called by Application.
	virtual void Stop();
protected:
	/// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
	virtual String GetScreenJoystickPatchString() const {
		return
			"<patch>"
			"    <add sel=\"/element\">"
			"        <element type=\"Button\">"
			"            <attribute name=\"Name\" value=\"Button3\" />"
			"            <attribute name=\"Position\" value=\"-120 -120\" />"
			"            <attribute name=\"Size\" value=\"96 96\" />"
			"            <attribute name=\"Horiz Alignment\" value=\"Right\" />"
			"            <attribute name=\"Vert Alignment\" value=\"Bottom\" />"
			"            <attribute name=\"Texture\" value=\"Texture2D;Textures/TouchInput.png\" />"
			"            <attribute name=\"Image Rect\" value=\"96 0 192 96\" />"
			"            <attribute name=\"Hover Image Offset\" value=\"0 0\" />"
			"            <attribute name=\"Pressed Image Offset\" value=\"0 0\" />"
			"            <element type=\"Text\">"
			"                <attribute name=\"Name\" value=\"Label\" />"
			"                <attribute name=\"Horiz Alignment\" value=\"Center\" />"
			"                <attribute name=\"Vert Alignment\" value=\"Center\" />"
			"                <attribute name=\"Color\" value=\"0 0 0 1\" />"
			"                <attribute name=\"Text\" value=\"Teleport\" />"
			"            </element>"
			"            <element type=\"Text\">"
			"                <attribute name=\"Name\" value=\"KeyBinding\" />"
			"                <attribute name=\"Text\" value=\"LSHIFT\" />"
			"            </element>"
			"            <element type=\"Text\">"
			"                <attribute name=\"Name\" value=\"MouseButtonBinding\" />"
			"                <attribute name=\"Text\" value=\"LEFT\" />"
			"            </element>"
			"        </element>"
			"        <element type=\"Button\">"
			"            <attribute name=\"Name\" value=\"Button4\" />"
			"            <attribute name=\"Position\" value=\"-120 -12\" />"
			"            <attribute name=\"Size\" value=\"96 96\" />"
			"            <attribute name=\"Horiz Alignment\" value=\"Right\" />"
			"            <attribute name=\"Vert Alignment\" value=\"Bottom\" />"
			"            <attribute name=\"Texture\" value=\"Texture2D;Textures/TouchInput.png\" />"
			"            <attribute name=\"Image Rect\" value=\"96 0 192 96\" />"
			"            <attribute name=\"Hover Image Offset\" value=\"0 0\" />"
			"            <attribute name=\"Pressed Image Offset\" value=\"0 0\" />"
			"            <element type=\"Text\">"
			"                <attribute name=\"Name\" value=\"Label\" />"
			"                <attribute name=\"Horiz Alignment\" value=\"Center\" />"
			"                <attribute name=\"Vert Alignment\" value=\"Center\" />"
			"                <attribute name=\"Color\" value=\"0 0 0 1\" />"
			"                <attribute name=\"Text\" value=\"Obstacles\" />"
			"            </element>"
			"            <element type=\"Text\">"
			"                <attribute name=\"Name\" value=\"MouseButtonBinding\" />"
			"                <attribute name=\"Text\" value=\"MIDDLE\" />"
			"            </element>"
			"        </element>"
			"    </add>"
			"    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/attribute[@name='Is Visible']\" />"
			"    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Set</replace>"
			"    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]\">"
			"        <element type=\"Text\">"
			"            <attribute name=\"Name\" value=\"MouseButtonBinding\" />"
			"            <attribute name=\"Text\" value=\"LEFT\" />"
			"        </element>"
			"    </add>"
			"    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/attribute[@name='Is Visible']\" />"
			"    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Debug</replace>"
			"    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]\">"
			"        <element type=\"Text\">"
			"            <attribute name=\"Name\" value=\"KeyBinding\" />"
			"            <attribute name=\"Text\" value=\"SPACE\" />"
			"        </element>"
			"    </add>"
			"</patch>";
	}

private:
	void InitAudio();
	void InitConsole();
	void InitScene();
	void InitNetworking();
	void InitTouchInput();

	void CreateCamera();
	void CreateOverlays();

	void StartGame(Connection* connection);

	void SpawnPlayer(Connection* connection);
	void SpawnObjects(float timeStep);

	void SendScore(int playerIndex);
	void SendHiscores(int playerIndex);

	void UpdateCamera();
	void UpdateFreelookCamera();
	void UpdateControls();
	void UpdateStatus();

	bool CheckHiscore(int playerIndex);
	void CheckEndAndRestart();
	void SortHiscores();

	void SetMessage(const String& message);

	int FindPlayerIndex(unsigned int nodeID);
	Node* FindPlayerNode(int id);
	Node* FindOwnNode();


	/// Handle the logic update event.
	void HandleUpdate(StringHash eventType, VariantMap& eventData);
	/// Handle the post-render update event.
	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
	void HandleFixedUpdate(StringHash eventType, VariantMap& eventData);
	void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
	/// Handle key down event to process key controls common to all samples.
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	/// Handle scene update event to control camera's pitch and yaw for all samples.
	void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
	/// Handle touch begin event to initialize touch input on desktop platform.
	void HandleTouchBegin(StringHash eventType, VariantMap& eventData);
	void HandleScreenMode(StringHash eventType, VariantMap& eventData);

	void HandlePoints(StringHash eventType, VariantMap& eventData);
	void HandleKill(StringHash eventType, VariantMap& eventData);

	void HandleClientIdentity(StringHash eventType, VariantMap& eventData);
	void HandleClientSceneLoaded(StringHash eventType, VariantMap& eventData);
	void HandleClientDisconnected(StringHash eventType, VariantMap& eventData);

	void HandlePlayerSpawned(StringHash eventType, VariantMap& eventData);
	void HandleUpdateScore(StringHash eventType, VariantMap& eventData);
	void HandleUpdateHiscores(StringHash eventType, VariantMap& eventData);

	void HandleNetworkUpdateSent(StringHash eventType, VariantMap& eventData);
	

	
	/// Scene.
	SharedPtr<Scene> scene_;
	/// Camera scene node.
	SharedPtr<Node> cameraNode_;
	/// Camera yaw angle.
	float yaw_;
	/// Camera pitch angle.
	float pitch_;
	/// Flag to indicate whether touch input has been enabled.
	bool touchEnabled_;
	/// Screen joystick index for navigational controls (mobile platforms only).
	unsigned screenJoystickIndex_;
	/// Screen joystick index for settings (mobile platforms only).
	unsigned screenJoystickSettingsIndex_;
	//////////////////////////////////////////////////////////////////////////
	/// Network stuff
	//////////////////////////////////////////////////////////////////////////
	bool runServer;
	bool runClient;
	String serverAddress;
	uint16_t serverPort;
	String userName;
	bool nobgm;
	void ParseNetworkArguments();
	//////////////////////////////////////////////////////////////////////////
	/// Game Stuff
	//////////////////////////////////////////////////////////////////////////
	SharedPtr<Node> musicNode;
	SharedPtr<Camera> gameCamera;
	SharedPtr<Text> scoreText;
	SharedPtr<Text> hiscoreText;
	SharedPtr<Text> messageText;
	SharedPtr<BorderImage> healthBar;
	SharedPtr<BorderImage> sight;
	SharedPtr<BorderImage> moveButton;
	SharedPtr<BorderImage> fireButton;
	SharedPtr<SoundSource> musicSource;

	Controls playerControls;
	Controls prevPlayerControls;
	bool singlePlayer;
	bool gameOn;
	bool drawDebug;
	bool drawOctreeDebug;
	int maxEnemies;
	int incrementCounter;
	float enemySpawnTimer;
	float powerupSpawnTimer;
	unsigned clientNodeID;
	int clientScore;

	Vector<Player> players;
	Vector<HiscoreEntry> hiscores;

	Engine* engine_;
};

#endif // GAME_H
