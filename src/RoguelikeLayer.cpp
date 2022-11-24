#include "RoguelikeLayer.h"
#include "Dungeon.h"
#include "Tile.h"
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

// fonts
static gueepo::FontSprite* dogicaFont = nullptr;

// Hero
static gueepo::math::vec2 heroPosition;

static const int DUNGEON_WIDTH = 60;
static const int DUNGEON_HEIGHT = 40;

static gueepo::math::vec2 MouseToWorldPosition(gueepo::math::vec2 mousePosition, gueepo::OrtographicCamera camera) {
	gueepo::math::vec2 worldPosition(mousePosition.x, mousePosition.y);
	gueepo::math::vec3 cameraPosition = camera.GetPosition();

	int ScreenToCameraRatioX = WINDOW_WIDTH / CAMERA_SIZE_X;
	int ScreenToCameraRatioY = WINDOW_HEIGHT / CAMERA_SIZE_Y;

	// (0) world is bottom up, mouse is originally top down.
	worldPosition.y = WINDOW_HEIGHT - worldPosition.y;

	// (1) First Thing, translating the mouse position into the realm of the camera size.
	worldPosition.x /= ScreenToCameraRatioX;
	worldPosition.y /= ScreenToCameraRatioY;

	// (2) We have to move half the camera to the left and down.
	// This sort of makes sense but I'm not certain about the reasoning.
	worldPosition.x -= (CAMERA_SIZE_X / 2);
	worldPosition.y -= (CAMERA_SIZE_Y / 2);

	
	// here's how the camera works...the camera maps the screen size in between -1 and 1 (so, 2)
	// so if the camera is, let's say, 1.0 moved to the x it moved 320 pixels (if the screen is 640 pixels) to the x
	int xOffsetInPixels = (CAMERA_SIZE_X * cameraPosition.x) / 2;
	int yOffsetInPixels = (CAMERA_SIZE_Y * cameraPosition.y) / 2;

	worldPosition.x += xOffsetInPixels;
	worldPosition.y += yOffsetInPixels;

	return worldPosition;
}

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

	{
		GUEEPO2D_SCOPED_TIMER("loading fonts");
		gueepo::Font* dogica = gueepo::Font::CreateFont("./assets/dogica.ttf");
		dogicaFont = new gueepo::FontSprite(dogica, 8);
		dogicaFont->SetLineGap(4);
	}
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
	/*
	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_R)) {
		theDungeon->GenerateLevel(DUNGEON_WIDTH, DUNGEON_HEIGHT);
		heroPosition = theDungeon->GetStartingPosition();
		CenterCameraOnPosition(heroPosition * TILE_SIZE);
	}
	*/

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

	// Mouse World Position
	gueepo::math::vec2 mouseWorldPosition = MouseToWorldPosition(currentInputState.Mouse.GetPosition(), *m_Camera);

	// Updating all tiles to see if the mouse is over it. very inneficient, but...
	Tile* mouseOverTile = nullptr;
	for (int x = 0; x < theDungeon->GetWidth(); x++) {
		for (int y = 0; y < theDungeon->GetHeight(); y++) {
			Tile* t = theDungeon->GetTile(x, y);
			gueepo::math::vec2 tileWorldPosition(t->x * TILE_SIZE, t->y * TILE_SIZE);
			gueepo::math::vec2 tileMinPosition(tileWorldPosition.x - TILE_SIZE / 2, tileWorldPosition.y - TILE_SIZE / 2);
			gueepo::math::vec2 tileMaxPosition(tileWorldPosition.x + TILE_SIZE / 2, tileWorldPosition.y + TILE_SIZE / 2);

			// checking if mouse is in this tile...
			if (
				tileMinPosition.x <= mouseWorldPosition.x && tileMaxPosition.x >= mouseWorldPosition.x &&
				tileMinPosition.y <= mouseWorldPosition.y && tileMaxPosition.y >= mouseWorldPosition.y
				) {
				t->isMouseOver = true;
				mouseOverTile = t;
			}
			else {
				t->isMouseOver = false;
			}
		}
	}

	if (currentInputState.Mouse.WasMouseKeyPressedThisFrame(gueepo::Mousecode::MOUSE_LEFT)) {
		if (mouseOverTile != nullptr && mouseOverTile->isPassable && mouseOverTile->bIsTileVisible) {
			LOG_INFO("clicked on tile: ({0}, {1})", mouseOverTile->x, mouseOverTile->y);
		}
	}

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
					Tile* t = theDungeon->GetTile(x, y);
					if (t->isMouseOver) {
						// #todo: ideally this would be its own sprite over the tile, but for now all we have is a red color...
						batch->Draw(floorTexture, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, gueepo::Color(1.0f, 0.0f, 1.0f, 1.0f));
					}
					else {
						batch->Draw(floorTexture, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
					}
				}
				else {
					batch->Draw(wallTexture, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
				}
			}
			else if (theDungeon->IsTileDiscovered(x, y)) {
				if (theDungeon->IsTilePassable(x, y)) {
					batch->Draw(floorTexture, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
				}
				else {
					batch->Draw(wallTexture, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
				}

				batch->Draw(partiallyVisible, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
			}


			/*
			Tile* t = theDungeon->GetTile(x, y);
			if (t != nullptr) {
				int octant = t->octant;
				if (octant >= 0 && octant < 8) {
					std::string octantText = std::to_string(t->octant);
					batch->DrawText(dogicaFont, octantText.c_str(), gueepo::math::vec2((x * TILE_SIZE) - 4, (y * TILE_SIZE) - 4), 1.0f, gueepo::Color(1.0f, 1.0f, 1.0f, 1.0f));
				}
			}
			*/
			
		}
	}

	// #todo: not really sure why I have to offset this here.
	// dungeon tiles and the player are drawn and rendered the exact same way
	batch->Draw(heroSprite, (heroPosition.x * TILE_SIZE), (heroPosition.y * TILE_SIZE), TILE_SIZE, TILE_SIZE);

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

