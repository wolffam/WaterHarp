

import numpy as N
import numpy as np
import pygame
import pygame.surfarray as surfarray
from pygame.locals import *
from numpy import int32, uint8, uint

from DepthCameraOpenNI import DepthCameraOpenNI


def display_surface(array_img, name):
    "displays a surface, waits for user to continue"
    screen = pygame.display.set_mode(array_img.shape[:2], 0)
    surfarray.blit_array(screen, array_img)
    pygame.display.flip()
    pygame.display.set_caption(name)


openni_cam = DepthCameraOpenNI(None)

openni_cam.start()
dmap = openni_cam.get_dmap()

dtex = N.zeros((len(dmap[0]), len(dmap), 3), int32)

currrow = 0
currcol = 0

left_stream_index = 9
right_stream_index = 294

num_streams = 16
DMAP_CONSTANT = 100000


while(True):
    dmap = openni_cam.get_dmap()
    dmap[dmap == 0] = DMAP_CONSTANT
    x_bins = [int(x) for x in np.linspace(left_stream_index, right_stream_index, num_streams)]
    stream_indicators = []
    for x_bins_idx, left_idx in enumerate(x_bins[:-1]):
        mins = np.min(dmap[:, left_idx:x_bins[x_bins_idx + 1]], axis=0)  # get min for every column in AOI
        stream_indicators.append(np.mean(mins))

    print(stream_indicators)


    # print(len(dmap))
    # print(len(dmap[0]))
    # print(dtex.shape)
    minval = dmap.min().min()
    maxval = dmap.max().max()
    # print("min=",minval,", max=",maxval)
    currrow = 0
    currcol = 0
    for row in dmap:
        
        for col in row:            
            newval = 0
            if type(col) != np.nan:
              newval = (int) ((col-minval)*255/(maxval - minval))
            # print(newval);
            dtex[currcol][currrow][0] = newval


            currcol = currcol + 1
        currrow = currrow + 1
        currcol = 0


    display_surface(dtex, 'WaterHarp')
    print(pygame.mouse.get_pos())
    # e = pygame.event.wait()
    # if e.type == MOUSEBUTTONDOWN:

    #     print(pygame.mouse.get_pos())
    
    pygame.time.delay(100)