# kge

A game/game engine project I have been working on tentatively titled `kge`
(Kenny Game Engine). Written in C11 and only tested on MacOS so far. It not very
well planned. It will probably be used in/become a Touhou clone or a platformer.
It's really just a place for me to get used to OpenGL, but there are a few
specific things I hope this engine can do:  

- Run at any framerate without timing issues or judder  

- Have very little input lag  

- Be fully deterministic (replays work on any platform)  

## Building

Run `make` from within the `tools` directory. Right now, this just builds
a program called `resgen` that generates a C source representation of resource
files.  

Once the tools are built, you can run `make` from within the root directory to
build the demo. You will have to figure out dependencies on your own and may
have to modify the Makefile for your platform.  

## Stuff I am trying (or plan to try)

I doubt any of these ideas are brand new or cutting-edge, but they are the
things that interest me. I plan on either implementing them or learning why I
don't see them very often/ever in other projects.  

### Late rendering with vsync

With vsync, you normally start rendering as soon as the screen refreshes, and
the image gets pushed to the screen about 17ms later on a 60Hz display. If it
only takes 4ms to render your frame, why not wait 10ms before you render and
have the screen refresh 7ms behind instead?  

This could even be a slider. Maybe it could be calculated during a stress-test
or dynamically during gameplay. I would love to be able to have a low-latency
vsync option for people that don't have monitors with a variable refresh rate.  

### Separate input and physics from rendering

Input can be polled rapidly, and I expect simple physics can be simulated
quickly too. If this happens on a separate loop, a slow framerate will not
affect your gameplay beyond the delay pushing to the screen. My hope is that
even a slow computer running the game at 30Hz will still be able to capture
inputs hundreds of times per second.

### Variable (event driven?) physics rate

As long as nothing accelerates and collisions are kept really, really simple, it
might not be impossible to predict the next collision and make that time the
next physics step. Player inputs and screen renders would cause an instant
update. Something like this could enable determinism at variable frame rates.
