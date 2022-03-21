cd ~/Downloads/ || exit
pwd
git clone --depth 1 https://github.com/libsdl-org/SDL.git
cd SDL || exit
mkdir build
cd build || exit
../configure
make
echo "Please enter your sudo password."
sudo make install
git clone --depth 1 https://github.com/libsdl-org/SDL_ttf.git
cd SDL_ttf || exit
mkdir build
cd build || exit
../configure
make
echo "Please enter your sudo password."
sudo make install
git clone --depth 1 https://github.com/libsdl-org/SDL_image.git
cd SDL_image || exit
mkdir build
cd build || exit
../configure
make
echo "Please enter your sudo password."
sudo make install