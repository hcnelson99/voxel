from PIL import Image

image = Image.open(open("terrain-original.png", "rb"))

N = 256

pixels = [[0] * N for _ in range(N)]

for i in range(N):
    for j in range(N):
        pixels[i][j] = image.getpixel((i, j))

def swap(a, b):
    ai, aj = a
    bi, bj = b
    ai *= 16
    aj *= 16
    bi *= 16
    bj *= 16
    for i in range(16):
        for j in range(16):
            pixels[ai + i][aj + j], pixels[bi + i][bj + j] = pixels[bi + i][bj + j], pixels[ai + i][aj + j]

def copy(a, b, alpha_color=(0, 0, 0, 255)):
    ai, aj = a
    bi, bj = b
    ai *= 16
    aj *= 16
    bi *= 16
    bj *= 16
    for i in range(16):
        for j in range(16):
            pixel = pixels[ai + i][aj + j]
            if pixel[3] == 0:
                pixels[bi + i][bj + j] = alpha_color
            else:
                pixels[bi + i][bj + j] = pixel

for s in range(10, 14):
    for i in range(s, 0, -1):
        for j in range(16):
            swap((j, i), (j, i - 1))

for i in range(13, 16):
    for j in range(2):
        swap((i, j), (i, j + 12))

slot = 6

def rotate(ai, aj):
    ai *= 16
    aj *= 16
    buffer = [[0] * 16 for _ in range(16)]
    for i in range(16):
        for j in range(16):
            buffer[i][j] = pixels[ai + i][aj + j]

    for i in range(16):
        for j in range(16):
            pixels[ai + i][aj + j] = buffer[j][15 - i]

def pack(x, y=None, z=None):
    global slot
    doRotate = True
    if y is None:
        doRotate = False
        y = x
    if z is None:
        z = x
    copy((x % 16, (x // 16) + 4), ((slot + 0) % 16, (slot + 0) // 16))
    copy((y % 16, (y // 16) + 4), ((slot + 1) % 16, (slot + 1) // 16))
    copy((z % 16, (z // 16) + 4), ((slot + 2) % 16, (slot + 2) // 16))
    copy((z % 16, (z // 16) + 4), ((slot + 3) % 16, (slot + 3) // 16))
    copy((z % 16, (z // 16) + 4), ((slot + 4) % 16, (slot + 4) // 16))
    copy((z % 16, (z // 16) + 4), ((slot + 5) % 16, (slot + 5) // 16))

    if doRotate:
        rotate((slot + 3) % 16, (slot + 3) // 16)
        rotate((slot + 4) % 16, (slot + 4) // 16)
        rotate((slot + 4) % 16, (slot + 4) // 16)
        rotate((slot + 5) % 16, (slot + 5) // 16)
        rotate((slot + 5) % 16, (slot + 5) // 16)
        rotate((slot + 5) % 16, (slot + 5) // 16)
    slot += 6

def pack_circuit(in_coord, out_coord, side):
    global slot
    color = pixels[(side % 16)*16][(side // 16 + 4)*16]
    copy((in_coord % 16, (in_coord // 16) + 4), ((slot + 0) % 16, (slot + 0) // 16), color)
    copy((out_coord % 16, (out_coord // 16) + 4), ((slot + 1) % 16, (slot + 1) // 16), color)
    copy((side % 16, (side // 16) + 4), ((slot + 2) % 16, (slot + 2) // 16))
    copy((side % 16, (side // 16) + 4), ((slot + 3) % 16, (slot + 3) // 16))
    copy((side % 16, (side // 16) + 4), ((slot + 4) % 16, (slot + 4) // 16))
    copy((side % 16, (side // 16) + 4), ((slot + 5) % 16, (slot + 5) // 16))

    rotate((slot + 3) % 16, (slot + 3) // 16)
    rotate((slot + 4) % 16, (slot + 4) // 16)
    rotate((slot + 4) % 16, (slot + 4) // 16)
    rotate((slot + 5) % 16, (slot + 5) // 16)
    rotate((slot + 5) % 16, (slot + 5) // 16)
    rotate((slot + 5) % 16, (slot + 5) // 16)
    slot += 6

# display
for i in range(16):
    for j in range(16):
        pixels[12 * 16 + i][13 * 16 + j] = (42, 42, 42, 255)
        pixels[11 * 16 + i][13 * 16 + j] = (255, 255, 255, 255)

# switch
for i in range(16):
    for j in range(16):
        pixels[10 * 16 + i][13 * 16 + j] = (255, 0, 0, 255)
        pixels[9 * 16 + i][13 * 16 + j] = (255, 0, 0, 255)
for i in range(8):
    for j in range(8):
        pixels[10 * 16 + i + 4][13 * 16 + j + 4] = (255, 255, 255, 255)
        pixels[9 * 16 + i + 4][13 * 16 + j + 4] = (255, 255, 255, 255)
for i in range(4):
    for j in range(4):
        pixels[9 * 16 + i + 6][13 * 16 + j + 6] = (255, 0, 0, 255)

# bluestone
for i in range(16):
    for j in range(16):
        x = pixels[4 * 16 + i][13 * 16 + j]
        pixels[8 * 16 + i][13 * 16 + j] = (x[2], x[1], x[0], x[3])

        x = pixels[5 * 16 + i][13 * 16 + j]
        pixels[7 * 16 + i][13 * 16 + j] = (x[2], x[1], x[0], x[3])

# greenstone
for i in range(16):
    for j in range(16):
        x = pixels[4 * 16 + i][13 * 16 + j]
        pixels[14 * 16 + i][13 * 16 + j] = (x[2], x[0], x[1], x[3])

        x = pixels[5 * 16 + i][13 * 16 + j]
        pixels[13 * 16 + i][13 * 16 + j] = (x[2], x[0], x[1], x[3])


pack(1) # stone
pack(2) # dirt
pack(4) # wood
pack(9 * 16 + 4) # active redstone
pack(9 * 16 + 5) # inactive redstone

pack_circuit(9 * 16 + 3, 9 * 16 + 2, 9 * 16 + 0) # active delay gate
pack_circuit(9 * 16 + 3, 9 * 16 + 2, 9 * 16 + 0) # delay gate
pack_circuit(9 * 16 + 3, 9 * 16 + 2, 9 * 16 + 1) # active not gate
pack_circuit(9 * 16 + 3, 9 * 16 + 2, 9 * 16 + 1) # not gate

pack_circuit(9 * 16 + 3, 9 * 16 + 2, 9 * 16 + 6) # active diode
pack_circuit(9 * 16 + 3, 9 * 16 + 2, 9 * 16 + 6) # diode

pack(9 * 16 + 11) # active display
pack(9 * 16 + 12) # display

pack(9 * 16 + 9) # active switch
pack(9 * 16 + 10) # switch

pack(9 * 16 + 8) # active bluestone
pack(9 * 16 + 7) # inactive bluestone

pack(9 * 16 + 14) # active greenstone
pack(9 * 16 + 13) # inactive greenstone

for i in range(N):
    for j in range(N):
        image.putpixel((i, j), pixels[i][j])

image.save("terrain.png")

