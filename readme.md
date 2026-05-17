# lapifetch

lapifetch is a simple bunny-themed Linux fetch utility written in C++.

*Note: This is a WIP. There might be some bugs.*

## Dependencies

All you need is a c++ compiler, like gcc.

## Install

### Arch

You can install the package from the AUR. Simply run:

```bash
$ yay -S lapifetch
```

### Debian

Check the latest release and download the `.deb` file and run it.

### Nix

To run directly:

```
nix run github:asunyan-dev/lapifetch
```

Install permanently:

```
nix profile install github:asunyan-dev/lapifetch
```

### Install with bash

Simply run the following command:

```bash
$ curl -fsSL https://raw.githubusercontent.com/asunyan-dev/lapifetch/main/install.sh | bash
```

## Install/Build from Source

- Clone the repository and go to the directory:

```bash
$ git clone https://github.com/asunyan-dev/lapifetch

$ cd lapifetch
```

### Build

To build, simply run:

```bash
$ make
```

### Install

To install to the command, run:

```bash
$ sudo make install

$ make clean
```

- You need `sudo` permission to install, cause it will be copied to `/usr/local/bin`.
- `make clean` removes the building files that are unnecessary after the installation.


## Configuration

Run:

```bash
$ lapifetch --gen-config
```

This will generate two files, `color` and `order` in the following path:
`$HOME/.config/lapifetch`

When you open these files you will see the default configs:

`color`:

```
purple
```

`order`:

```
os
kernel
uptime
packages
cpu
gpu
de
terminal
shell
ram
swap
root
```

You can edit them.
Check:

```bash
$ lapifetch --color-info # for color

$ lapifetch --order-info # for order
```

For the order you can also remove fields if you wanna hide them.

## Usage

For a basic usage, simply run:

```bash
$ lapifetch
```

If you want to see more information, for example to hide the bunny art, run:

```bash
$ lapifetch --help
```

This will show you all the information you need.

### Screenshots

Example of `lapifetch` with pink color:

![Lapifetch with pink](screenshots/lapifetch-pink.png)

Now `lapifetch` but without the bunny art:

![Lapifetch without art](screenshots/lapifetch-noart.png)

And with GPU information + blue color

![Lapifetch with GPU](screenshots/lapifetch-withgpu.png)

## Uninstall

To uninstall the command, go to the repo folder you've cloned and run:

```bash
$ sudo make uninstall
```

## Contact

If you have any bug, issue, or advice to give, always feel free to contact me on discord: `@lapinou112`.