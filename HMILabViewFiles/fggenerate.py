from __future__ import print_function

from PIL import Image
import sys
import numpy as np


# call with image name
im = Image.open(sys.argv[1], mode='r')
has_alpha = im.mode == 'RGBA'

alpha_index = im.getbands().index('A')
sz = im.size
l0 = None
l1 = None
r0 = None
r1 = None

data = list(im.getdata(alpha_index))
for l in range(0,200):
    if l0 is None and min(data[l:sz[0]*sz[1]:sz[0]]) != 255:
        l0 = l
    if l1 is None and max(data[l:sz[0]*sz[1]:sz[0]]) == 0:
        l1 = l

for l in range(sz[0]-1,sz[0] - 200,-1):
    if r0 is None and min(data[l:sz[0]*sz[1]:sz[0]]) != 255:
        r0 = l
    if r1 is None and max(data[l:sz[0]*sz[1]:sz[0]]) == 0:
        r1 = l
        
print("left side, transition from", l0, "to", l1) 
print("right side, transition from", r0, "to", r1) 

# grid points, 5 over the transition?
nxdiv = 11
nydiv = 5
if l0 == l1:
    xc0 = np.array([0.0])
else:
    xc0 = np.zeros((nxdiv+1,))
    xc0[1:] = np.linspace(float(l0)/(sz[0]-1), float(l1)/(sz[0]-1), nxdiv)
if r0 == r1:
    xc1 = np.array([1.0])
else:
    xc1 = np.ones((nxdiv+1,))
    xc1[:-1] = np.linspace(float(r1)/(sz[0]-1), float(r0)/(sz[0]-1), nxdiv)

yc = np.linspace(1.0, 0.0, nydiv)
xc = np.concatenate((xc0, xc1))

# print(len(data), sz[0]*sz[1])

# print the file, if 1 full color, 0 is black
fout = open(sys.argv[1][:-4] + ".warp", 'w')
fout.write("screenwarp 1 %i %i 0 0 0 0\n" % (nydiv, xc.shape[0]))
for y in yc:
    for x in xc:
        #print (y, x)
        ix = int(round(x * (sz[0]-1)))
        iy = int(round(y * (sz[1]-1)))
        col = 1.0 - data[ix + iy * sz[0]] / 255.0
        #print(ix, iy, col)
        fout.write("%f %f %f %f %f\n" % (x, y, x, y, col))

fout.close()
