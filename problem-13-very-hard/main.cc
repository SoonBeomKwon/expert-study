#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstring>
#include <iostream>

#include "type.h"

extern void init(void);
extern void cleanHouse();
extern Time GetTimeChecker();

static unsigned long long seed = 5;

int num_call_moves = 0;
int num_call_scans = 0;
int num_call_turns = 0;

static int psuedo_rand(void) {
  seed = seed * 25214903917ULL + 11ULL;
  return (seed >> 16) & 0x3fffffff;
}

static const int N = 64;
static const int TC_COUNT = 10;
static const int SUB_TASK_COUNT = 10;

static const long long PENALTY = 1'234'567'000;
static long long SCORE = 0;

static const int MOVE_COST = 10;
static const int TURN_COST = 15;
static const int SCAN_COST = 20;

static const int dy[4] = {-1, 0, 1, 0};
static const int dx[4] = {0, -1, 0, 1};

static int houseInfo[N][N];
static int isCleaned[N][N];

static int robotY;
static int robotX;
static int robotDir;

void scan(int floorState[3][3]) {
  num_call_scans += 1;
  SCORE += SCAN_COST;

  switch (robotDir) {
    case 0:  // UP
      for (int y = robotY - 1, sy = 0; y <= robotY + 1; ++y, ++sy) {
        for (int x = robotX - 1, sx = 0; x <= robotX + 1; ++x, ++sx) {
          floorState[sy][sx] = houseInfo[y][x];
        }
      }
      break;
    case 1:  // LEFT
      for (int y = robotY - 1, sx = 2; y <= robotY + 1; ++y, --sx) {
        for (int x = robotX - 1, sy = 0; x <= robotX + 1; ++x, ++sy) {
          floorState[sy][sx] = houseInfo[y][x];
        }
      }
      break;
    case 2:  // DOWN
      for (int y = robotY - 1, sy = 2; y <= robotY + 1; ++y, --sy) {
        for (int x = robotX - 1, sx = 2; x <= robotX + 1; ++x, --sx) {
          floorState[sy][sx] = houseInfo[y][x];
        }
      }
      break;
    case 3:  // RIGHT
      for (int y = robotY - 1, sx = 0; y <= robotY + 1; ++y, ++sx) {
        for (int x = robotX - 1, sy = 2; x <= robotX + 1; ++x, --sy) {
          floorState[sy][sx] = houseInfo[y][x];
        }
      }
      break;
  }
}

int move(void) {
  num_call_moves += 1;
  SCORE += MOVE_COST;

  int next_y = robotY + dy[robotDir];
  int next_x = robotX + dx[robotDir];

  if (houseInfo[next_y][next_x] == 1) {
    return 0;
  }

  robotY = next_y;
  robotX = next_x;
  isCleaned[robotY][robotX] = 1;

  return 1;
}

void turn(int mCommand) {
  num_call_turns += 1;
  SCORE += TURN_COST;

  if (mCommand <= 0 || mCommand >= 4) {
    return;
  }

  robotDir = (robotDir + mCommand) % 4;
}

static bool run(int tc) {
  init();

  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      scanf("%d", &houseInfo[y][x]);
    }
  }

  for (int i = 0; i < 10; ++i) {
    do {
      robotY = psuedo_rand() % (N - 2) + 1;
      robotX = psuedo_rand() % (N - 2) + 1;
      robotDir = psuedo_rand() % 4;
    } while (houseInfo[robotY][robotX] == 1);

    memset(isCleaned, 0, sizeof(isCleaned));

    isCleaned[robotY][robotX] = 1;

    std::cout << "robot pose: (" << robotX << ", " << robotY << ", " << robotDir
              << ")\n";
    cleanHouse();

    for (int y = 1; y < N - 1; ++y) {
      for (int x = 1; x < N - 1; ++x) {
        if (houseInfo[y][x] == 0 && isCleaned[y][x] == 0) {
          return false;
        }
      }
    }
  }

  return true;
}

int main() {
  freopen("rv4.txt", "r", stdin);
  for (int tc = 1; tc <= TC_COUNT; ++tc) {
    std::cout << "tc: " << tc << "\n";
    if (run(tc) == false) {
      printf("SCORE: %lld\n", PENALTY);
      printf("Failed %d (exited)..\n", tc);
      return 0;
    }
  }

  if (SCORE <= 5250000) {
    printf("SCORE: %lld\n", SCORE);
    printf("PASSED\n");

    auto time_checker = GetTimeChecker();
    std::cout << "time check\n";
    std::cout << "Relocalize: " << time_checker.relocalize << "\n";
    std::cout << "ResetMap: " << time_checker.reset_map << "\n";
    std::cout << "ExistUncoverFreeCell: "
              << time_checker.exist_uncover_free_cell << "\n";
    std::cout << "FindNextCellForFreeCell: "
              << time_checker.find_next_cell_for_free_cell << "\n";
    std::cout << "FindNextCellForUnknownCell: "
              << time_checker.find_next_cell_for_unknown_cell << "\n";
    std::cout << "CheckEndCOndition: " << time_checker.check_end_condition
              << "\n";
  } else {
    printf("SCORE: %lld\n", SCORE);
  }

  return 0;
}
