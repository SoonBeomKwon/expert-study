#include <cstdio>

// 1. 설정 및 상수 정의
#define MAP_SIZE 10     // 맵의 크기 (10x10)
#define MAX_NODES 5000  // 힙(Heap) 최대 크기
#define INF 99999999

// 방향: 0:상, 1:우, 2:하, 3:좌
const int dy[4] = {-1, 0, 1, 0};
const int dx[4] = {0, 1, 0, -1};

// 2. 구조체 정의
struct Node {
  int y, x;
  int dir;           // 현재 바라보는 방향
  int g;             // 시작점부터 현재까지의 비용
  int h;             // 목적지까지의 추정 비용 (Heuristic)
  int f;             // f = g + h
  int pY, pX, pDir;  // 경로 역추적을 위한 부모 좌표 및 방향
};

// 맵 데이터 (0: 빈곳, 1: 벽)
int map[MAP_SIZE][MAP_SIZE] = {
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, {0, 1, 1, 1, 1, 1, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 1, 0, 0}, {0, 1, 1, 1, 0, 1, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 1, 0, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 1, 0, 0, 0, 0}};

// 모든 상태를 저장할 배열 (y, x, 방향)
Node nodePool[MAP_SIZE][MAP_SIZE][4];
bool closedList[MAP_SIZE][MAP_SIZE][4];  // 방문 여부 체크

// 3. Min-Heap (Priority Queue) 직접 구현
struct MinHeap {
  Node* heap[MAX_NODES];
  int size;

  void init() { size = 0; }

  void push(Node* node) {
    if (size >= MAX_NODES) return;  // 예외처리: 힙 가득 참
    heap[size] = node;
    int cur = size;
    size++;

    // Bubble Up
    while (cur > 0) {
      int parent = (cur - 1) / 2;
      if (heap[cur]->f < heap[parent]->f) {
        Node* temp = heap[cur];
        heap[cur] = heap[parent];
        heap[parent] = temp;
        cur = parent;
      } else {
        break;
      }
    }
  }

  Node* pop() {
    if (size == 0) return nullptr;
    Node* ret = heap[0];
    size--;
    heap[0] = heap[size];

    // Bubble Down
    int cur = 0;
    while (cur * 2 + 1 < size) {
      int left = cur * 2 + 1;
      int right = cur * 2 + 2;
      int smallest = cur;

      if (left < size && heap[left]->f < heap[smallest]->f) {
        smallest = left;
      }
      if (right < size && heap[right]->f < heap[smallest]->f) {
        smallest = right;
      }

      if (smallest != cur) {
        Node* temp = heap[cur];
        heap[cur] = heap[smallest];
        heap[smallest] = temp;
        cur = smallest;
      } else {
        break;
      }
    }
    return ret;
  }

  bool empty() { return size == 0; }
} openList;

// 4. 유틸리티 함수
int myAbs(int a) { return (a < 0) ? -a : a; }

// 휴리스틱 함수 (맨해튼 거리 * 기본 비용 10)
int calculateH(int y, int x, int targetY, int targetX) {
  return (myAbs(targetY - y) + myAbs(targetX - x)) * 10;
}

