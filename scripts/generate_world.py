import random

WORLD_SIZE = 256
N = WORLD_SIZE - 4

def print_boilerplate(str):
    #print(str)
    pass

OrientationWidth = 3

BLOCK = {
    "Block::Air": 0 << OrientationWidth,
    "Block::Stone": 1 << OrientationWidth,
    "Block::Dirt": 2 << OrientationWidth,
    "Block::Glowstone": 3 << OrientationWidth,
    "Block::ActiveRedstone": 4 << OrientationWidth,
    "Block::InactiveRedstone": 5 << OrientationWidth,
    "Block(Block::ActiveDelayGate, Orientation::PosX)": 6 << OrientationWidth | 1,
    "Block(Block::ActiveDelayGate, Orientation::NegX)": 6 << OrientationWidth | 0,
    "Block(Block::ActiveDelayGate, Orientation::PosY)": 6 << OrientationWidth | 3,
    "Block(Block::ActiveDelayGate, Orientation::NegY)": 6 << OrientationWidth | 2,
    "Block(Block::ActiveDelayGate, Orientation::PosZ)": 6 << OrientationWidth | 5,
    "Block(Block::ActiveDelayGate, Orientation::NegZ)": 6 << OrientationWidth | 4,
    "Block(Block::DelayGate, Orientation::PosX)": 7 << OrientationWidth | 1,
    "Block(Block::DelayGate, Orientation::NegX)": 7 << OrientationWidth | 0,
    "Block(Block::DelayGate, Orientation::PosY)": 7 << OrientationWidth | 3,
    "Block(Block::DelayGate, Orientation::NegY)": 7 << OrientationWidth | 2,
    "Block(Block::DelayGate, Orientation::PosZ)": 7 << OrientationWidth | 5,
    "Block(Block::DelayGate, Orientation::NegZ)": 7 << OrientationWidth | 4,
    "Block(Block::ActiveNotGate, Orientation::PosX)": 8 << OrientationWidth | 1,
    "Block(Block::ActiveNotGate, Orientation::NegX)": 8 << OrientationWidth | 0,
    "Block(Block::ActiveNotGate, Orientation::PosY)": 8 << OrientationWidth | 3,
    "Block(Block::ActiveNotGate, Orientation::NegY)": 8 << OrientationWidth | 2,
    "Block(Block::ActiveNotGate, Orientation::PosZ)": 8 << OrientationWidth | 5,
    "Block(Block::ActiveNotGate, Orientation::NegZ)": 8 << OrientationWidth | 4,
    "Block(Block::NotGate, Orientation::PosX)": 9 << OrientationWidth | 1,
    "Block(Block::NotGate, Orientation::NegX)": 9 << OrientationWidth | 0,
    "Block(Block::NotGate, Orientation::PosY)": 9 << OrientationWidth | 3,
    "Block(Block::NotGate, Orientation::NegY)": 9 << OrientationWidth | 2,
    "Block(Block::NotGate, Orientation::PosZ)": 9 << OrientationWidth | 5,
    "Block(Block::NotGate, Orientation::NegZ)": 9 << OrientationWidth | 4,
    "Block(Block::ActiveDiodeGate, Orientation::PosX)": 10 << OrientationWidth | 1,
    "Block(Block::ActiveDiodeGate, Orientation::NegX)": 10 << OrientationWidth | 0,
    "Block(Block::ActiveDiodeGate, Orientation::PosY)": 10 << OrientationWidth | 3,
    "Block(Block::ActiveDiodeGate, Orientation::NegY)": 10 << OrientationWidth | 2,
    "Block(Block::ActiveDiodeGate, Orientation::PosZ)": 10 << OrientationWidth | 5,
    "Block(Block::ActiveDiodeGate, Orientation::NegZ)": 10 << OrientationWidth | 4,
    "Block(Block::DiodeGate, Orientation::PosX)": 11 << OrientationWidth | 1,
    "Block(Block::DiodeGate, Orientation::NegX)": 11 << OrientationWidth | 0,
    "Block(Block::DiodeGate, Orientation::PosY)": 11 << OrientationWidth | 3,
    "Block(Block::DiodeGate, Orientation::NegY)": 11 << OrientationWidth | 2,
    "Block(Block::DiodeGate, Orientation::PosZ)": 11 << OrientationWidth | 5,
    "Block(Block::DiodeGate, Orientation::NegZ)": 11 << OrientationWidth | 4,
    "Block::ActiveDisplay": 12 << OrientationWidth,
    "Block::Display": 13 << OrientationWidth,
    "Block::ActiveSwitch": 14 << OrientationWidth,
    "Block::Switch": 15 << OrientationWidth,
    "Block::ActiveBluestone": 16 << OrientationWidth,
    "Block::InactiveBluestone": 17 << OrientationWidth,
    "Block::ActiveGreenstone": 18 << OrientationWidth,
    "Block::InactiveGreenstone": 19 << OrientationWidth,
}

WORLD = [0] * (WORLD_SIZE * WORLD_SIZE * WORLD_SIZE)

def set_block(x, y, z, block):
    #print("world->set_block(%s, %s, %s, %s);" % (x, y, z, block))
    WORLD[x * WORLD_SIZE * WORLD_SIZE + y * WORLD_SIZE + z] = BLOCK[block]

def block(left):
    orientation = "Orientation::PosX" if left else "Orientation::NegX"
    r = random.random()
    if r < 0.4:
        return "Block::InactiveRedstone"
    if r < 0.5:
        return "Block(Block::DiodeGate, %s)" % orientation
    if r < 0.85:
        return "Block(Block::NotGate, %s)" % orientation
    return "Block(Block::DelayGate, %s)" % orientation

def create(Y):
    left = True
    for row in range(2, N, 2):
        for col in range(2, N):
            set_block(col, Y, row, block(left))

    if random.random() < 0.4:
        for col in range(4, N - 2):
            if random.random() < 0.05:
                for row in range(2, N):
                    set_block(col, Y, row, "Block::InactiveRedstone")


    for row in range(2, N):
        set_block(1, Y, row, "Block::InactiveRedstone")

    for row in range(2, N):
        set_block(N, Y, row, "Block::InactiveRedstone")

print_boilerplate("#include \"world.h\"")
print_boilerplate("#include <iostream>")

print_boilerplate("World *world;")
print_boilerplate("int main() {")
print_boilerplate("world = new World;")


set_block(1, 0, 0, "Block(Block::DelayGate, Orientation::PosX)")
set_block(1, 0, 1, "Block(Block::ActiveDelayGate, Orientation::NegX)")
set_block(2, 0, 0, "Block::InactiveBluestone")
set_block(2, 0, 1, "Block::InactiveBluestone")
set_block(0, 0, 0, "Block::InactiveBluestone")
set_block(0, 0, 1, "Block::InactiveBluestone")
set_block(0, 1, 1, "Block::InactiveBluestone")
set_block(0, 2, 1, "Block::InactiveBluestone")
set_block(0, 2, 2, "Block::InactiveBluestone")

for i in range(2, N, 2):
    set_block(0, i, 2, "Block::InactiveBluestone")
    set_block(0, i, 3, "Block(Block::DiodeGate, Orientation::PosZ)")
    set_block(0, i, 4, "Block::InactiveRedstone")
    set_block(0, i + 1, 2, "Block(Block::DiodeGate, Orientation::PosY)")
    create(i)

print_boilerplate("world->save(\"test.world\");")
print_boilerplate("return 0;")
print_boilerplate("}")


with open("world", "wb") as output:
    output.write(bytearray(WORLD))
