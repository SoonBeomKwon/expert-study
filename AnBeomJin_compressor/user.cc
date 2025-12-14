#include <bitset>
#include <iostream>

class BitStream {
 public:
  explicit BitStream(char* data) : data_(data), pos_(0) {}
  bool Write(const int num, const int bit_count) {
    if (pos_ + bit_count >= kMaxBitCount) {
      std::cout << "bit overflow, pos_: " << pos_
                << ", bit_count: " << bit_count
                << ", kMaxBitCount: " << kMaxBitCount << "\n";
      return false;
    }
    for (int i = 0; i < bit_count; ++i) {
      if (num & (1 << i)) data_[(pos_ + i) / 8] |= (1 << (pos_ + i) % 8);
    }
    pos_ += bit_count;
    return true;
  }

  bool Read(int* num, const int bit_count) {
    if (pos_ + bit_count >= kMaxBitCount) {
      std::cout << "bit overflow, pos_: " << pos_
                << ", bit_count: " << bit_count
                << ", kMaxBitCount: " << kMaxBitCount << "\n";
      return false;
    }
    *num = 0;
    for (int i = 0; i < bit_count; ++i) {
      if (data_[(pos_ + i) / 8] & (1 << (pos_ + i) % 8)) (*num) |= (1 << i);
    }
    pos_ += bit_count;
    return true;
  }

  int GetPos() { return pos_; }

 private:
  const int kMaxBitCount{150000 * 8};
  char* data_;
  int pos_;
};

bool MatchMask(const int map_x, const int map_y, const char map[10000][10000],
               const char* mask, const int mask_size) {
  for (int y = 0; y < mask_size; ++y) {
    for (int x = 0; x < mask_size; ++x) {
      if (map[map_y + y][map_x + x] != mask[y * mask_size + x]) return false;
    }
  }
  return true;
}

void comp(char map[10000][10000], char data[150000]) {
  for (int i = 0; i < 150000; ++i) data[i] = 0;

  const int kMapSize{10000};
  BitStream bit_stream(data);
  for (int y = 0; y < kMapSize; ++y) {
    for (int x = 0; x < kMapSize; ++x) {
      if (map[y][x] == 1 && (x + 1 < kMapSize && map[y][x + 1] == 1)) {
        map[y][x] = 0;
        std::cout << "line start: (" << x << ", " << y << ")\n";
        std::cout << "bit pos 1: " << bit_stream.GetPos() << "\n";
        bit_stream.Write(x, 14);
        std::cout << "bit pos 2: " << bit_stream.GetPos() << "\n";
        bit_stream.Write(y, 14);
        std::cout << "bit pos 3: " << bit_stream.GetPos() << "\n";

        int line_x = x + 1;
        for (; line_x < kMapSize; ++line_x) {
          if (map[y][line_x] == 0) break;
          map[y][line_x] = 0;
        }

        std::cout << "line length: " << line_x - x << "\n";
        bit_stream.Write(line_x - x, 14);
        std::cout << "bit pos 4: " << bit_stream.GetPos() << "\n";
        bit_stream.Write(0, 1);
        std::cout << "bit pos 5: " << bit_stream.GetPos() << "\n";
      }
    }
  }

  std::cout << "pos_: " << bit_stream.GetPos() << "\n";
}

void decomp(char result[10000][10000], char data[150000]) {
  BitStream bit_stream(data);
}
