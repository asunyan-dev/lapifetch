{
    description = "lapifetch - Bunny-themed Linux fetch utility";

    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = { self, nixpkgs, flake-utils }:
        flake-utils.lib.eachDefaultSystem (system:
            let
                pkgs = import nixpkgs {
                    inherit system;
                };
            in
            {
                packages.default = pkgs.stdenv.mkDerivation {
                    pname = "lapifetch";
                    version = "0.5.0";

                    src = ./.;

                    nativeBuildInputs = with pkgs; [
                        cmake
                    ];

                    meta = with pkgs.lib; {
                        description = "Bunny-themed Linux fetch utility";
                        homepage = "https://github.com/asunyan-dev/lapifetch";
                        license = licenses.mit;
                        platforms = platforms.linux;
                    };
                };

                devShells.default = pkgs.mkShell {
                    packages = with pkgs; [
                        cmake
                        gcc
                        clang-tools
                    ];
                };
            }
        );
}