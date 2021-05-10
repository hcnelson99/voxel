#!/usr/bin/env python3
import os
import shutil

def build_dir(world_size):
    return 'build-w' + str(world_size)

def build_cmd(s, w, r, p = None):
    d = build_dir(s)
    exe = './' + d + '/voxel'
    flags = '-b -w ' + w + ' -r ' + r
    if p != None:
        flags += ' -p ' + str(p)
    pipe = '>> bench_output.csv'
    cmd = exe + ' ' + flags + ' ' + pipe
    return cmd

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

    # sizes = [16, 32, 64, 128, 256]
    sizes = [256]
    ps = [0, 0.001, 0.005, 0.01, 0.05, 0.1, 0.5, 1]
    # raycasts = ['naive', 'predicated', 'mipmapped']
    raycasts = ['mipmapped']
    worlds = ['random']


    for s in sizes:
        d = build_dir(s)
        if d not in os.listdir():
            cppflags = "CPPFLAGS=-DWORLD_SIZE=" + str(s)
            meson = "meson --buildtype release " + d
            os.system(cppflags + " " + meson)
        os.system("ninja -C " + d)

    for s in sizes:
        for r in raycasts:
            for w in worlds:
                for p in ps:
                    cmd = build_cmd(s, w, r, p)
                    print(cmd)
                    os.system(cmd)

main()
