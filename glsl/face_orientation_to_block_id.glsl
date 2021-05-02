/*
 * This is mostly generated using the C++ code below.
 */

const uint[] face_orientation_to_block_offset = uint[36](
    1, 0, 2, 2, 2, 2, 0, 1, 4, 4, 4, 4, 3, 3, 1, 0, 3, 3, 5, 5, 0, 1, 5, 5, 2, 2, 3, 3, 1, 0, 4, 4, 5, 5, 0, 1
);

uint face_orientation_to_block_id(uint block, uint face) {
    uint orientation = block & 7;
    uint offset = face_orientation_to_block_offset[orientation * 6 + face];
    return (block >> 3) * 6 + offset;
}

/*
#include <iostream>
using namespace std;
int main() {
  for (int orientation = 0; orientation < 6; orientation++) {
    for (int face = 0; face < 6; face++) {
      int block_id = 0;
      if (orientation == face) {
        block_id = 1;
      } else if (orientation / 2 == face / 2) {
        block_id = 0;
      } else {
        uint offset = 0;

        if (orientation == 1) {
          offset = 2;
        } else if (orientation == 0) {
          offset = 0;
        } else if (orientation == 3) {
          offset = 3;
        } else if (orientation == 2) {
          offset = 1;
        } else if (orientation == 5 && face / 2 == 0) {
          offset = 2;
        } else if (orientation == 5 && face == 3) {
          offset = 3;
        } else if (orientation == 5 && face == 2) {
          offset = 3;
        } else if (orientation == 4 && face / 2 == 0) {
          offset = 0;
        } else if (orientation == 4 && face == 3) {
          offset = 1;
        } else if (orientation == 4 && face == 2) {
          offset = 1;
        }
        block_id = 2 + offset;
      }

      int id = orientation * 6 + face;
      printf("%d, ", block_id);
    }
  }
}
*/
