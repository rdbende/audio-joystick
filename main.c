#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jack/jack.h>
#include <jack/midiport.h>

#define cubic_to_lin(fader) (pow ((fader), 2.5)) // Yeah, not exactly cubic, but this is better

jack_port_t *midi_input_port;
jack_port_t *in_l_port;
jack_port_t *in_r_port;
jack_port_t *fl_out_port;
jack_port_t *fr_out_port;
jack_port_t *rl_out_port;
jack_port_t *rr_out_port;
jack_client_t *client;

double fl_volume = 1.0;
double fr_volume = 1.0;
double rl_volume = 1.0;
double rr_volume = 1.0;

int
process (jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t *in_l, *in_r, *fl_out, *fr_out, *rl_out, *rr_out;
  jack_midi_event_t midi_event;
  void *midi_port_buffer = jack_port_get_buffer (midi_input_port, nframes);
  jack_nframes_t event_count = jack_midi_get_event_count (midi_port_buffer);

  for (int i = 0; i < event_count; i++)
    {
      jack_midi_event_get (&midi_event, midi_port_buffer, i);
      if ((midi_event.buffer[0] & 0xF0) == 0xB0)
        {
          if (midi_event.buffer[1] == 0)
            fl_volume = cubic_to_lin (midi_event.buffer[2] / 127.0);
          if (midi_event.buffer[1] == 1)
            fr_volume = cubic_to_lin (midi_event.buffer[2] / 127.0);
          if (midi_event.buffer[1] == 2)
            rl_volume = cubic_to_lin (midi_event.buffer[2] / 127.0);
          if (midi_event.buffer[1] == 3)
            rr_volume = cubic_to_lin (midi_event.buffer[2] / 127.0);
        }
    }

  in_l = jack_port_get_buffer (in_l_port, nframes);
  in_r = jack_port_get_buffer (in_r_port, nframes);
  fl_out = jack_port_get_buffer (fl_out_port, nframes);
  fr_out = jack_port_get_buffer (fr_out_port, nframes);
  rl_out = jack_port_get_buffer (rl_out_port, nframes);
  rr_out = jack_port_get_buffer (rr_out_port, nframes);

  for (jack_nframes_t frame = 0; frame < nframes; frame++)
    {
      fl_out[frame] = in_l[frame] * fl_volume;
      fr_out[frame] = in_r[frame] * fr_volume;
      rl_out[frame] = in_l[frame] * rl_volume;
      rr_out[frame] = in_r[frame] * rr_volume;
    }
  return 0;
}

void
jack_shutdown (void *arg)
{
  exit (1);
}

int
main (int argc, char *argv[])
{
  const char **ports;
  const char *client_name = "Audio Joystick";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client = jack_client_open (client_name, options, &status, server_name);
  if (client == NULL)
    {
      fprintf (stderr, "Couldn't create client, status = 0x%2.0x\n", status);
      if (status & JackServerFailed)
        {
          fprintf (stderr, "Couldn't connect to JACK server\n");
        }
      exit (1);
    }

  jack_set_process_callback (client, process, 0);
  jack_on_shutdown (client, jack_shutdown, 0);

  in_l_port = jack_port_register (client, "in_L", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  in_r_port = jack_port_register (client, "in_R", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

  fl_out_port = jack_port_register (client, "out_FL", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  fr_out_port = jack_port_register (client, "out_FR", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  rl_out_port = jack_port_register (client, "out_RL", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  rr_out_port = jack_port_register (client, "out_RR", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

  midi_input_port = jack_port_register (client, "MIDI", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

  if (jack_connect (client, jack_port_name (fl_out_port), "system:playback_1"))
    {
      fprintf (stderr, "Couldn't connect front left\n");
    }

  if (jack_connect (client, jack_port_name (fr_out_port), "system:playback_2"))
    {
      fprintf (stderr, "Couldn't connect front right\n");
    }

  if (jack_connect (client, jack_port_name (rl_out_port), "system:playback_3"))
    {
      fprintf (stderr, "Couldn't connect rear left\n");
    }

  if (jack_connect (client, jack_port_name (rr_out_port), "system:playback_4"))
    {
      fprintf (stderr, "Couldn't connect rear right\n");
    }

  if (jack_connect (client, "MIDI Joystick:out", jack_port_name (midi_input_port)))
    {
      fprintf (stderr, "Couldn't connectMIDI joystick\n");
    }

  if ((in_l_port == NULL) || (in_r_port == NULL) || (fl_out_port == NULL) || (fr_out_port == NULL) || (rl_out_port == NULL) || (rr_out_port == NULL))
    {
      fprintf (stderr, "No available ports\n");
      exit (1);
    }

  if (jack_activate (client))
    {
      fprintf (stderr, "Couldn't activate client");
      exit (1);
    }

  sleep (-1);

  jack_client_close (client);
  exit (0);
}

