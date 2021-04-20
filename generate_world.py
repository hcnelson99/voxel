import random

N = 120

def block(left):
    orientation = "Orientation::PosX" if left else "Orientation::NegX"
    r = random.random()
    if r < 0.4:
        return "Block::InactiveRedstone"
    if r < 0.5:
        return "Block(Block::DiodeGate, %s)" % orientation
    if r < 0.85:
        return "Block(Block::NotGate, %s)" % orientation
    if r < 0.95:
        return "Block(Block::DelayGate, %s)" % orientation
    return "Block::InactiveBluestone"

def create(Y):
    left = True
    for row in range(2, N, 2):
        for col in range(2, N):
            print("world->set_block(%s, %s, %s, %s);" % (col, Y, row, block(left)))
        for col in range(7, N):
            if random.random() < 0.05:
                print("world->set_block(%s, %s, %s, %s);" % (col, Y, row + 1, "Block::InactiveRedstone"))

    if random.random() < 0.4:
        for col in range(4, N - 2):
            if random.random() < 0.05:
                for row in range(2, N):
                    print("world->set_block(%s, %s, %s, %s);" % (col, Y, row, "Block::InactiveRedstone"))


    for row in range(2, N):
        print("world->set_block(%s, %s, %s, %s);" % (1, Y, row, "Block::InactiveRedstone"))

    for row in range(2, N):
        print("world->set_block(%s, %s, %s, %s);" % (N, Y, row, "Block::InactiveRedstone"))

print("#include \"world.h\"")
print("#include <iostream>")

print("World *world;")
print("int main() {")
print("world = new World;")


print("world->set_block(%s, %s, %s, %s);" % (1, 0, 0, "Block(Block::DelayGate)"))
print("world->set_block(%s, %s, %s, %s);" % (1, 0, 1, "Block(Block::ActiveDelayGate, Orientation::NegX)"))
print("world->set_block(%s, %s, %s, %s);" % (2, 0, 0, "Block::InactiveRedstone"))
print("world->set_block(%s, %s, %s, %s);" % (2, 0, 1, "Block::InactiveRedstone"))
print("world->set_block(%s, %s, %s, %s);" % (0, 0, 0, "Block::InactiveRedstone"))
print("world->set_block(%s, %s, %s, %s);" % (0, 0, 1, "Block::InactiveRedstone"))
print("world->set_block(%s, %s, %s, %s);" % (0, 1, 1, "Block::InactiveRedstone"))
print("world->set_block(%s, %s, %s, %s);" % (0, 2, 1, "Block::InactiveRedstone"))
print("world->set_block(%s, %s, %s, %s);" % (0, 2, 2, "Block::InactiveRedstone"))

for i in range(2, N, 2):
    print("world->set_block(%s, %s, %s, %s);" % (0, i, 2, "Block::InactiveRedstone"))
    print("world->set_block(%s, %s, %s, %s);" % (0, i + 1, 2, "Block(Block::DiodeGate, Orientation::PosY)"))
    create(i)

print("world->save(\"test.world\");")
print("return 0;")
print("}")

