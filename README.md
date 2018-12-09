# PACMAN C

A basic version of the popular game 'pacman' made in C and using ASCII to draw the screen & the characters. Runs in linux systems.

## Installation:

In order to run the game, you will need the ncurses library installed in your system.

You can compile it using:

```
gcc pacman.c -lncurses -pthread
```

and you can run it using:

```
./a.out <H> <W> <FILENAME>
```

where H is the map's height, W is the map's width and FILENAME is the map's location.
For a quick game, you can use:

```
./a.out 11 19 mapa.txt
```

## Map:

The map will look like this:

```
*******************
*........*........*
*.**.*********.**.*
*.................*
*.*.***** *****.*.*
*.*.*1 2   3 4*.*.*
*.*.***********.*.*
*........P........*
*.**.*********.**.*
*........*........*
*******************
```

Where:
- 1,2,3,4 are the ghosts
- P is PACMAN
- the '*' are the walls
- the dots are the points.

## Keys:

- w -> up
- a -> left
- s -> down
- d -> right
- q -> quit game

## Levels:

Every time you finnish a level, the ghosts speed up. Good Luck!