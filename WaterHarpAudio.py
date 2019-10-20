import os
from typing import List
from typing import Tuple

import pygame


class WaterHarpAudio:
    NUM_STREAMS = 16
    NUM_AUDIO_CHANNELS = NUM_STREAMS * 10
    l = os.listdir(".")
    MUSIC_DIR = os.path.join(os.path.dirname(__file__), 'data/sounds/Fingerstyle_Electric_Base')
    NOTES = sorted([x for x in os.listdir(MUSIC_DIR) if x.endswith(".wav")], key=lambda x: int(x.split("_")[0]))

    def __init__(self):
        self.current_channel = 0
        self.last_kinect_array = None

        pygame.mixer.init()
        pygame.mixer.set_num_channels(WaterHarpAudio.NUM_AUDIO_CHANNELS)

    def play_notes(self, kinect_array: List[float]):
        channel_and_volume = [None] * WaterHarpAudio.NUM_STREAMS  # type: List[Tuple[int, float]]
        for idx, volume in enumerate(kinect_array):
            if volume > 0.001:  # if not zero
#                if not channel_and_volume[idx][0] or not pygame.mixer.Channel(channel_and_volume[idx][0]).get_busy():  # If channel not assigned or not playing
                sound_filepath = os.path.join(WaterHarpAudio.MUSIC_DIR, WaterHarpAudio.NOTES[idx])
                sound = pygame.mixer.Sound(sound_filepath)
                sound.set_volume(volume)
                bounded_channel = self.current_channel % WaterHarpAudio.NUM_AUDIO_CHANNELS
                pygame.mixer.Channel(bounded_channel).play(sound)
                channel_and_volume[idx] = (bounded_channel, volume)  # Assign channel and volume currently playing to the proper stream
                self.current_channel += 1
        self.last_kinect_array = kinect_array
