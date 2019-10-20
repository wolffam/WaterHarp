import os
import time
from random import randint

import pygame


class WaterHarpAudio:
    NUM_STREAMS = 16
    NUM_AUDIO_CHANNELS = NUM_STREAMS * 10
    MUSIC_DIR = "./data/sounds/Fingerstyle Electric Base"
    NOTES = sorted([x for x in os.listdir(MUSIC_DIR) if x.endswith(".wav")], key=lambda x: int(x.split("_")[0]))

    def __init__(self):
        self.streams_update = None  # type: StreamsUpdate
        self.current_channel = 0

        pygame.mixer.init()
        pygame.mixer.set_num_channels(WaterHarpAudio.NUM_AUDIO_CHANNELS)

    def play_notes(self):
        for idx, state in enumerate(self.streams_update.stream_states):
            if state:
                sound_filepath = os.path.join(WaterHarpAudio.MUSIC_DIR, WaterHarpAudio.NOTES[idx])
                sound = pygame.mixer.Sound(sound_filepath)
                pygame.mixer.Channel(self.current_channel % WaterHarpAudio.NUM_AUDIO_CHANNELS).play(sound)
                self.current_channel += 1
        self.streams_update = None


class StreamsUpdate:
    def __init__(self):
        self.kinect_array = None
        self.stream_states = None
        self.start_channel = None

    @staticmethod
    def from_kinect(kinect_array):
        streams_update = StreamsUpdate()
        streams_update.kinect_array = kinect_array
        streams_update.parse_kinect_array()
        return streams_update

    def parse_kinect_array(self):
        self.stream_states = []
        #  ***Add kinect parsing code here***
        for i in range(16):
            self.stream_states.append(False)
        self.stream_states[2] = True


audio = WaterHarpAudio()
audio.streams_update = StreamsUpdate()

audio.streams_update.stream_states = [False] * 16
random_int = randint(0, 15)
audio.streams_update.stream_states[random_int] = True
random_int = randint(0, 15)
audio.streams_update.stream_states[random_int] = True
random_int = randint(0, 15)
audio.streams_update.stream_states[random_int] = True


audio.play_notes()
time.sleep(2)
