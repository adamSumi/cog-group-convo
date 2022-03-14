# Captioning on Glass: Group Conversations

## Prerequisites

### VLC

You must have the VLC media player installed on your system. You can install it from [here](https://www.videolan.org/).
We rely on the code that VLC runs on (libvlc), so please ensure that your VLC version is >= 3.0.16.

### FFmpeg

You should install FFmpeg on your system. For most Linux-based systems, you can get this from your package manager. If
not, you can get it [from their website](https://ffmpeg.org/).

### Building dependencies from source

Unfortunately, due to some features being used by this project that are not in package manager repositories, you'll have
to build some dependencies from source code.

The following instructions were performed on Ubuntu 20.04, but will _probably_ work on Unix-like systems (i.e. MacOS)

#### SDL

This project depends heavily on a library called [SDL](https://libsdl.org), or **S**imple **D**irectMedia **L**ayer. SDL
is in charge of rendering most of our content to the screen.

##### Instructions

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

#### SDL_ttf

SDL actually doesn't render the text to the screen, another library
called [SDL_ttf](https://github.com/libsdl-org/SDL_ttf) or SDL **T**rue **T**ype **F**ont is in charge of that.

##### Instructions

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

#### SDL_image

Lastly, we rely on a library called [SDL_image](https://github.com/libsdl-org/SDL_image) for rendering our assets (
indicators pointing in the direction of the next speaker).

##### Instructions

Perform the following _OUTSIDE OF THIS REPOSITORY_. Your "Downloads" folder will work fine.

```shell
git clone https://github.com/libsdl-org/SDL_image
cd SDL_image
mkdir build
cd build
../configure
make
sudo make install
```

## Videos

Our work depends heavily upon the use of a 3.22GB recording (it's 4K and 10 minutes long) of a group conversation.
You should obtain a copy of that video file, copy it into the `resources/videos` folder, naming `main.mp4` if it isn't already.

You'll notice that `resources/videos` already has a file in it: `split.sh`. This script splits `main.mp4` into 4 roughly-equal-sized chunks, using `ffmpeg` (which you should have already installed).

Run the script (i.e. `bash split.sh`), and you should be good for development!

## Development

Most development of this repository has been done using [CLion](https://www.jetbrains.com/clion/), which is the
recommended development tool for this repository.
[JetBrains offers free educational licenses to students](https://www.jetbrains.com/community/education/#students), which
should make getting CLion a cinch.