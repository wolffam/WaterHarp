from typing import List

import mido
import pygame


class WaterHarpAudio:
    NUM_STREAMS = 16
    NUM_AUDIO_CHANNELS = NUM_STREAMS * 10
    TURN_OFF_STREAMS = True
    # MUSIC_DIR = os.path.join(os.path.dirname(__file__), 'data/sounds/MelodiousBass')
    # NOTES = sorted([x for x in os.listdir(MUSIC_DIR) if x.endswith(".wav")], key=lambda x: int(x.split("_")[0]), reverse=True)
    CAMILLE_SCALE = (57, 60, 62, 64, 67, 69, 72, 74, 76, 79, 81, 84, 86, 88, 91, 93)
    C_MIXOLYDIAN_SCALE = (48, 50, 52, 53, 55, 57, 58, 60, 62, 64, 65, 67, 69, 70, 72, 74)
    C_HARMONIC_MINOR_SCALE = (48, 50, 51, 53, 55, 56, 59, 60, 62, 63, 65, 67, 68, 71, 72, 74)
    C_DORIAN_MINOR_SCALE = (48, 50, 51, 53, 55, 57, 58, 60, 62, 63, 65, 67, 69, 70, 72, 74)

    def __init__(self):
        self.current_channel = 0
        self.stream_channels = [None] * WaterHarpAudio.NUM_STREAMS
        self.last_kinect_array = [0] * WaterHarpAudio.NUM_STREAMS

        pygame.mixer.init()
        pygame.mixer.set_num_channels(WaterHarpAudio.NUM_AUDIO_CHANNELS)
        self.midi_outport = mido.open_output(name='test', virtual=True)

    def play_notes(self, kinect_array: List[float]):
        # Find notes to play
        if sum(kinect_array) < .0005:
            self.all_notes_off()
        else:
            for idx, volume in enumerate(kinect_array):
                if volume > 0.001 >= self.last_kinect_array[idx]:  # If the note was OFF previously and is now ON
                    self.play_note(idx, volume)
                elif volume < 0.001 <= self.last_kinect_array[idx]:
                    self.stop_note(idx)

                    #print("PLAYED: {}, VOLUME: {}".format(WaterHarpAudio.C_MAJOR_SCALE[idx], volume))

                if WaterHarpAudio.TURN_OFF_STREAMS:
                    if volume <= 0.001 < self.last_kinect_array[idx]:  # IF the note was ON previously and is now OFF
                        if not self.stream_channels[idx]:
                            continue
                        if self.stream_channels[idx].get_busy():  # Is the channel actually playing
                            self.stream_channels[idx].stop()  # Turn off channel
                            self.stream_channels[idx] = None
        self.last_kinect_array = kinect_array

    def all_notes_off(self):
        self.midi_outport.reset()

    def stop_note(self, note_idx):
        msg = mido.Message('note_off', channel=0, note=WaterHarpAudio.C_MIXOLYDIAN_SCALE[note_idx], velocity=127)
        self.midi_outport.send(msg)

    def play_note(self, note_idx, volume):
        msg = mido.Message('note_on', channel=0, note=WaterHarpAudio.C_MIXOLYDIAN_SCALE[note_idx], velocity=max(int(127 * volume), 40))
        self.midi_outport.send(msg)

        #  Play sounds from files, not MIDI
        # sound_filepath = os.path.join(WaterHarpAudio.MUSIC_DIR, WaterHarpAudio.NOTES[note_idx])
        # sound = pygame.mixer.Sound(sound_filepath)
        # sound.set_volume(volume)
        # bounded_channel = self.current_channel % WaterHarpAudio.NUM_AUDIO_CHANNELS
        # chan = pygame.mixer.Channel(bounded_channel).play(sound)
        # self.stream_channels[note_idx] = chan
        # self.current_channel += 1
