all: main.c
	gcc main.c -L/usr/lib64/pipewire-0.3/jack/ -ljack -lm -o audio_joystick
