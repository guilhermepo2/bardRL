#include "Tile.h"

Tile::Tile(int _x, int _y, bool _passable) : x(_x), y(_y), isPassable(_passable), hasTreasure(false), isExit(false) {}

bool Tile::operator==(const Tile& rhs) const {
	return this->x == rhs.x &&
		this->y && rhs.y &&
		// this->monsterOnTile == rhs.monsterOnTile &&
		this->hasTreasure == rhs.hasTreasure &&
		this->isExit == rhs.isExit;
}