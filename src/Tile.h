#pragma once

// class Monster;

class Tile {
public:
	int x;
	int y;
	// Monster* monsterOnTile;
	bool isPassable;

	bool hasTreasure;
	bool isExit;

	bool isMouseOver;

	// Visibility Related
	bool bBlockVision = false;
	bool bWasTileDiscovered = false;
	bool bIsTileVisible = false;

	Tile(int _x, int _y, bool _passable);
	bool operator==(const Tile& rhs) const;

	// Debug
	int octant;
};