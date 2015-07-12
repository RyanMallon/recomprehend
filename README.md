Re-Comprehend
=============

Re-Comprehend is an implementation of the Comprehend/Graphics Magician engines
used to implement the following Penguin Software games:

 * Transylvania
 * Crimson Crown
 * OO-Topos
 * Talisman, Challenging the Sands of Time (not currently working)

A history of Penguin Software, the Comprehend engine and the Graphics Magician
software can be found here:

 * www.filfre.net/2014/06/comprehend/
 * graphicsmagician.com/polarware/compadv.htm

License
-------

Re-Comprehend is public domain. The code may be freely incorporated into other
projects, either source or binary.

Building Re-Comprehend
----------------------

Re-Comprehend requires LibSDL2. Building has only been tested on Linux, but
it should be trivially portable to other operating systems.

To build:

```
make
```

Playing the Games
-----------------

To play the games you need a copy of the original DOS version of the game. To
my knowledge all of the games are now abandon-ware (the are available for
download from the Graphics Magician website).

To run a game, start Re-Comprehend with the short code for the game and the
path to copy of the original game. For example:

```
./recomprehend tr /local/games/dosbox/transylvania
```

To see a list of options and the supported games:

```
./recomprehend --help
```

Note the some of the games require knowledge from the manuals distributed with
the original games in order to succesfully complete them.

To quit Re-Comprehend type "!quit" at the game prompt.

Save and Restore
----------------

Saving and restoring is implemented in all the games using the original file
format to maintain backwards compatibility. The "save" and "restore" commands
in Re-Comprehend will save or restore from the original game directory passed
on the command line.

Debugging
---------

Re-Comprehend provides support for debugging the games. Command line options
are provided (see --help) for dumping human readable game data. When playing
debug commands can be entered by prefixing them with a "!" character. The
following debug commands are supported:

 * debug: Toggle debug mode. When debug mode is enabled verbose debugging of
   functions will be printed.
 * dump objects: Dump information about that current state of all objects in
   the game.
 * dump rooms: Dump information about the current state of all rooms in the
   game.
 * dump state: Dump information about the current game state.
