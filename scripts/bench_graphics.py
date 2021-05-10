#!/usr/bin/env python3
import os
import shutil

def build_dir(world_size):
    return 'build-w' + str(world_size)


def main():
    if 'meson.build' not in os.listdir():
        print("script should be run from 'voxel'")
        exit(1)

    if 'bench_output.csv' in os.listdir():
        print("clean out old bench_output.csv")
        exit(1)

    # for d in os.listdir():
    #     if d.startswith('build-w'):
    #         shutil.rmtree(d)

    sizes = [16, 32, 64, 128, 256]

    for s in sizes:
        d = build_dir(s)
        if d not in os.listdir():
            cppflags = "CPPFLAGS=-DWORLD_SIZE=" + str(s)
            meson = "meson --buildtype release " + d
            os.system(cppflags + " " + meson)
        os.system("ninja -C " + d)

    raycasts = ['naive', 'predicated', 'mipmapped']
    worlds = ['outline']

    for s in sizes:
        for r in raycasts:
            for w in worlds:
                d = build_dir(s)
                exe = './' + d + '/voxel'
                flags = '-b -w ' + w + ' -r ' + r
                pipe = '>> bench_output.csv'
                cmd = exe + ' ' + flags + ' ' + pipe
                print(cmd)
                os.system(cmd)

main()
