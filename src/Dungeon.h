#pragma once
#include "gueepo2D.h"

class Tile;

// ==========================
// Field of View Stuff
// https://journal.stuffwithstuff.com/2015/09/07/what-the-hero-sees/
// ==========================
class Shadow {
public:
	float start;
	float end;

	Shadow() : start(0.0f), end(0.0f) {}
	Shadow(float _start, float _end) : start(_start), end(_end) {}
	inline bool ContainsOther(const Shadow& other) const { return (start <= other.start && end >= other.end); }
};

class ShadowLine {
public:
	gueepo::vector<Shadow> shadows;

	ShadowLine() {}

	inline bool IsFullShadow() const { return shadows.size() == 1 && shadows[0].start == 0 && shadows[0].end == 1; }
	void AddShadowToLine(Shadow shadowToAdd);
	bool IsInShadow(Shadow projection);

};

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

	void RefreshOctant(int x, int y, int octant);
	gueepo::math::vec2 ConvertPositionToOctantPosition(int row, int col, int octant);
	Shadow ProjectTile(int row, int col);

private:
	int m_roomMaxSize;
	int m_roomMinSize;
	int m_maxRooms;

	int m_width;
	int m_height;

	gueepo::vector<gueepo::math::rect> m_rooms;
	gueepo::vector<Tile*> m_map;
};