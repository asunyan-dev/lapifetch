#!/usr/bin/env bash

set -e

require() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "Error: '$1' is required but not installed."
        exit 1
    }
}

require git
require make
require g++

REPO_URL="https://github.com/asunyan-dev/lapifetch.git"
DIR_NAME="lapifetch"

echo "===> Cloning repository..."
git clone "$REPO_URL"

cd "$DIR_NAME"

echo "===> Installing..."
sudo make install

cd ..
rm -rf "$DIR_NAME"

echo
echo "lapifetch installed successfully!"
echo "Run it with: lapifetch"