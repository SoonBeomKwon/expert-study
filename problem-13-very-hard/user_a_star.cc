// 5070065
#include <iomanip>
#include <iostream>
extern void scan(int floorState[3][3]);
extern int move(void);
extern void turn(int mCommand);

static const int kInf = 210000000;
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

static const int kForwardCost{10};
static const int kTurnCost{15};

static int robot_x = N / 2;
static int robot_y = N / 2;
static int robot_heading = 0;

struct Cell {
  int x = 0;
  int y = 0;
  int heading = 0;
};

struct Node {
  Cell cell;
  // int g{0};
  // int h{0};
  int f{0};
  Cell previous_cell;
};

Node cost_map[N][N];
bool closed_list[N][N] = {false};

class MinHeap {
 public:
  void Init() { size = 0; }

  bool Push(Node* node) {
    if (size >= kMaxSize) return false;
    heap[size] = node;
    int cur = size;
    size++;

    while (cur > 0) {
      int parent = (cur - 1) / 2;
      if (heap[cur]->f < heap[parent]->f) {
        Node* temp = heap[cur];
        heap[cur] = heap[parent];
        heap[parent] = temp;
        cur = parent;
      } else
        break;
    }
    return true;
  }

  Node* Pop() {
    if (size == 0) return nullptr;
    Node* ret = heap[0];
    size--;
    heap[0] = heap[size];

    int cur = 0;
    while (cur * 2 + 1 < size) {
      int left = cur * 2 + 1;
      int right = cur * 2 + 2;
      int smallest = cur;

      if (left < size && heap[left]->f < heap[smallest]->f) smallest = left;
      if (right < size && heap[right]->f < heap[smallest]->f) smallest = right;
      if (smallest != cur) {
        Node* temp = heap[cur];
        heap[cur] = heap[smallest];
        heap[smallest] = temp;
        cur = smallest;
      } else
        break;
    }
    return ret;
  }

  bool Empty() { return size == 0; }

