#pragma once
#include "gueepo2D.h"

class Tile;
struct Slope;

class Dungeon {
public:
	Dungeon();
	void GenerateLevel(int mapWidth, int mapHeight);

	bool IsTilePassable(int x, int y);
	bool IsTileVisible(int x, int y);
	bool IsTileDiscovered(int x, int y);
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	gueepo::math::vec2 GetStartingPosition();

	void RefreshVisibility(int x, int y);

protected:
	void CreateRoom(gueepo::math::rect newRoom);
	void CreateHorizontalTunnel(int x1, int x2, int y);
	void CreateVerticalTunnel(int y1, int y2, int x);

private:
	int m_roomMaxSize;
	int m_roomMinSize;
	int m_maxRooms;

	int m_width;
	int m_height;

	gueepo::vector<gueepo::math::rect> m_rooms;
	gueepo::vector<Tile*> m_map;
};