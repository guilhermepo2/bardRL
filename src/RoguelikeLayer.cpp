#include "RoguelikeLayer.h"

#include "Dungeon.h"

static gueepo::SpriteBatcher* batch = nullptr;
static gueepo::Texture* mainSpriteSheet = nullptr;
static gueepo::TextureRegion* heroSprite = nullptr;

static gueepo::TextureRegion* wallTexture = nullptr;
static gueepo::TextureRegion* floorTexture = nullptr;

static Dungeon* theDungeon = nullptr;

static const int DUNGEON_WIDTH = 60;
static const int DUNGEON_HEIGHT = 40;

void RoguelikeLayer::OnAttach() {
	gueepo::rand::Init();
	batch = new gueepo::SpriteBatcher();
	batch->Initialize();

	m_Camera = new gueepo::OrtographicCamera(1280, 720);
	m_Camera->SetBackgroundColor(0.1f, 0.1f, 0.1f, 1.0f);
	mainSpriteSheet = gueepo::Texture::Create("./assets/spritesheet.png");
	heroSprite = new gueepo::TextureRegion(mainSpriteSheet, 0, 0, 16, 16);
	wallTexture = new gueepo::TextureRegion(mainSpriteSheet, 16, 64, 16, 16);
	floorTexture = new gueepo::TextureRegion(mainSpriteSheet, 0, 64, 16, 16);

	// Creating the Dungeon
	theDungeon = new Dungeon();
	theDungeon->GenerateLevel(DUNGEON_WIDTH, DUNGEON_HEIGHT);
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
	gueepo::math::vec3 cameraPosition = m_Camera->GetPosition();
	float cameraSpeed = 0.01f;

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

	if (currentInputState.Keyboard.WasKeyPressedThisFrame(gueepo::Keycode::KEYCODE_R)) {
		theDungeon->GenerateLevel(DUNGEON_WIDTH, DUNGEON_HEIGHT);
	}
}

void RoguelikeLayer::OnEvent(gueepo::Event& e)
{

}

void RoguelikeLayer::OnRender() {
	gueepo::Color bgColor = m_Camera->GetBackGroundColor();
	gueepo::Renderer::Clear(bgColor.rgba[0], bgColor.rgba[1], bgColor.rgba[2], bgColor.rgba[3]);

	batch->Begin(*m_Camera);
	// batch->Draw(heroSprite, 0, 0, 32, 32);

	for (int x = 0; x < theDungeon->GetWidth(); x++) {
		for (int y = 0; y < theDungeon->GetHeight(); y++) {
			if (theDungeon->IsTilePassable(x, y)) {
				batch->Draw(floorTexture, x * 16, y * 16, 16, 16);
			}
			else {
				batch->Draw(wallTexture, x * 16, y * 16, 16, 16);
			}
		}
	}

	batch->End();
}

void RoguelikeLayer::OnImGuiRender()
{

}

