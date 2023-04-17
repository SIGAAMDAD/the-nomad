# GL-Nomad
this is an optimized, improved version of my previous game, TheNomad-ASCII. This game now uses 2d hardware-accelerated rendering with optimizations for very specific hardware and platforms. You'll need modern stuff to run this.

## The Technical Details
ATTENTION: UNLESS YOU ARE MODDING THIS OR ARE INTERESTED IN HOW THE LOW-LEVEL SOFTWARE WORKS, DON'T READ THIS

This game uses custom implementations of standard library memmove, memcpy, memset, etc. ultilizing the insane performance gains of avx2. The zone (if you look at the log, you'll occassionally see a heap dump) allocates on a 1024 byte alignment.