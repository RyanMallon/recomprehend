Generic Bugs
============

This list of bugs is almost certainly incomplete.

 * The code is a bit of a mess. It evolved out of a set of prototypes for
   reversing various bits of the original games.
 * Re-Comprehend is not very robust against corrupted game files.
 * Some game strings decode as garbage.
 * Some instruction opcodes are still unknown. Unknown commands do nothing,
   unknown tests return false.

Parser
------

 * The parser doesn't handle "and" clauses.
 * The parser combines word pairs that are next to each other only. This
   doens't work correclty for some join words. For example "put object in box"
   needs to be written as "putin box object" in Re-Comprehend ("putin" is
   a fake word in most of the games that is used when "put" and "in" are
   combined.
 * The save action opcode is not yet supported. This was used in the original
   games to allow the game to prompt for more information when a sentence was
   ambiguous. For example if the player types "push button", then game may
   reponse "which button?" if there is more than one, allowing the player to
   just respond with "red".
 * Re-Comprehend does not interrupt multiple sentences with "before you can
   continue" if something happens between sentences.

Graphics
--------

 * Title screens are not yet shown.
 * The original games used dithered floodfills to simulate more colors than
   the CGA display could manage. Re-Comprehend only supports solid colors.
 * The color choices could use some work.
 * The graphics floodfill operation occasionally bleeds. This appears to be
   caused by the original line algorithm being different to the SDL line
   algorithm used by Re-Comprehend leading to cases where regions are not
   properly closed.
 * The graphics shapes do not appear to be quite correct.
 * Text is graphics is currently shown as empty square boxes. A monospace font
   that matches the original needs to be used.
 * Graphical animations are not yet supported.


Game Specific Bugs
==================

Transylvania is completable.

Note that this section may contain spoilers (for thirty year old games).

Transylvania
------------

 * Various random events are not implemented in the game functions, and were
   instead probably hardcoded in the game binary. The following have not yet
   been implemented in Re-Comprehend.
    * The random messages on some turns.
    * The eagle which grabs the player and drops them elsewhere.
    * The goblin's antics.

Crimson Crown
-------------

Crimson Crown is completable.

Crimson Crown is split into two disk images which are actually separate games
(the player loses all items at the end of the first disk).

 * Re-Comprehend does not automatically transition between the game disks.
 * Save games must be loaded for the current disk.
 * Various random events are not implemented in the game functions, and were
   instead probably hardcoded in the game binary. The following have not yet
   been implemented in Re-Comprehend.
    * The sage who appears in disk 1 to offer advice.
    * The troll doesn't randomly appear to steal the scepter in disk 2.
 * The ending of disk 2 doesn't have the merchant ship arrive.

OO-Topos
--------

OO-Topos should be completable, however a full play through has not been done.

 * Some game strings display incorrectly. For example shooting the alien
   displays an odd response.
 * In the original game wearing the goggles displays the room in black
   and white lines only, and shows a hidden button in the garbage disposal.
   Re-Comprehend does not support this.

Talisman, Challenging the Sands of Time
---------------------------------------

This game cannot be played.

 * The header format appears different to the other games. Re-Comprehend will
   exit with a fatal error attempting to read the string table. Some game
   strings appear to be stored in plain text as opposed to the packed strings
   used by the other Comprehend games.
