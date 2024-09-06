# The Nomad
"The Nomad" is a 2D hack-and-slash fighting game where you play as a mercenary in a post-apocalypse world known as Bellatum Terrae.

## Current Build
The currently release version of the game is a Demo Alpha Test, so expect lots of bugs. If you do find a bug, please do me a favor and report it, after all, I can only fix bugs that I know about.

## WIP Features
Here's a complete list of things I plan to put into the game:
- Desert Sea Shanties
- Desert Brigs
- Mercenaries
- Mercenary Guilds
- Renown System
- Stealth Mechanics (cover, hiding spots, essentially a dumbed down version of AC Unity)
- Executions
- Zandatsu (ripping out the heart instead of the gatorade)
- Blade Mode
- Grenades
- Fear
- Shadow Mapping
- Light Mapping
- Shadow System
- Spotlights/Directional Lights
- Bladed Weapons
- Counter parries


## Requirements
### Minimum
Note that this engine is NOT multithreaded, it will be in the future however when I do a full-scale rewrite

__CPU__ | __GPU__ | __RAM__ | __OS__
--------|---------|---------|-------
Intel i5, 2 or more cores | IGPU is fine | 4 Gb | Windows 10 or Ubuntu

### Recommended
__CPU__ | __GPU__ | __RAM__ | __OS__
--------|---------|---------|-------
Intel i7, 4 or more cores | NVidia GTX 1050 Ti or better | 16 Gb | Windows 10 or Ubuntu

## The Technical Details
Unless you're into programming, this won't really be interesting...

### The Future
Since Steam is the place where I intend to publish the game in the future, I'll have to do a rewrite of the engine because it uses GPL v2'd code from the Quake III Arena engine.

### Graphics APIs
__Name__ | __Status__
---------|------------
OpenGL   | Implemented
Vulkan   | Coming soon
D3D11    | In the far future
Metal    | Unless OSX forces you to use it, probably never

Audio: OpenAL Soft

Everything else that's system specific is either handled by SDL2 or the engine
Future Features: custom simd implementations for standard library functions
