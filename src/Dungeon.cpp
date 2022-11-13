#include "Dungeon.h"
#include "Tile.h"

static int ConvertCoordinatesToIndex(int x, int y, int width, int height) {
	return (y * width) + x;
}

static bool IsValidCoordinate(int x, int y, int width, int height) {
	return ((width * y) + x) < width * height;
}

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
}

bool Dungeon::IsTilePassable(int x, int y) {
	int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);
	if (IsValidCoordinate(x, y, m_width, m_height) && m_map[index] != nullptr) {
		return m_map[index]->isPassable;
	}

	return false;
}

void Dungeon::CreateRoom(gueepo::math::rect newRoom) {
	for (int x = newRoom.bottomLeft.x; x < newRoom.topRight.x; x++) {
		for (int y = newRoom.bottomLeft.y; y < newRoom.topRight.y; y++) {
			if (!IsValidCoordinate(x, y, m_width, m_height)) {
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
		
		if (!IsValidCoordinate(x, y, m_width, m_height)) {
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

		if (!IsValidCoordinate(x, y, m_width, m_height)) {
			continue;
		}

		int index = ConvertCoordinatesToIndex(x, y, m_width, m_height);

		if (m_map[index] == nullptr) {
			m_map[index] = new Tile(x, y, true);
		}
	}
}
