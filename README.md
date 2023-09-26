
# NES emu

A NES emulator written in C++ for educational purpose.



## Build Linux

### Ubuntu / Debian
```bash
sudo apt-get install libsdl2-dev make g++ make

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Fedora
```bash
sudo dnf install SDL2-devel cmake gcc-c++ make

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Arch
```bash
sudo pacman -S sdl2 cmake make gcc

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

    
## Usage

```bash
./bin/nes_emu rom/path/romname.nes
```

## Roadmap

- [x]  CPU
- [x]  PPU
- [ ]  APU
- [x]  SDL renderer
- [x]  Mapper 0
- [ ]  Mapper 1
- [x]  Mapper 2
- [ ]  Mapper 3
- [ ]  Mapper 4
- [ ]  Mapper 5
