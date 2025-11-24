# The Bikeshed browser (temporary name) 

A simple and truly independent (*cough cough*) browser written in plain C code.

## Quickstart
### Dependencies
You'll need the following for building raylib:
```sh
$ sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```
### Build & run
First, clone the repository and update the `fonts/` git module:
```sh
 $ git clone https://github.com/bikeshed-browser/browser --depth=1
...
 $ git submodule update --init --depth 1 fonts
```
Bikeshed uses the [nob.h](https://github.com/tsoding/nob.h) library for it's fully C build system. Build the build system with your system's compiler:
```sh
 $ cc nob.c -o nob
```
Then build the project:
```sh
 $ ./nob
```
And finally, you can run any HTML file in `examples/`. For example:
```sh
 $ ./bikeshed examples/barebones.html
...
 $ ./bikeshed examples/motherfuckingwebsite.html
```

## Philosophy & goals
Bikeshed was somewhat the effect of our group's talks about the [Ladybird](https://github.com/LadybirdBrowser/ladybird) browser's development team's tendency to seem to focus on less important things (microoptimisations, browser stuff that has no effect on the web engine, etc.) which took away from the important urgent things (aka actually having a usable browser engine that is spec compliant). This is where the name [Bikeshedding](https://en.wikipedia.org/wiki/Law_of_triviality) came from. We intend to do what's important early, and what we *want* when it's needed.

On top of that though, we also just wanna have a fun community project to work on! This is quite an interesting project, and having a good ol' read of the CSS+HTML5 specifications is always fun :)

## Contributing
Checkout [CONTRIBUTE.md](CONTRIBUTE.md)

## Licensing
Bikeshed is under the Mozilla Public License 2.0. See `LICENSE` for more information. If you contribute to Bikeshed, you agree to your code being licensed under MPL 2.0 as well.
