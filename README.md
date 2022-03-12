# Captioning on Glass: Group Conversations

## Prerequisites

Unfortunately, due to some features being used by this project that are not in package manager repositories, you'll have
to build from source. But don't worry! It's not too bad.

The following instructions were performed on Ubuntu 20.04, but will _probably_ work on Unix-like systems (i.e. MacOS)

### SDL

This project depends heavily on a library called [SDL](https://libsdl.org), or **S**imple **D**irectMedia **L**ayer. SDL
is in charge of rendering most of our content to the screen.

#### Instructions

Perform the following _OUTSIDE OF THIS REPOSITORY_. Your "Downloads" folder will work fine.

```shell
git clone https://github.com/libsdl-org/SDL
cd SDL
mkdir build
cd build
../configure
make
sudo make install
```

### SDL_ttf

SDL actually doesn't render the text to the screen, another library
called [SDL_ttf](https://github.com/libsdl-org/SDL_ttf) or SDL **T**rue **T**ype **F**ont is in charge of that.

#### Instructions

Perform the following _OUTSIDE OF THIS REPOSITORY_. Your "Downloads" folder will work fine.

```shell
git clone https://github.com/libsdl-org/SDL_ttf
cd SDL_ttf
mkdir build
cd build
../configure
make
sudo make install
```

### VLC

With any luck, you should now have both SDL and SDL_ttf installed, which should take care of the dependency issues for
this repo.

You should now obtain a copy of the 3.22GB video file used to power this experiment. Once obtained, copy it into
the `resources/videos` folder, under the name `main.mp4`.

## Development

Most development of this repository has been done using [CLion](https://www.jetbrains.com/clion/), which is the
recommended development tool for this repository.
[JetBrains offers free educational licenses to students](https://www.jetbrains.com/community/education/#students), which
should make getting CLion a cinch.