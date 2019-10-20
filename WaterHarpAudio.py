import os

import pygame


class WaterHarpAudio:
    NUM_STREAMS = 16
    NUM_AUDIO_CHANNELS = NUM_STREAMS * 10
    MUSIC_DIR = "data/sounds/Fingerstyle Electric Base"
    NOTES = sorted([x for x in os.listdir(MUSIC_DIR) if x.endswith(".wav")], key=lambda x: int(x.split("_")[0]))

    def __init__(self):
        self.current_channel = 0

        pygame.mixer.init()
        pygame.mixer.set_num_channels(WaterHarpAudio.NUM_AUDIO_CHANNELS)

    def play_notes(self, kinect_array):
        assert len(kinect_array) == WaterHarpAudio.NUM_STREAMS, "Check kinect array length!"
        for idx, height in enumerate(kinect_array):
            assert 0 <= height <= 1, "Height not between 0, 1!"
            if height > 0.001:  # if not zero
                sound_filepath = os.path.join(WaterHarpAudio.MUSIC_DIR, WaterHarpAudio.NOTES[idx])
                sound = pygame.mixer.Sound(sound_filepath)
                sound.set_volume(height)
                pygame.mixer.Channel(self.current_channel % WaterHarpAudio.NUM_AUDIO_CHANNELS).play(sound)
                self.current_channel += 1


audio = WaterHarpAudio()
audio.play_notes(ARRAY)