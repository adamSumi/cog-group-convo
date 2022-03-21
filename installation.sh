function clone_and_install() {
    cd ~/Downloads/ || exit
    pwd
    git clone --depth 1 "$1" "$2"
    cd $2 || exit
    mkdir build
    cd build || exit
    ../configure
    make
    echo "Please enter your sudo password."
    sudo make install
}

clone_and_install https://github.com/libsdl-org/SDL.git SDL
clone_and_install https://github.com/libsdl-org/SDL_ttf.git SDL_ttf
clone_and_install https://github.com/libsdl-org/SDL_image.git SDL_image