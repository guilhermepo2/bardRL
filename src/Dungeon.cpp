#include "Dungeon.h"
#include "Tile.h"

static const int FOV_MAX_DISTANCE = 9;

static int ConvertCoordinatesToIndex(int x, int y, int width, int height) {
	return (y * width) + x;
}

// ==========================
// Field of View Stuff
// https://journal.stuffwithstuff.com/2015/09/07/what-the-hero-sees/
// ==========================
void ShadowLine::AddShadowToLine(Shadow shadowToAdd) {
	// First we have to figure out where to slot the new shadow in the list
	int index = 0;
	for (; index < shadows.size(); index++) {
		// we stop when we hit an intersection point
		if (shadows[index].start >= shadowToAdd.start) {
			break;
		}
	}

	// we know where the shadow is going to be, now we check if it overlaps the previous or next
	Shadow previousOverlappedShadow;
	bool bFoundPrevious = false;
	if (index > 0 && shadows[index - 1].end > shadowToAdd.start) {
		previousOverlappedShadow = shadows[index - 1];
		bFoundPrevious = true;
	}

	Shadow nextOverlappedShadow;
	bool bFoundNext = false;
	if (index < shadows.size() && shadows[index].start < shadowToAdd.end) {
		nextOverlappedShadow = shadows[index];
		bFoundNext = true;
	}

	if (bFoundNext) {
		if (bFoundPrevious) {
			// overlaps both, so unify one and delete the other
			shadows[index - 1].end = shadows[index].end;
			shadows.erase(index);
		}
		else {
			// overlaps the next one, so unify it with that
			shadows[index].start = shadowToAdd.start;
		}
	}
	else {
		if (bFoundPrevious) {
			// overlaps the previous one, so unify it with that
			shadows[index - 1].end = shadowToAdd.end;
		}
		else {
			// does not overlap anything, so we have to insert
			shadows.insert(index, shadowToAdd);
		}
	}
}

bool ShadowLine::IsInShadow(Shadow projection) {
	for (int i = 0; i < shadows.size(); i++) {
		if (shadows[i].ContainsOther(projection)) {
			return true;
		}
	}

	return false;
}

// ==========================
// Dungeon Stuff
// ==========================
Dungeon::Dungeon() {
	m_roomMaxSize = 10;
	m_roomMinSize = 5;
	m_maxRooms = 30;

	m_width = 0;
	m_height = 0;
}
void Dungeon::GenerateLevel(int mapWidth, int mapHeight) {
	m_width = mapWidth;
	m_height = mapHeight;

	// 1. Create an empty 2D Array of Tiles.
	m_rooms.clear();
	m_map.clear();
	m_map.reserve(mapWidth * mapHeight);

	for (int i = 0; i < mapWidth * mapHeight; i++) {
		m_map.add(nullptr);
	}

	for (int i = 0; i < m_maxRooms; i++) {
		// random width and height
		int randomFactor1 = gueepo::rand::Int();
		int randomFactor2 = gueepo::rand::Int();
		int randomFactor3 = gueepo::rand::Int();
		int randomFactor4 = gueepo::rand::Int();
		randomFactor1 = gueepo::math::abs(randomFactor1);
		randomFactor2 = gueepo::math::abs(randomFactor2);
		randomFactor3 = gueepo::math::abs(randomFactor3);
		randomFactor4 = gueepo::math::abs(randomFactor4);

		int randomWidth = (m_roomMinSize + (randomFactor1 % m_roomMaxSize));
		int randomHeight = (m_roomMinSize + (randomFactor2 % m_roomMaxSize));
		
		// random position within map boundaries
		int randomX = randomFactor3 % (mapWidth - randomWidth - 1);
		int randomY = randomFactor4 % (mapHeight - randomHeight - 1);

		gueepo::math::rect newRoom(randomX, randomY, randomX + randomWidth, randomY + randomHeight);
		bool bFailed = false;
		for (int i = 0; i < m_rooms.size(); i++) {
			if (newRoom.Intersect(m_rooms[i])) {
				bFailed = true;
				break;
			}
		}

		if (!bFailed) {
			CreateRoom(newRoom);

			gueepo::math::vec2 newRoomSize = newRoom.GetSize();
			int newX = newRoom.bottomLeft.x + (newRoomSize.x / 2);
			int newY = newRoom.bottomLeft.y + (newRoomSize.y / 2);

			if (m_rooms.size() != 0) {
				// all rooms after the first one connect to the previous room
				// center coordinates of the previous room
				gueepo::math::rect previousRoom = m_rooms[m_rooms.size() - 1];
				gueepo::math::vec2 previousRoomSize = previousRoom.GetSize();
				int prevX = previousRoom.bottomLeft.x + (previousRoomSize.x / 2);
				int prevY = previousRoom.bottomLeft.y + (previousRoomSize.y / 2);

				// 50% chance that a tunnel will start horizontally
				if (gueepo::rand::Int() % 2 == 0) {
					CreateHorizontalTunnel(prevX, newX, prevY);
					CreateVerticalTunnel(prevY, newY, newX);
				}
				else {
					CreateVerticalTunnel(prevY, newY, prevX);
					CreateHorizontalTunnel(prevX, newX, newY);
				}
			}

			m_rooms.add(newRoom);
		}
	}

	// we are done... go through all the dungeon, if something is null then we create walls
	for (int x = 0; x < mapWidth; x++) {
		for (int y = 0; y < mapHeight; y++) {
			int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);

			if (m_map[index] == nullptr) {
				m_map[index] = new Tile(x, y, false);
			}
		}
	}
}