 private:
  static const int kMaxSize{5000};
  Node* heap[kMaxSize];
  int size{0};
} open_list;

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
  open_list.Init();

  for (int y = left_top_y; y <= right_bottom_y; ++y) {
    for (int x = left_top_x; x <= right_bottom_x; ++x) {
      closed_list[y][x] = false;
      Node* node = &cost_map[y][x];
      // node->g = kInf;
      node->f = kInf;
    }
  }

  Node* start_node = &cost_map[cell.y][cell.x];
  start_node->cell.heading = cell.heading;
  start_node->previous_cell.y = -1;
  start_node->f = 0;
  open_list.Push(start_node);

  Node* target_node = nullptr;

  while (!open_list.Empty()) {
    Node* current_node = open_list.Pop();

    // std::cout << "current node: " << current_node->cell.x << ", "
    //           << current_node->cell.y << ", " << current_node->cell.heading
    //           << ")\n";
    const int obs_value = obs_map[current_node->cell.y][current_node->cell.x];
    const bool is_visited =
        visited_map[current_node->cell.y][current_node->cell.x];
    // std::cout << "a\n";
    if (obs_value == 0 && !is_visited) {
      // std::cout << "b\n";
      target_node = current_node;
      break;
    }
    // std::cout << "c\n";

    bool* is_closed = &closed_list[current_node->cell.y][current_node->cell.x];
    // std::cout << "d\n";
    if ((*is_closed)) continue;
    // std::cout << "e\n";
    (*is_closed) = true;
    // std::cout << "f\n";

    for (int i = 0; i < 4; ++i) {
      // std::cout << "g\n";
      int next_x =
          current_node->cell.x + dx[(current_node->cell.heading + i) % 4];
      int next_y =
          current_node->cell.y + dy[(current_node->cell.heading + i) % 4];
      // std::cout << "next pose: (" << next_x << ", " << next_y << ", " << i
      //           << ")\n";

      // std::cout << "h\n";
      if (obs_map[next_y][next_x] == 1) continue;
      // std::cout << "i\n";

      const int move_cost = (i == 0) ? kForwardCost : kTurnCost;
      int new_cost = current_node->f + move_cost;

      if (new_cost < cost_map[next_y][next_x].f) {
        // std::cout << "j\n";
        Node* neighbor_node = &cost_map[next_y][next_x];
        neighbor_node->cell.heading = (current_node->cell.heading + i) % 4;
        neighbor_node->f = new_cost;
        neighbor_node->previous_cell.x = current_node->cell.x;
        neighbor_node->previous_cell.y = current_node->cell.y;
        neighbor_node->previous_cell.heading = current_node->cell.heading;
        open_list.Push(neighbor_node);
      }
    }
  }

  if (target_node) {
    int size = 0;
    Node* search_node = target_node;
    while (search_node->previous_cell.y != -1) {
      // std::cout << "search_node->cell: (" << search_node->cell.x << ", "
      //           << search_node->cell.y << ", " << search_node->cell.heading
      //           << ")\n";
      target_x_list[size] = search_node->cell.x;
      target_y_list[size] = search_node->cell.y;
      size++;
      search_node =
          &cost_map[search_node->previous_cell.y][search_node->previous_cell.x];
    }
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
                int* relocal_pose_heading, int move_state_list[N],
                int* move_state_size) {
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

  int n = 0;
  const int filter = 2;
  while (true) {
    int move_state = 0;
    if (!move()) {
      turn(1);
      move_state = 1;
      if (!move()) {
        turn(2);
        move_state = 3;
        if (!move()) {
          turn(3);
          move();
          move_state = 2;
        }
      }
    }

    if (n % filter == 0) scan(local_map);

    move_state_list[*move_state_size] = move_state;
    (*move_state_size)++;

    for (int i = 0; i < candidate_capacity; ++i) {
      if (candidate_heading[i] == -1) continue;

      int next_x = candidate_x[i];
      int next_y = candidate_y[i];
      int next_heading = (candidate_heading[i] + move_state) % 4;
      next_x += dx[next_heading];
      next_y += dy[next_heading];

      candidate_x[i] = next_x;
      candidate_y[i] = next_y;
      candidate_heading[i] = next_heading;

      if (n % filter == 0) {
        if (!MatchScan(next_x, next_y, next_heading, local_map)) {
          candidate_heading[i] = -1;
          candidate_count--;
        }
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
    n++;
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

  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      closed_list[y][x] = false;
      Node* node = &cost_map[y][x];
      node->cell.x = x;
      node->cell.y = y;
      // node->g = kInf;
      node->f = kInf;
      node->previous_cell.y = -1;
    }
  }
}

void cleanHouse() {
  ResetVisitedMap();
  heap_count = 0;
  robot_x = N / 2;
  robot_y = N / 2;
  robot_heading = 0;

  bool do_relocalize = false;
  if (ExistMap()) {
    int move_state_list[N * N];
    int move_state_size = 0;
    Relocalize(&robot_x, &robot_y, &robot_heading, move_state_list,
               &move_state_size);
    int check_x = robot_x;
    int check_y = robot_y;
    int check_heading = robot_heading;
    for (int i = move_state_size - 1; i >= 0; --i) {
      check_x -= dx[check_heading];
      check_y -= dy[check_heading];
      check_heading = check_heading - move_state_list[i];
      if (check_heading < 0) check_heading += 4;
      visited_map[check_y][check_x] = true;
    }
    do_relocalize = true;
  }
  visited_map[robot_y][robot_x] = true;

  while (true) {
    Cell curr_cell;
    curr_cell.x = robot_x;
    curr_cell.y = robot_y;
    curr_cell.heading = robot_heading;

    int next_x_list[N * 2];
    int next_y_list[N * 2];
    int size = 0;
    if (!do_relocalize) {
      if (IsNear8UnknownCell(robot_x, robot_y))
        ApplyObsMap(robot_x, robot_y, robot_heading);
    }
    // std::cout << "obs map\n";
    // for (int y = left_top_y; y <= right_bottom_y; ++y) {
    //   for (int x = left_top_x; x <= right_bottom_x; ++x) {
    //     std::cout << std::setw(3) << obs_map[y][x];
    //   }
    //   std::cout << "\n";
    // }
    // std::cout << "visited map\n";
    // for (int y = left_top_y; y <= right_bottom_y; ++y) {
    //   for (int x = left_top_x; x <= right_bottom_x; ++x) {
    //     std::cout << std::setw(3) << visited_map[y][x];
    //   }
    //   std::cout << "\n";
    // }
    // std::cout << "left top: (" << left_top_x << ", " << left_top_y << ")\n";
    // std::cout << "right bottom: (" << right_bottom_x << ", " <<
    // right_bottom_y
    //           << ")\n";
    // std::cout << "robot pose: (" << robot_x << ", " << robot_y << ", "
    //           << robot_heading << ")\n";
    size = FindNextCellForFreeCell(curr_cell, next_x_list, next_y_list);
    // std::cout << "cost map\n";
    // for (int y = left_top_y; y <= right_bottom_y; ++y) {
    //   for (int x = left_top_x; x <= right_bottom_x; ++x) {
    //     std::cout << "(" << std::setw(3)
    //               << (cost_map[y][x].f == kInf ? -1 : cost_map[y][x].f) << ",
    //               "
    //               << std::setw(3) << cost_map[y][x].cell.x << ", "
    //               << std::setw(3) << cost_map[y][x].cell.y << ", "
    //               << std::setw(3) << cost_map[y][x].cell.heading << ", "
    //               << std::setw(3) << cost_map[y][x].previous_cell.x << ", "
    //               << std::setw(3) << cost_map[y][x].previous_cell.y << ") ";
    //   }
    //   std::cout << "\n";
    // }
    if (size == 0) break;

    for (int i = size - 1; i >= 0; i--) {
      int next_x = next_x_list[i];
      int next_y = next_y_list[i];

      // std::cout << "next pose: (" << next_x << ", " << next_y << ")\n";

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
