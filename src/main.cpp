#define GUEEPO2D_MAIN
#include <gueepo2d.h>

class BardRoguelike : public gueepo::Application {
public:
	BardRoguelike() : Application("bardRL ", 640, 360) {}
	~BardRoguelike() {}
};

gueepo::Application* gueepo::CreateApplication() {
	return new BardRoguelike();
}
