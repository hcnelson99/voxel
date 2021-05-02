world = list(open("test.world2", "rb").read())

world2 = [0] * len(world)

N = 64

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


copy((28, 3, 22), (48, 20, 40), (28, 3, 41))

"""
copy((38, 3, 5), (53, 16, 15), (38, 3, 16))
copy((38, 3, 5), (53, 16, 15), (38, 3, 27))
copy((38, 3, 5), (53, 16, 15), (38, 3, 38))
copy((38, 3, 5), (53, 16, 15), (38, 3, 49))

clear((20, 3, 22), (35, 11, 39))

copy((19, 3, 5), (35, 11, 21), (19, 3, 24))
copy((19, 3, 5), (35, 11, 21), (19, 12, 24))
"""

open("test.world", "wb").write(bytearray(world))

