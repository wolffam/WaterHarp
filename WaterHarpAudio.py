import os
from typing import List

import pygame


class WaterHarpAudio:
    NUM_STREAMS = 16
    NUM_AUDIO_CHANNELS = NUM_STREAMS * 10
    TURN_OFF_STREAMS = True
    l = os.listdir(".")
    MUSIC_DIR = os.path.join(os.path.dirname(__file__), 'data/sounds/Fingerstyle_Electric_Base')
    NOTES = sorted([x for x in os.listdir(MUSIC_DIR) if x.endswith(".wav")], key=lambda x: int(x.split("_")[0]))

    def __init__(self):
        self.current_channel = 0
        self.stream_channels = [None] * WaterHarpAudio.NUM_STREAMS
        self.last_kinect_array = [0] * WaterHarpAudio.NUM_STREAMS

        pygame.mixer.init()
        pygame.mixer.set_num_channels(WaterHarpAudio.NUM_AUDIO_CHANNELS)

    def play_notes(self, kinect_array: List[float]):
        # Find notes to play
        for idx, volume in enumerate(kinect_array):
            if volume > 0.001 >= self.last_kinect_array[idx]:  # If the note was OFF previously and is now ON
                self.play_note(idx, volume)
                print("PLAYED: {}, VOLUME: {}".format(WaterHarpAudio.NOTES[idx].split("_")[-1], volume))

            if WaterHarpAudio.TURN_OFF_STREAMS:
                if volume <= 0.001 < self.last_kinect_array[idx]:  # IF the note was ON previously and is now OFF
                    if not self.stream_channels[idx]:
                        continue
                    if self.stream_channels[idx].get_busy():  # Is the channel actually playing
                        self.stream_channels[idx].stop()  # Turn off channel
                        self.stream_channels[idx] = None
        self.last_kinect_array = kinect_array

    def play_note(self, note_idx, volume):
        sound_filepath = os.path.join(WaterHarpAudio.MUSIC_DIR, WaterHarpAudio.NOTES[note_idx])
        sound = pygame.mixer.Sound(sound_filepath)
        sound.set_volume(volume)
        bounded_channel = self.current_channel % WaterHarpAudio.NUM_AUDIO_CHANNELS
        chan = pygame.mixer.Channel(bounded_channel).play(sound)
        self.stream_channels[note_idx] = chan
        self.current_channel += 1
