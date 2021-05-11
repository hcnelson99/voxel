world = list(open("bk", "rb").read())

world2 = [0] * len(world)

N = 256

def get(x, y, z):
    if (0 <= x < N and 0 <= y < N and 0 <= z < N):
        return world[x * N * N + y * N + z]
    return 0

def set(x, y, z, v):
    if (0 <= x < N and 0 <= y < N and 0 <= z < N):
        world[x * N * N + y * N + z] = v

def set2(x, y, z, v):
    if (0 <= x < N and 0 <= y < N and 0 <= z < N):
        world2[x * N * N + y * N + z] = v

def copy(minimum, maximum, corner):
    for x in range(minimum[0], maximum[0]):
        for y in range(minimum[1], maximum[1]):
            for z in range(minimum[2], maximum[2]):
                set(corner[0] + x - minimum[0], corner[1] + y - minimum[1], corner[2] + z - minimum[2], get(x, y, z))

def clear(minimum, maximum):
    for x in range(minimum[0], maximum[0]):
        for y in range(minimum[1], maximum[1]):
            for z in range(minimum[2], maximum[2]):
                set(x, y, z, 0)

copy((3, 8, 2), (115, 120, 86), (3 + 112, 8, 2))
copy((3, 8, 2), (115, 120, 86), (3, 8 + 112, 2))
copy((3, 8, 2), (115, 120, 86), (3 + 112, 8 + 112, 2))

open("game_of_life_256_acorn", "wb").write(bytearray(world))