// 5. 메인 알고리즘
void aStar(int startY, int startX, int startDir, int targetY, int targetX) {
  // 초기화
  openList.init();
  for (int y = 0; y < MAP_SIZE; ++y) {
    for (int x = 0; x < MAP_SIZE; ++x) {
      for (int d = 0; d < 4; ++d) {
        closedList[y][x][d] = false;
        nodePool[y][x][d].y = y;
        nodePool[y][x][d].x = x;
        nodePool[y][x][d].dir = d;
        nodePool[y][x][d].g = INF;
        nodePool[y][x][d].f = INF;
        nodePool[y][x][d].pY = -1;  // 부모 없음
      }
    }
  }

  // 시작 노드 설정
  Node* startNode = &nodePool[startY][startX][startDir];
  startNode->g = 0;
  startNode->h = calculateH(startY, startX, targetY, targetX);
  startNode->f = startNode->g + startNode->h;
  openList.push(startNode);

  Node* targetNode = nullptr;

  while (!openList.empty()) {
    Node* current = openList.pop();

    // 목적지 도착 확인
    if (current->y == targetY && current->x == targetX) {
      targetNode = current;
      break;
    }

    // 이미 방문한 상태(더 적은 비용으로 처리됨)라면 스킵
    if (closedList[current->y][current->x][current->dir]) continue;
    closedList[current->y][current->x][current->dir] = true;

    // 4방향 탐색
    for (int i = 0; i < 4; i++) {
      int ny = current->y + dy[i];
      int nx = current->x + dx[i];

      // 맵 범위 체크
      if (ny < 0 || ny >= MAP_SIZE || nx < 0 || nx >= MAP_SIZE) continue;
      // 장애물 체크
      if (map[ny][nx] == 1) continue;

      // 비용 계산 로직
      int moveCost = 0;
      if (current->dir == i) {
        moveCost = 10;  // 전진 (방향 유지)
      } else {
        moveCost = 15;  // 좌/우/후진 (방향 변경)
      }

      int newG = current->g + moveCost;

      // 더 나은 경로를 발견했는지 확인
      if (newG < nodePool[ny][nx][i].g) {
        Node* neighbor = &nodePool[ny][nx][i];
        neighbor->g = newG;
        neighbor->h = calculateH(ny, nx, targetY, targetX);
        neighbor->f = neighbor->g + neighbor->h;
        neighbor->pY = current->y;
        neighbor->pX = current->x;
        neighbor->pDir = current->dir;  // 부모의 방향 저장

        openList.push(neighbor);
      }
    }
  }

  // 6. 결과 출력 (역추적)
  if (targetNode == nullptr) {
    printf("경로를 찾을 수 없습니다.\n");
  } else {
    printf("최종 비용: %d\n", targetNode->g);

    // 경로 역추적을 위한 임시 배열
    int pathY[MAP_SIZE * MAP_SIZE];
    int pathX[MAP_SIZE * MAP_SIZE];
    int pathIdx = 0;

    Node* curr = targetNode;
    while (curr->pY != -1) {
      pathY[pathIdx] = curr->y;
      pathX[pathIdx] = curr->x;
      pathIdx++;
      curr = &nodePool[curr->pY][curr->pX][curr->pDir];
    }
    // 시작점 추가
    pathY[pathIdx] = startY;
    pathX[pathIdx] = startX;

    printf("경로 (Target -> Start 역순):\n");
    for (int i = 0; i <= pathIdx; i++) {
      printf("(%d, %d) ", pathY[i], pathX[i]);
      if (i > 0 && i % 5 == 0) printf("\n");
    }
    printf("\n");

    // 시각화 (간단 텍스트)
    printf("\n[맵 시각화 (S:시작, E:끝, *:경로)]\n");
    char visualMap[MAP_SIZE][MAP_SIZE];
    for (int y = 0; y < MAP_SIZE; y++) {
      for (int x = 0; x < MAP_SIZE; x++) {
        visualMap[y][x] = (map[y][x] == 1) ? '#' : '.';
      }
    }
    for (int i = 0; i <= pathIdx; i++) visualMap[pathY[i]][pathX[i]] = '*';
    visualMap[startY][startX] = 'S';
    visualMap[targetY][targetX] = 'E';

    for (int y = 0; y < MAP_SIZE; y++) {
      for (int x = 0; x < MAP_SIZE; x++) {
        printf("%c ", visualMap[y][x]);
      }
      printf("\n");
    }
  }
}

int main() {
  // 시작점(0,0), 초기방향(0:상), 도착점(9,9)
  // 초기 방향이 중요합니다. (0,0)에서 0(상)을 보고 있다면,
  // (1,0)으로 가는 것은 '후진'이 되어 비용 15가 됩니다.
  // (0,1)로 가는 것은 '우회전'이 되어 비용 15가 됩니다.
  // 여기서는 테스트를 위해 Start Direction을 1(우측)로 설정해 봅니다.
  printf("A* 알고리즘 시작 (STL 미사용)\n");
  aStar(0, 0, 1, 9, 9);

  return 0;
}
