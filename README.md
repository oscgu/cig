# cig

Conventional commits cli, written in `C`

## Dependencies

- readline
- libgit2

Arch install:
```bash
sudo pacman -S libgit2 readline
```

## Build

`make`

## Configure

Edit [config.def.h](./config.def.h)

Running `make` will copy the file as `config.h`
