# Papyrus Chess

![Menu screen](https://github.com/joaoreboucas1/papyrus_chess/blob/main/assets/menu.gif "Menu screen")

**NOTE: this is a work in progress!**

Papyrus chess is an *antique*-themed (i.e. ugly) chess program.

## Build Dependencies

The developers who want to build the project by themselves must have [`raylib`](https://www.raylib.com/index.html) installed. The compilation process is usual:

```console
$ gcc -o chess chess.c -lraylib -mwindows
```

**NOTE:** the `-mwindows` flag suppresses the terminal from opening every time the application is run. For debugging purposes, this flag may be omitted during compilation. Check [this issue](https://github.com/raysan5/raylib/issues/324) for building with MSVC.

The code should be cross-platform so it should compile in both Windows and Linux.

## Running the Program

Just run the executable.

## TODO

Figure out a way to ship the application, since the font and assets are loaded dynamically.