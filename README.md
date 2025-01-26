# audio-joystick
Use a joystick to pan audio in four directions

This is a small client application that runs on the JACK audio server, and converts a stereo input to four output channels. The panning is determined by the incoming MIDI CC messages. You can basically pan the sound wherever you want in the room, by simply moving the MIDI joystick.
The MIDI joystick is made with a Rapsberry Pi Pico, and a basic resistive joystick mounted on a rather ugly PCB. The Pico runs CircuitPython, and sends out MIDI messages via USB. The code for it is in `code.py`
