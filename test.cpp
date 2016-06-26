#include <iostream>
using namespace std;

int main() {
  unsigned int fan_speed = 0xd9;
  
  float const step = float(0xff) / fan_speed;
  float tour = step;
  bool intr = 0;  // interleave positive/negative half-waves

  // evenly-distributed triggers
  for (unsigned int i = 0; i <= 0xff; i++) {
    cout << "line "  << i << "\t";
    if (i + 1 >= tour) {
      if (intr == (i % 2 == 0)) {
	cout << 0 << endl;
	continue;
      }

      cout << 1 << endl;
      tour += step;
      intr = (i % 2 == 0);
    }
    else cout << 0 << endl;
  }
}
