// 5210485
extern void scan(int floorState[3][3]);
extern int move(void);
extern void turn(int mCommand);

static const int N = 128;
static int obs_map[N][N] = {
    -1,
};
static bool visited_map[N][N] = {
    false,
};
static int left_top_x = N / 2;
static int left_top_y = N / 2;
static int right_bottom_x = N / 2;
static int right_bottom_y = N / 2;

static const int dx[4] = {0, -1, 0, 1};
static const int dy[4] = {-1, 0, 1, 0};

static const int ddx[8] = {0, -1, -1, -1, 0, 1, 1, 1};
static const int ddy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

static int robot_x = N / 2;
static int robot_y = N / 2;
static int robot_heading = 0;

struct Cell {
  int x = 0;
  int y = 0;
  int heading = 0;
  Cell* parent_cell = nullptr;
};

static Cell heap_cell[1000000];
static int heap_count = 0;

Cell* BringHeap() { return &heap_cell[heap_count++]; }

bool CheckEndCondition(Cell cell) {
  bool bfs_visit[N][N] = {
      false,
  };

  Cell* queue_cell[N * N] = {
      nullptr,
  };
  int front = 0;
  int rear = 0;

  bfs_visit[robot_y][robot_x] = true;
  queue_cell[rear] = &cell;
  rear++;

  while (front < rear) {
    Cell* curr_cell = queue_cell[front];
    front++;

    for (int i = 0; i < 8; i++) {
      int next_x = curr_cell->x + ddx[i];
      int next_y = curr_cell->y + ddy[i];

      if (obs_map[next_y][next_x] == -1) return false;

      if (bfs_visit[next_y][next_x]) continue;
      if (obs_map[next_y][next_x] == 0) {
        Cell* next_cell = BringHeap();
        next_cell->x = next_x;
        next_cell->y = next_y;
        next_cell->parent_cell = curr_cell;
        queue_cell[rear] = next_cell;
        bfs_visit[next_y][next_x] = true;
        rear++;
      }
    }
  }
  return true;
}

bool ExistUncoverFreeCell() {
  for (int y = left_top_y; y <= right_bottom_y; ++y) {
    for (int x = left_top_x; x <= right_bottom_x; ++x) {
      if (obs_map[y][x] == 0 && !visited_map[y][x]) {
        return true;
      }
    }
  }
  return false;
}

bool IsNear8UnknownCell(const int robot_x, const int robot_y) {
  for (int y = -1; y <= 1; ++y) {
    for (int x = -1; x <= 1; ++x) {
      if (x == 0 && y == 0) continue;
      if (obs_map[robot_y + y][robot_x + x] == -1) return true;
    }
  }
  return false;
}

void ApplyObsMap(const int robot_x, const int robot_y,
                 const int robot_heading) {
  int local_map[3][3];
  scan(local_map);
  if (robot_heading == 0) {
    for (int global_y = robot_y - 1, local_y = 0; local_y < 3;
         ++global_y, ++local_y) {
      for (int global_x = robot_x - 1, local_x = 0; local_x < 3;
           ++global_x, ++local_x) {
        obs_map[global_y][global_x] = local_map[local_y][local_x];
      }
    }
  } else if (robot_heading == 1) {
    for (int global_y = robot_y - 1, local_x = 2; local_x >= 0;
         ++global_y, --local_x) {
      for (int global_x = robot_x - 1, local_y = 0; local_y < 3;
           ++global_x, ++local_y) {
        obs_map[global_y][global_x] = local_map[local_y][local_x];
      }
    }
  } else if (robot_heading == 2) {
    for (int global_y = robot_y - 1, local_y = 2; local_y >= 0;
         ++global_y, --local_y) {
      for (int global_x = robot_x - 1, local_x = 2; local_x >= 0;
           ++global_x, --local_x) {
        obs_map[global_y][global_x] = local_map[local_y][local_x];
      }
    }
  } else if (robot_heading == 3) {
    for (int global_y = robot_y - 1, local_x = 0; local_x < 3;
         ++global_y, ++local_x) {
      for (int global_x = robot_x - 1, local_y = 2; local_y >= 0;
           ++global_x, --local_y) {
        obs_map[global_y][global_x] = local_map[local_y][local_x];
      }
    }
  }
  if (left_top_x > robot_x - 1) left_top_x = robot_x - 1;
  if (left_top_y > robot_y - 1) left_top_y = robot_y - 1;
  if (right_bottom_x < robot_x + 1) right_bottom_x = robot_x + 1;
  if (right_bottom_y < robot_y + 1) right_bottom_y = robot_y + 1;
}

bool Move() {
  if (move() == 0) return false;
  robot_x += dx[robot_heading];
  robot_y += dy[robot_heading];
  visited_map[robot_y][robot_x] = true;
  return true;
}

void Turn(const int direction) {
  turn(direction);
  robot_heading = (robot_heading + direction) % 4;
}

