import numpy as np
import pygame
import pygame.surfarray as surfarray
from numpy import int32

from WaterHarpAudio import WaterHarpAudio
from DepthCameraOpenNI import DepthCameraOpenNI


class WaterHarp:
    NUM_STREAMS = 16
    LEFT_STREAM_INDEX = 15
    RIGHT_STREAM_INDEX = 290
    TOP = 10
    BOTTOM = 230
    BACKGROUND = 50000
    THRESHOLD = 950  # Found through calibration

    def __init__(self):
        self.openni_cam = DepthCameraOpenNI(None)
        self.openni_cam.start()
        dmap = self.openni_cam.get_dmap()
        self.dtex = np.zeros((len(dmap[0]), len(dmap), 3), int32)
        self.audio = WaterHarpAudio()

    def display_surface(self, name="Water Harp"):
        "displays a surface, waits for user to continue"
        screen = pygame.display.set_mode(self.dtex.shape[:2], 0)
        surfarray.blit_array(screen, self.dtex)
        pygame.display.flip()
        pygame.display.set_caption(name)

    def play_video(self, dmap):
        minval = dmap.min().min()
        maxval = dmap.max().max()
        currrow = 0
        currcol = 0
        for row in dmap:

            for col in row:
                newval = 0
                if type(col) != np.nan:
                    newval = (int)((col - minval) * 255 / (maxval - minval))
                self.dtex[currcol][currrow][0] = newval

                currcol = currcol + 1
            currrow = currrow + 1
            currcol = 0

        self.display_surface('WaterHarp')
        # e = pygame.event.wait()
        # if e.type == MOUSEBUTTONDOWN:

        #     print(pygame.mouse.get_pos())

    def run(self):
        while True:
            dmap = self.openni_cam.get_dmap()
            dmap[dmap < WaterHarp.THRESHOLD / 2] = WaterHarp.BACKGROUND
            x_bins = [int(x) for x in np.linspace(WaterHarp.LEFT_STREAM_INDEX, WaterHarp.RIGHT_STREAM_INDEX, WaterHarp.NUM_STREAMS + 1)]
            stream_indicators = []
            for x_bins_idx, left_idx in enumerate(x_bins[:-1]):
                subarray = dmap[WaterHarp.TOP:WaterHarp.BOTTOM, left_idx:x_bins[x_bins_idx + 1]]
                below_threshold = np.where(subarray < WaterHarp.THRESHOLD)
                num_below = len(below_threshold[0]) * len(below_threshold[1])
                if num_below > 20:
                    height = np.mean(below_threshold[0])
                    volume = np.round(1 - (height / (240 - 0)), 2)  # 0 -> 1
                    stream_indicators.append(volume)
                else:
                    stream_indicators.append(0)

            self.audio.play_notes(stream_indicators)
            #self.play_video(dmap)

            #pygame.time.delay(1)


if __name__ == "__main__":
    WaterHarp().run()
