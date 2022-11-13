#define GUEEPO2D_MAIN
#include <gueepo2d.h>
#include "RoguelikeLayer.h"

#include "Tile.h"

// Who needs project management software?
// How do you make a roguelike? https://www.gamedeveloper.com/design/how-to-make-a-roguelike
// 1. character
// 2. map generation
// 3. FoV
// 4. pathfinding
// 5. monsters
// 6. combat!
// 7. items
// 8. GUI
// 9. saving (?)

class BardRoguelike : public gueepo::Application {
public:
	BardRoguelike() : Application("bardRL ", 1280, 720) {
		PushLayer(new RoguelikeLayer());
	}
	~BardRoguelike() {}
};

gueepo::Application* gueepo::CreateApplication() {
	return new BardRoguelike;
}
