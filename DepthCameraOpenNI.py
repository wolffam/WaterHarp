import numpy as np
# noinspection PyProtectedMember
from primesense import _openni2 as c_api
from primesense import openni2  # , nite2
from primesense.utils import OpenNIError

from DepthCamera import DepthCamera

# Path of the OpenNI redistribution OpenNI2.so or OpenNI2.dll
dist = './OpenNI-BuildFromSrc/OpenNI2/Bin/x64-Release'

class DepthCameraOpenNI(DepthCamera):
    def __init__(self, fp):
        super().__init__(fp)
        self.depth_stream = None
        # Initialize openni and check
        print("Starting openNI initialization")
        openni2.initialize(dist)  #
        if openni2.is_initialized():
            print("openNI2 initialized")
        else:
            print("openNI2 not initialized")

    def start(self):
        # Register the device
        try:
            dev = openni2.Device.open_any()
        except OpenNIError as exc:
            print("Unable to open any depth camera:", exc)
            raise Exception("Unable to open any depth camera")

        # Create the streams stream
        self.depth_stream = dev.create_depth_stream()

        # Configure the depth_stream -- changes automatically based on bus speed
        print('Get b4 video mode', self.depth_stream.get_video_mode())
        self.depth_stream.set_video_mode(
            c_api.OniVideoMode(pixelFormat=c_api.OniPixelFormat.ONI_PIXEL_FORMAT_DEPTH_1_MM, resolutionX=320,
                               resolutionY=240,
                               fps=30))

        # Check and configure the mirroring -- default is True
        # print 'Mirroring info1', depth_stream.get_mirroring_enabled()
        self.depth_stream.set_mirroring_enabled(False)

        # Start the streams
        self.depth_stream.start()
        return True

    def stop(self):
        if self.depth_stream is not None:
            self.depth_stream.stop()

    def get_dmap(self):
        """Return a current dmap as a 240x320 matrix of uint16 representing depth in mm"""
        dmap = np.fromstring(self.depth_stream.read_frame().get_buffer_as_uint16(),
                             dtype=np.uint16).reshape(240, 320)  # Works & It's FAST
        return dmap