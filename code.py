import analogio
import board
import digitalio
import time

import usb_midi
import adafruit_midi
from adafruit_midi.control_change import ControlChange


x_axis = analogio.AnalogIn(board.A0)
y_axis = analogio.AnalogIn(board.A1)

run_pin = digitalio.DigitalInOut(board.GP23)
run_pin.direction = digitalio.Direction.OUTPUT
run_pin.value = True  # This smoothens ADC reads somewhat

midi = adafruit_midi.MIDI(midi_out=usb_midi.ports[1])

last_x_value = 0
last_y_value = 0


while True:
    x_value = max(x_axis.value // 256 - 1, 0)
    y_value = max(y_axis.value // 256 - 1, 0)

    FL = min(min(x_value, 127), min(255 - y_value, 127))
    FR = min(min(255 - x_value, 127), min(255 - y_value, 127))
    RL = min(min(x_value, 127), min(y_value, 127))
    RR = min(min(255 - x_value, 127), min(y_value, 127))

    midi.send(ControlChange(0, FL))
    midi.send(ControlChange(1, FR))
    midi.send(ControlChange(2, RL))
    midi.send(ControlChange(3, RR))

    time.sleep(0.002)
