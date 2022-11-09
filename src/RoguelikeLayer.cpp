#include "RoguelikeLayer.h"

static gueepo::SpriteBatcher* batch = nullptr;
static gueepo::Texture* mainSpriteSheet = nullptr;
static gueepo::TextureRegion* heroSprite = nullptr;

void RoguelikeLayer::OnAttach() {
	gueepo::rand::Init();
	batch = new gueepo::SpriteBatcher();
	batch->Initialize();

	m_Camera = new gueepo::OrtographicCamera(640, 360);
	m_Camera->SetBackgroundColor(0.1f, 0.1f, 0.1f, 1.0f);
	mainSpriteSheet = gueepo::Texture::Create("./assets/spritesheet.png");
	heroSprite = new gueepo::TextureRegion(mainSpriteSheet, 0, 0, 16, 16);
}

void RoguelikeLayer::OnDetach()
{
	delete m_Camera;
	delete heroSprite;
	delete mainSpriteSheet;

	batch->Shutdown();
	delete batch;
}

void RoguelikeLayer::OnUpdate(float DeltaTime)
{

}

void RoguelikeLayer::OnInput(const gueepo::InputState& currentInputState)
{

}

void RoguelikeLayer::OnEvent(gueepo::Event& e)
{

}

void RoguelikeLayer::OnRender() {
	gueepo::Color bgColor = m_Camera->GetBackGroundColor();
	gueepo::Renderer::Clear(bgColor.rgba[0], bgColor.rgba[1], bgColor.rgba[2], bgColor.rgba[3]);

	batch->Begin(*m_Camera);
	batch->Draw(heroSprite, 0, 0, 32, 32);
	batch->End();
}

void RoguelikeLayer::OnImGuiRender()
{

}