int FindNextCellForFreeCell(Cell cell, int* target_x_list, int* target_y_list) {
  bool bfs_visit[N][N] = {
      false,
  };

  Cell* queue_cell[N * N] = {
      nullptr,
  };
  int front = 0;
  int rear = 0;

  bfs_visit[robot_y][robot_x] = true;
  queue_cell[rear] = &cell;
  rear++;

  bool find_free_cell = false;
  Cell* free_cell;

  while (front < rear) {
    Cell* curr_cell = queue_cell[front];
    front++;

    if (obs_map[curr_cell->y][curr_cell->x] == 0 &&
        !visited_map[curr_cell->y][curr_cell->x]) {
      find_free_cell = true;
      free_cell = curr_cell;
      break;
    }

    for (int i = 0; i < 4; i++) {
      int next_heading = (curr_cell->heading + i) % 4;
      int next_x = curr_cell->x + dx[next_heading];
      int next_y = curr_cell->y + dy[next_heading];

      if (bfs_visit[next_y][next_x]) continue;
      if (obs_map[next_y][next_x] == 0) {
        Cell* next_cell = BringHeap();
        next_cell->x = next_x;
        next_cell->y = next_y;
        next_cell->heading = next_heading;
        next_cell->parent_cell = curr_cell;
        queue_cell[rear] = next_cell;
        bfs_visit[next_y][next_x] = true;
        rear++;
      }
    }
  }

  if (find_free_cell) {
    int size = 0;
    Cell* search_cell = free_cell;
    do {
      target_x_list[size] = search_cell->x;
      target_y_list[size] = search_cell->y;
      size++;
      search_cell = search_cell->parent_cell;
    } while (search_cell->parent_cell != nullptr);
    return size;
  }
  return 0;
}

int FindNextCellForUnknownCell(Cell cell, int* target_x_list,
                               int* target_y_list) {
  bool bfs_visit[N][N] = {
      false,
  };

  Cell* queue_cell[N * N];
  int front = 0;
  int rear = 0;

  bfs_visit[robot_y][robot_x] = true;
  queue_cell[rear] = &cell;
  rear++;

  bool find_unknown_cell = false;
  Cell* unknown_cell;

  while (front < rear) {
    Cell* curr_cell = queue_cell[front];
    front++;

    if (IsNear8UnknownCell(curr_cell->x, curr_cell->y)) {
      find_unknown_cell = true;
      unknown_cell = curr_cell;
      break;
    }

    for (int i = 0; i < 4; i++) {
      int next_heading = (curr_cell->heading + i) % 4;
      int next_x = curr_cell->x + dx[next_heading];
      int next_y = curr_cell->y + dy[next_heading];

      if (bfs_visit[next_y][next_x]) continue;
      if (obs_map[next_y][next_x] == 0) {
        Cell* next_cell = BringHeap();
        next_cell->x = next_x;
        next_cell->y = next_y;
        next_cell->heading = next_heading;
        next_cell->parent_cell = curr_cell;
        queue_cell[rear] = next_cell;
        bfs_visit[next_y][next_x] = true;
        rear++;
      }
    }
  }

  if (find_unknown_cell) {
    int size = 0;
    Cell* search_cell = unknown_cell;
    do {
      target_x_list[size] = search_cell->x;
      target_y_list[size] = search_cell->y;
      size++;
      search_cell = search_cell->parent_cell;
    } while (search_cell->parent_cell != nullptr);
    return size;
  }
  return 0;
}

void ResetObsMap() {
  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      obs_map[y][x] = -1;
    }
  }
}

void ResetVisitedMap() {
  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      visited_map[y][x] = false;
    }
  }
}

void ResetMap() {
  ResetObsMap();
  ResetVisitedMap();
  left_top_x = N / 2;
  left_top_y = N / 2;
  right_bottom_x = N / 2;
  right_bottom_y = N / 2;
}

bool ExistMap() { return obs_map[N / 2][N / 2] == 0; }

bool MatchScan(const int candidate_x, const int candidate_y,
               const int candidate_heading, const int local_map[3][3]) {
  if (candidate_heading == 0) {
    for (int global_y = candidate_y - 1, local_y = 0; local_y < 3;
         ++global_y, ++local_y)
      for (int global_x = candidate_x - 1, local_x = 0; local_x < 3;
           ++global_x, ++local_x)
        if (obs_map[global_y][global_x] != local_map[local_y][local_x])
          return false;
  } else if (candidate_heading == 1) {
    for (int global_y = candidate_y - 1, local_x = 2; local_x >= 0;
         ++global_y, --local_x)
      for (int global_x = candidate_x - 1, local_y = 0; local_y < 3;
           ++global_x, ++local_y)
        if (obs_map[global_y][global_x] != local_map[local_y][local_x])
          return false;
  } else if (candidate_heading == 2) {
    for (int global_y = candidate_y - 1, local_y = 2; local_y >= 0;
         ++global_y, --local_y)
      for (int global_x = candidate_x - 1, local_x = 2; local_x >= 0;
           ++global_x, --local_x)
        if (obs_map[global_y][global_x] != local_map[local_y][local_x])
          return false;
  } else if (candidate_heading == 3) {
    for (int global_y = candidate_y - 1, local_x = 0; local_x < 3;
         ++global_y, ++local_x)
      for (int global_x = candidate_x - 1, local_y = 2; local_y >= 0;
           ++global_x, --local_y)
        if (obs_map[global_y][global_x] != local_map[local_y][local_x])
          return false;
  }
  return true;
}