bool Dungeon::IsPositionValid(int x, int y) {
    return (((m_width * y) + x) < m_width * m_height) && (((m_width * y) + x) >= 0);
}

bool Dungeon::IsTilePassable(int x, int y) {
	int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);
	if (IsPositionValid(x, y) && m_map[index] != nullptr) {
		return m_map[index]->isPassable;
	}

	return false;
}

bool Dungeon::IsTileVisible(int x, int y) {
	int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);
	if (IsPositionValid(x, y) && m_map[index] != nullptr) {
		return m_map[index]->bIsTileVisible;
	}

	return false;
}

bool Dungeon::IsTileDiscovered(int x, int y) {
	int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);
	if (IsPositionValid(x, y) && m_map[index] != nullptr) {
		return m_map[index]->bWasTileDiscovered;
	}

	return false;
}

gueepo::math::vec2 Dungeon::GetStartingPosition() {
	g2dassert(m_rooms.size() != 0, "getting position when there's no rooms?");

	gueepo::math::vec2 position;
	gueepo::math::rect firstRoom = m_rooms[0];
	gueepo::math::vec2 firstRoomCenter = firstRoom.GetCenter();

	firstRoomCenter.x = static_cast<int>(firstRoomCenter.x);
	firstRoomCenter.y = static_cast<int>(firstRoomCenter.y);

	return firstRoomCenter;
}

Tile* Dungeon::GetTile(int x, int y) {
	Tile* t = nullptr;

	if (IsPositionValid(x, y)) {
		int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);
		t = m_map[index];
	}

	return t;
}

void Dungeon::CreateRoom(gueepo::math::rect newRoom) {
	for (int x = newRoom.bottomLeft.x; x < newRoom.topRight.x; x++) {
		for (int y = newRoom.bottomLeft.y; y < newRoom.topRight.y; y++) {
			if (!IsPositionValid(x, y)) {
				continue;
			}

			int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);

			if (m_map[index] == nullptr) {
				m_map[index] = new Tile(x, y, true);
			}
		}
	}
}

void Dungeon::CreateHorizontalTunnel(int x1, int x2, int y) {
	for (int x = gueepo::math::minimum(x1, x2); x < gueepo::math::maximum(x1, x2) + 1; x++) {
		
		if (!IsPositionValid(x, y)) {
			continue;
		}

		int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);
		
		if (m_map[index] == nullptr) {
			m_map[index] = new Tile(x, y, true);
		}
	}
}

