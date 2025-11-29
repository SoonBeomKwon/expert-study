extern void scanFromRobot(int ceilingState[3][3]);
extern int moveRobot(int mCommand);

int map[100][100];
int weight_map[100][100];
bool visited_map[100][100];
int pose_x;
int pose_y;
int pose_theta;  // 0: up, 1: left, 2: down, 3: right

const int vec_x[4] = {0, -1, 0, 1};
const int vec_y[4] = {-1, 0, 1, 0};

void MarkLocalMap(int local_map[3][3], const int pose_x, const int pose_y, const int pose_theta) {
  map[pose_y][pose_x] = 2;
  if(pose_theta == 0) {
    for(int local_y = 0; local_y < 3; ++local_y){
      for(int local_x = 0; local_x < 3; ++local_x){
          if(local_x == 0 && local_y == 0) continue;
          int n_pose_x = pose_x + local_x;
          int n_pose_y = pose_y + local_y;
          if(map[n_pose_y][n_pose_x] == 2) continue;
          map[n_pose_y][n_pose_x] = local_map[local_y][local_x];
      }
    }
  }
  else if(pose_theta == 1) {
    for(int local_x = 0; local_x < 3; ++local_x){
      for(int local_y = 2; local_y >= 0; --local_y){
          if(local_x == 0 && local_y == 0) continue;
          int n_pose_x = pose_x + local_x;
          int n_pose_y = pose_y + local_y;
          if(map[n_pose_y][n_pose_x] == 2) continue;
          map[n_pose_y][n_pose_x] = local_map[local_y][local_x];
      }
    }
  }
  else if(pose_theta == 2) {
    for(int local_y = 2; local_y >= 0; --local_y){
      for(int local_x = 2; local_x >= 0; --local_x){
          if(local_x == 0 && local_y == 0) continue;
          int n_pose_x = pose_x + local_x;
          int n_pose_y = pose_y + local_y;
          if(map[n_pose_y][n_pose_x] == 2) continue;
          map[n_pose_y][n_pose_x] = local_map[local_y][local_x];
      }
    }
  }
  else if(pose_theta == 3) {
    for(int local_x = 2; local_x >= 0; --local_x){
      for(int local_y = 0; local_y < 3; ++local_y){
          if(local_x == 0 && local_y == 0) continue;
          int n_pose_x = pose_x + local_x;
          int n_pose_y = pose_y + local_y;
          if(map[n_pose_y][n_pose_x] == 2) continue;
          map[n_pose_y][n_pose_x] = local_map[local_y][local_x];
      }
    }
  }
}

// TODO: implement
int CalculateWeightMap(int pose_x, int pose_y) {
  visited_map[pose_y][pose_x] = true;
  for(int i = 0; i < 4; ++i){
    int next_pose_x = pose_x + vec_x[i];
    int next_pose_y = pose_y + vec_y[i];
    if(!visited_map[next_pose_y][next_pose_x])
      continue;
    if(map[next_pose_y][next_pose_x] == 1)
      continue;
  }
}

int FindDirection(const int pose_x, const int pose_y, const int pose_theta) {
  int max_weight = -100000000;
  int max_i = 0;
  for(int i = 0; i < 4; ++i) {
    int weight = weight_map[pose_y + vec_y[i]][pose_x + vec_x[i]];
    if(max_weight < weight){
      max_weight = weight;
      max_i = i;
    }
  }
  return (pose_theta + max_i) % 4;
}

int CalculateTheta(const int curr_theta, const int mCommand) {
  return (curr_theta + mCommand) % 4;
}

void Reset() {
  for(int i=0; i < 100; ++i){
    for(int j=0; j < 100; ++j){
        weight_map[i][j] = 0;
        visited_map[i][j] = false;
    }
  }
}

void init(int N, int subTaskCount)
{
  for(int i=0; i < 100; ++i){
    for(int j=0; j < 100; ++j){
        map[i][j] = -1;
        weight_map[i][j] = 0;
        visited_map[i][j] = false;
    }
  }
  pose_x = 50;
  pose_y = 50;
  pose_theta = 0;
}

void cleanHouse(int mLimitMoveCount)
{
  int local_map[3][3];
  scanFromRobot(local_map);
  MarkLocalMap(local_map, pose_x, pose_y, pose_theta);
  CalculateWeightMap(pose_x, pose_y);
  int direction = FindDirection(pose_x, pose_y, pose_theta);
  moveRobot(direction);
  pose_theta = CalculateTheta(pose_theta, direction);
  Reset();
}