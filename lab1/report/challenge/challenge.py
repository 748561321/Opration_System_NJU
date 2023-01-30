#!/usr/bin/python
import os
str = input()
size = os.path.getsize(str)
if size < 510:
    file = open(str, mode='ab+')
    for i in range(size, 510):
        file.write(chr(0).encode())
    file.write(chr(0x55).encode())
    # file.write(chr(0xaa).encode())
    # 这样写会多一个c2 即上文为b'\xc2\xaa'
    file.write(b'\xaa')
    file.close()
