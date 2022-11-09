#pragma once
#include <gueepo2d.h>

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

private:
	gueepo::OrtographicCamera* m_Camera;
};