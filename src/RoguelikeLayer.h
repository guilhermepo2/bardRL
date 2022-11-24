#pragma once
#include <gueepo2d.h>

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

class RoguelikeLayer : public gueepo::Layer {
public:
	RoguelikeLayer() : Layer("roguelike layer") {}

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float DeltaTime) override;
	void OnInput(const gueepo::InputState & currentInputState) override;
	void OnEvent(gueepo::Event& e) override;
	void OnRender() override;
	void OnImGuiRender() override;

protected:
	void CenterCameraOnPosition(gueepo::math::vec2 position);
private:
	gueepo::OrtographicCamera* m_Camera;
};