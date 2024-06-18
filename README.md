# cig

Conventional commits cli, written in `C`

![demo](./assets/demo.gif)

## Dependencies

- readline
- libgit2

### Arch install:
```bash
sudo pacman -S libgit2 readline
```
### Ubuntu install:
```bash
sudo apt-get install libgit2-dev libreadline-dev
```

Apply the ubuntu patch, because libgit2-dev appears to be an old version:
```bash
patch < ./ubuntu.patch
```

## Configure

Edit [config.def.h](./config.def.h)

Running `make` will copy the file to `config.h`

## Build

`make`

## Install

```bash
sudo make install
```