bool Relocalize(int* relocal_pose_x, int* relocal_pose_y,
                int* relocal_pose_heading) {
  int candidate_x[N * N * 4] = {
      -1,
  };
  int candidate_y[N * N * 4] = {
      -1,
  };
  int candidate_heading[N * N * 4] = {
      -1,
  };
  int candidate_capacity = 0;
  int candidate_count = 0;

  int local_map[3][3];
  scan(local_map);
  for (int y = left_top_y + 1; y <= right_bottom_y - 1; ++y) {
    for (int x = left_top_x + 1; x <= right_bottom_x - 1; ++x) {
      for (int heading = 0; heading < 4; ++heading) {
        if (MatchScan(x, y, heading, local_map)) {
          candidate_x[candidate_capacity] = x;
          candidate_y[candidate_capacity] = y;
          candidate_heading[candidate_capacity] = heading;
          candidate_capacity++;
          candidate_count++;
        }
      }
    }
  }

  while (true) {
    int move_state = 0;
    if (!move()) {
      turn(1);
      move_state = 1;
      if (!move()) {
        turn(2);
        move_state = 2;
        if (!move()) {
          turn(3);
          move();
          move_state = 3;
        }
      }
    }
    scan(local_map);

    for (int i = 0; i < candidate_capacity; ++i) {
      if (candidate_heading[i] == -1) continue;

      int next_x = candidate_x[i];
      int next_y = candidate_y[i];
      int next_heading = candidate_heading[i];
      if (move_state == 1) {
        next_heading = (candidate_heading[i] + 1) % 4;
      } else if (move_state == 2) {
        next_heading = (candidate_heading[i] + 3) % 4;
      } else if (move_state == 3) {
        next_heading = (candidate_heading[i] + 2) % 4;
      }
      next_x += dx[next_heading];
      next_y += dy[next_heading];

      if (MatchScan(next_x, next_y, next_heading, local_map)) {
        candidate_x[i] = next_x;
        candidate_y[i] = next_y;
        candidate_heading[i] = next_heading;
      } else {
        candidate_heading[i] = -1;
        candidate_count--;
      }
    }

    if (candidate_count == 1) {
      for (int i = 0; i < candidate_capacity; ++i) {
        if (candidate_heading[i] != -1) {
          *relocal_pose_x = candidate_x[i];
          *relocal_pose_y = candidate_y[i];
          *relocal_pose_heading = candidate_heading[i];
          return true;
        }
      }
    }
  }
  return false;
}

void init() {
  ResetMap();
  heap_count = 0;
  robot_x = N / 2;
  robot_y = N / 2;
  robot_heading = 0;
  visited_map[robot_y][robot_x] = true;
}

void cleanHouse() {
  ResetVisitedMap();
  heap_count = 0;
  robot_x = N / 2;
  robot_y = N / 2;
  robot_heading = 0;

  bool do_relocalize = false;
  if (ExistMap()) {
    if (Relocalize(&robot_x, &robot_y, &robot_heading)) {
      visited_map[robot_y][robot_x] = true;
      do_relocalize = true;
    } else {
      ResetMap();
      visited_map[robot_y][robot_x] = true;
      ApplyObsMap(robot_x, robot_y, robot_heading);
    }
  } else {
    visited_map[robot_y][robot_x] = true;
    ApplyObsMap(robot_x, robot_y, robot_heading);
  }
  while (true) {
    Cell curr_cell;
    curr_cell.x = robot_x;
    curr_cell.y = robot_y;
    curr_cell.heading = robot_heading;
    curr_cell.parent_cell = nullptr;

    int next_x_list[N * 2];
    int next_y_list[N * 2];
    int size = 0;
    if (do_relocalize) {
      if (!ExistUncoverFreeCell()) break;
      size = FindNextCellForFreeCell(curr_cell, next_x_list, next_y_list);
    } else {
      if (ExistUncoverFreeCell()) {
        size = FindNextCellForFreeCell(curr_cell, next_x_list, next_y_list);
      } else {
        if (CheckEndCondition(curr_cell)) break;
        if (IsNear8UnknownCell(robot_x, robot_y)) {
          ApplyObsMap(robot_x, robot_y, robot_heading);
          continue;
        }
        size = FindNextCellForUnknownCell(curr_cell, next_x_list, next_y_list);
      }
    }

    for (int i = size - 1; i >= 0; i--) {
      int next_x = next_x_list[i];
      int next_y = next_y_list[i];

      for (int dir = 0; dir < 4; ++dir) {
        int diff_x = next_x - robot_x;
        int diff_y = next_y - robot_y;
        if (dx[dir] == diff_x && dy[dir] == diff_y) {
          int direction = dir - robot_heading;
          if (direction < 0) direction += 4;
          if (direction != 0) Turn(direction);
          break;
        }
      }
      Move();
    }
  }
}