void Dungeon::CreateVerticalTunnel(int y1, int y2, int x) {
	for (int y = gueepo::math::minimum(y1, y2); y < gueepo::math::maximum(y1, y2) + 1; y++) {

		if (!IsPositionValid(x, y)) {
			continue;
		}

		int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);

		if (m_map[index] == nullptr) {
			m_map[index] = new Tile(x, y, true);
		}
	}
}

// =================================================
// The intersection between dungeon and field of view.
// =================================================

void Dungeon::RefreshVisibility(int x, int y) {
	if (m_map.size() == 0) {
		return;
	}

	if (!IsPositionValid(x, y)) {
		return;
	}

	int startingTileIndex = ConvertCoordinatesToIndex(x, y, m_width, m_height);
	m_map[startingTileIndex]->bWasTileDiscovered = true;
	m_map[startingTileIndex]->bIsTileVisible = true;

	for (int octant = 0; octant < 8; octant++) {
		RefreshOctant(x, y, octant);
	}
}

void Dungeon::RefreshOctant(int x, int y, int octant) {
	ShadowLine line;
	bool fullShadow = false;

	for (int row = 1; row < FOV_MAX_DISTANCE; row++) {
		for (int col = 0; col <= row; col++) {
			gueepo::math::vec2 octantPosition = ConvertPositionToOctantPosition(row, col, octant);
			gueepo::math::vec2 position(x + octantPosition.x, y + octantPosition.y);
			int tileIndex = ConvertCoordinatesToIndex(position.x, position.y, m_width, m_height);

			// it can happen that the index is smaller than zero, or bigger than the room size
			// because the FoV will try to do its thing and look at positions
			// its up to us to tell it that there is nothing there...
			if (tileIndex < 0 || tileIndex >= m_map.size()) {
				continue;
			}

			if (m_map[tileIndex] == nullptr) {
				continue;
			}

			m_map[tileIndex]->octant = octant;

			if (fullShadow) {
				m_map[tileIndex]->bIsTileVisible = false;
			}
			else {
				Shadow projection = ProjectTile(row, col);
				bool bIsVisible = !line.IsInShadow(projection);
				m_map[tileIndex]->bIsTileVisible = bIsVisible;

				if (bIsVisible) {
					m_map[tileIndex]->bWasTileDiscovered = true;
				}

				if (bIsVisible && (!m_map[tileIndex]->isPassable || m_map[tileIndex]->bBlockVision)) {
					line.AddShadowToLine(projection);
					fullShadow = line.IsFullShadow();
				}
			}
		}
	}
}

gueepo::math::vec2 Dungeon::ConvertPositionToOctantPosition(int row, int col, int octant) {

	gueepo::math::vec2 retVal;

	switch (octant) {
	case 0:
		retVal.x = col;
		retVal.y = row;
		break;
	case 1:
		retVal.x = row;
		retVal.y = col;
		break;
	case 2:
		retVal.x = row;
		retVal.y = -col;
		break;
	case 3:
		retVal.x = col;
		retVal.y = -row;
		break;
	case 4:
		retVal.x = -col;
		retVal.y = -row;
		break;
	case 5:
		retVal.x = -row;
		retVal.y = -col;
		break;
	case 6:
		retVal.x = -row;
		retVal.y = col;
		break;
	case 7:
		retVal.x = -col;
		retVal.y = row;
		break;
	}
	
	return retVal;

}

Shadow Dungeon::ProjectTile(int row, int col) {
	float fRow = static_cast<float>(row);
	float fCol = static_cast<float>(col);

	float topLeft = (fCol / (fRow + 2));
	float bottomRight = (fCol + 1) / (fRow + 1);

	return Shadow(topLeft, bottomRight, gueepo::math::vec2(col, row+2), gueepo::math::vec2(col + 1, row + 1));
}
