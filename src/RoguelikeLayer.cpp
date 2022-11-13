#include "RoguelikeLayer.h"
#include "Dungeon.h"
#include <imgui.h>

static const int CAMERA_SIZE_X = 320;
static const int CAMERA_SIZE_Y = 180;
static const int TILE_SIZE = 16;

static gueepo::SpriteBatcher* batch = nullptr;
static gueepo::Texture* mainSpriteSheet = nullptr;
static gueepo::TextureRegion* heroSprite = nullptr;

static gueepo::TextureRegion* wallTexture = nullptr;
static gueepo::TextureRegion* floorTexture = nullptr;
static gueepo::TextureRegion* partiallyVisible = nullptr;

static Dungeon* theDungeon = nullptr;

// Hero
static gueepo::math::vec2 heroPosition;

static const int DUNGEON_WIDTH = 60;
static const int DUNGEON_HEIGHT = 40;

void RoguelikeLayer::OnAttach() {
	gueepo::rand::Init();
	batch = new gueepo::SpriteBatcher();
	batch->Initialize();

	m_Camera = new gueepo::OrtographicCamera(CAMERA_SIZE_X, CAMERA_SIZE_Y);
	m_Camera->SetBackgroundColor(0.1f, 0.1f, 0.1f, 1.0f);

	mainSpriteSheet = gueepo::Texture::Create("./assets/spritesheet.png");
	heroSprite = new gueepo::TextureRegion(mainSpriteSheet, 0, 0, 16, 16);
	wallTexture = new gueepo::TextureRegion(mainSpriteSheet, 16, 64, 16, 16);
	floorTexture = new gueepo::TextureRegion(mainSpriteSheet, 0, 64, 16, 16);
	partiallyVisible = new gueepo::TextureRegion(mainSpriteSheet, 32, 64, 16, 16);

	// Creating the Dungeon
	theDungeon = new Dungeon();
	theDungeon->GenerateLevel(DUNGEON_WIDTH, DUNGEON_HEIGHT);
	heroPosition = theDungeon->GetStartingPosition();
	CenterCameraOnPosition(heroPosition * 16);
	theDungeon->RefreshVisibility(heroPosition.x, heroPosition.y);
}

void RoguelikeLayer::OnDetach()
{
	delete m_Camera;
	delete heroSprite;
	delete mainSpriteSheet;

	batch->Shutdown();
	delete batch;

	// MemoryTracker_CheckForLeaks(); // might give false negative.
}

void RoguelikeLayer::OnUpdate(float DeltaTime) {
	
}

void RoguelikeLayer::OnInput(const gueepo::InputState& currentInputState) {

	// Handling Camera Movement...
	gueepo::math::vec3 cameraPosition = m_Camera->GetPosition();
	float cameraSpeed = 0.00f;

	if(currentInputState.Keyboard.IsKeyDown(gueepo::Keycode::KEYCODE_W)) {
		cameraPosition.y += cameraSpeed;
	}
	else if (currentInputState.Keyboard.IsKeyDown(gueepo::Keycode::KEYCODE_S)) {
		cameraPosition.y -= cameraSpeed;
	}
	else if (currentInputState.Keyboard.IsKeyDown(gueepo::Keycode::KEYCODE_A)) {
		cameraPosition.x -= cameraSpeed;
	}
	else if (currentInputState.Keyboard.IsKeyDown(gueepo::Keycode::KEYCODE_D)) {
		cameraPosition.x += cameraSpeed;
	}

	m_Camera->SetPosition(cameraPosition);

	// Regenerating the dungeon for visualization purposes...
	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_R)) {
		theDungeon->GenerateLevel(DUNGEON_WIDTH, DUNGEON_HEIGHT);
		heroPosition = theDungeon->GetStartingPosition();
		CenterCameraOnPosition(heroPosition * TILE_SIZE);
	}

	// moving the hero
	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_W)) {
		heroPosition.y += 1;
		theDungeon->RefreshVisibility(heroPosition.x, heroPosition.y);
	}
	else if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_S)) {
		heroPosition.y -= 1;
		theDungeon->RefreshVisibility(heroPosition.x, heroPosition.y);
	}
	else if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_A)) {
		heroPosition.x -= 1;
		theDungeon->RefreshVisibility(heroPosition.x, heroPosition.y);
	}
	else if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_D)) {
		heroPosition.x += 1;
		theDungeon->RefreshVisibility(heroPosition.x, heroPosition.y);
	}

	CenterCameraOnPosition(heroPosition * TILE_SIZE);

}

void RoguelikeLayer::OnEvent(gueepo::Event& e)
{

}

void RoguelikeLayer::OnRender() {
	gueepo::Color bgColor = m_Camera->GetBackGroundColor();
	gueepo::Renderer::Clear(bgColor.rgba[0], bgColor.rgba[1], bgColor.rgba[2], bgColor.rgba[3]);

	batch->Begin(*m_Camera);

	for (int x = 0; x < theDungeon->GetWidth(); x++) {
		for (int y = 0; y < theDungeon->GetHeight(); y++) {

			if (theDungeon->IsTileVisible(x,y)) {
				if (theDungeon->IsTilePassable(x, y)) {
					batch->Draw(floorTexture, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
				}
				else {
					batch->Draw(wallTexture, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
				}
			}
			else if (theDungeon->IsTileDiscovered(x, y)) {
				batch->Draw(partiallyVisible, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
			}
		}
	}

	// #todo: not really sure why I have to offset this here.
	// dungeon tiles and the player are drawn and rendered the exact same way
	batch->Draw(heroSprite, (heroPosition.x * TILE_SIZE) - 8, (heroPosition.y * TILE_SIZE) - 8, TILE_SIZE, TILE_SIZE);

	batch->End();
}

void RoguelikeLayer::OnImGuiRender() {
	ImGui::Begin("debug");
	ImGui::LabelText("debug", "debug");
	ImGui::End();
}

void RoguelikeLayer::CenterCameraOnPosition(gueepo::math::vec2 position) {
	// here's how the camera works...the camera maps the screen size in between -1 and 1 (so, 2)
	// so if the camera is, let's say, 1.0 moved to the x it moved 320 pixels (if the screen is 640 pixels) to the x

	float halfWidth = CAMERA_SIZE_X / 2.0f;
	float halfHeight = CAMERA_SIZE_Y / 2.0f;

	// if the width is 640, I have to move 0.1 for every 32 pixels on the position...

	// if position.x == 32, then cameraX = 0.1 - what's the math here?
	// 32 / 320 -> so position.x / halfWidth
	float cameraX = position.x / halfWidth;
	float cameraY = position.y / halfHeight;

	m_Camera->SetPosition(gueepo::math::vec3(cameraX, cameraY, m_Camera->GetPosition().z));
}

