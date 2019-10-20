import logging
import cv2
import numpy as np
import scipy.io as sio

from datetime import datetime
from pyOSC3 import *


class DepthCamera(object):

    def __init__(self, fp):
        self.fp = fp
        self.mindist = 500  # Dist in mm
        self.maxdist = 2000
        self.zoom = 1  # Zoom factor of sensor
        self.aim = [0, 0]  # Relative position on sensor to center (-1:1)
        self.blur = 3
        self.erode = 1
        self.savecnt = 0
        self.saveDepth = []
        self.refreshGUI = True

    def start(self):
        return True

    def stop(self):
        pass

    def addOSCHandlers(self, server: OSCServer):
        # Add any OSC handlers specific to this app

        def rangeHandler(path, tags, args, source):
            logging.info("rangeHandler %s %s %s %s", path, tags, args, source)
            if path[-1] == 'n':
                self.mindist = args[0]
            else:
                self.maxdist = args[0]
            if self.maxdist < self.mindist + 10:
                self.maxdist = self.mindist + 10
            self.refreshGUI = True

        def zoomHandler(path, tags, args, source):
            logging.info("zoomHandler %s %s %s %s", path, tags, args, source)
            self.zoom = args[0]
            self.refreshGUI = True

        def aimHandler(path, tags, args, source):
            logging.info("aimHandler %s %s %s %s", path, tags, args, source)
            self.aim[0] = args[0]
            self.aim[1] = args[1]
            self.refreshGUI = True

        def captureHandler(path, tags, args, source):
            logging.info("captureHandler %s %s %s %s", path, tags, args, source)
            if args[0] == 1:
                self.savecnt = 20

        server.addMsgHandler(address="/depth/range/min", callback=rangeHandler)
        server.addMsgHandler(address="/depth/range/max", callback=rangeHandler)
        server.addMsgHandler(address="/depth/zoom", callback=zoomHandler)
        server.addMsgHandler(address="/depth/aim", callback=aimHandler)
        server.addMsgHandler(address="/depth/capture", callback=captureHandler)

    def setGUI(self):
        # Refresh controls
        self.fp.oscSet("/depth/range/min", self.mindist)
        self.fp.oscSet("/depth/range/max", self.maxdist)
        self.fp.oscSet("/depth/zoom", self.zoom)
        self.fp.oscSet("/depth/aim", self.aim)
        self.refreshGUI = False

    def get_dmap(self):
        """Return a current dmap as a 240x320 matrix of uint16 representing depth in mm"""
        return None  # Implemented by subclasses

    def get_depth(self):
        """
        Returns numpy ndarrays representing the raw and ranged depth images.
        Outputs:
            dmap:= distancemap in mm, 1L ndarray, dtype=uint16, min=0, max=2**12-1
            d4d := depth for dislay, 3L ndarray, dtype=uint8, min=0, max=255
        Note1:
            fromstring is faster than asarray or frombuffer
        Note2:
            .reshape(120,160) #smaller image for faster response
                    OMAP/ARM default video configuration
            .reshape(240,320) # Used to MATCH RGB Image (OMAP/ARM)
                    Requires .set_video_mode
        """
        dmap = self.get_dmap()
        if len(self.saveDepth) < self.savecnt:
            self.saveDepth.append(dmap)
            if len(self.saveDepth) == self.savecnt:
                now = datetime.now()
                filename = 'depth-%s.mat' % now.strftime('%Y%m%d_%H%M%S')
                print("Saving %d depth frames to %s..." % (len(self.saveDepth), filename))
                sio.savemat(filename, {'depth': self.saveDepth})
                self.saveDepth = []
                self.savecnt = 0  # Stop saving

        d4d = np.uint8(dmap.astype(float) * 255 / 2 ** 12 - 1)  # Correct the range. Depth images are 12bits
        d4d = cv2.cvtColor(d4d, cv2.COLOR_GRAY2RGB)
        # Shown unknowns in black
        d4d = 255 - d4d
        # Display the stream
        cv2.imshow('depth', d4d)
        # Convert depth=0 and large depths to maxdepth
        maxDepth = 9000
        dmap = np.where(dmap == 0, maxDepth, dmap)
        # Filter the stream
        if self.blur > 0:
            dmap = np.float32(cv2.medianBlur(dmap, int(self.blur * 2 + 1)))
        # And erode to capture closest object after downsampling
        if self.erode > 0:
            kernel = np.ones((self.erode * 2 + 1, self.erode * 2 + 1), np.uint8)
            dmap = cv2.erode(dmap, kernel)
        # Handle zoom and pan
        sz = (int(240 / self.zoom), int(320 / self.zoom))
        center = [120.0 + 90.0 * self.aim[0], 160.0 - 120.0 * self.aim[1]]
        if center[0] - sz[0] / 2 < 0:
            center[0] = sz[0] / 2
        elif center[0] + sz[0] / 2 > 240:
            center[0] = 240 - sz[0] / 2
        if center[1] - sz[1] / 2 < 0:
            center[1] = sz[1] / 2
        elif center[1] + sz[1] / 2 > 320:
            center[1] = 320 - sz[1] / 2
        # print("sz=", sz, ", center=", center)
        patch = cv2.getRectSubPix(dmap, sz, tuple(center))
        return patch
