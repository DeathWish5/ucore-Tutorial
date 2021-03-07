import os
import shutil

def build(apps):
    app_id = 0
    base_address = 0x80400000
    step = 0x20000
    linker = 'lib/arch/riscv/user.ld'
    for app in apps:
        lines = []
        lines_before = []
        with open(linker, 'r') as f:
            for line in f.readlines():
                lines_before.append(line)
                line = line.replace(hex(base_address), hex(base_address+step*app_id))
                lines.append(line)
        with open(linker, 'w+') as f:
            f.writelines(lines)
        
        print('[build.py] application %s start with address %s' %(app, hex(base_address+step*app_id)))
        with open(linker, 'w+') as f:
            f.writelines(lines_before)
        app_id = app_id + 1

if __name__ == '__main__':
    apps = os.listdir('srcfile/')
    apps.sort()
    os.remove([ for f in os.listdir('src/')])
    build(apps)

