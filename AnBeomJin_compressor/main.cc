#include <stdio.h>
#include <time.h>

static char map[10000][10000];
static char map_org[10000][10000];
static char result[10000][10000];
static char data[150000];

static unsigned short random(void) {
  static unsigned long long seed = 5;
  return ((unsigned short)((seed = seed * 25214903917ULL + 11ULL) >> 16));
}

void build() {
  int sx, sy, ex, ey, cnt = 0;

  for (register int i = 0; i < 10000; ++i) {
    for (register int j = 0; j < 10000; ++j) {
      map[i][j] = map_org[i][j] = result[i][j] = 0;
    }
  }

  while (cnt < 10000) {
    sx = random() % 10000;
    sy = random() % 10000;
    ex = random() % 10000;
    ey = random() % 10000;

    if (sx >= ex || sy >= ey) continue;

    // printf("main left_top: (%d, %d)\n", sx, sy);
    // printf("main right_bottom: (%d, %d)\n", ex, ey);

    for (int x = sx; x <= ex; ++x) {
      map[sy][x] = map_org[sy][x] = 1;
      map[ey][x] = map_org[ey][x] = 1;
    }

    for (int y = sy + 1; y <= ey - 1; ++y) {
      map[y][sx] = map_org[y][sx] = 1;
      map[y][ex] = map_org[y][ex] = 1;
    }

    cnt++;
  }
}

static bool verify() {
  for (register int y = 0; y < 10000; ++y) {
    for (register int x = 0; x < 10000; ++x) {
      if (result[y][x] != map_org[y][x]) return false;
    }
  }
  return true;
}

extern void comp(char map[10000][10000], char data[150000]);
extern void decomp(char result[10000][10000], char data[150000]);

void save_map_to_pgm() {
  FILE* fp = fopen("map.pgm", "wb");
  if (!fp) {
    printf("Failed to open map.pgm for writing\n");
    return;
  }
  fprintf(fp, "P5\n10000 10000\n255\n");
  for (int i = 0; i < 10000; ++i) {
    for (int j = 0; j < 10000; ++j) {
      fputc(map[i][j] ? 255 : 0, fp);
    }
  }
  fclose(fp);
  printf("Map saved to map.pgm\n");
}

int main() {
  build();
  save_map_to_pgm();

  time_t SCORE = clock();
  comp(map, data);
  decomp(result, data);

  SCORE = (clock() - SCORE) / (CLOCKS_PER_SEC / 1000);
  if (!verify()) SCORE = 100000000;

  if (SCORE < 7000)
    printf("PASS\n");
  else
    printf("FAIL\n");

  printf("SCORE: %d\n", SCORE);
}
