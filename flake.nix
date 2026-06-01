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
                    version = "1.4.0";

                    src = ./.;

                    nativeBuildInputs = with pkgs; [
                        cmake
                        pkg-config
                    ];

                    buildInputs = with pkgs; [
                        hwdata
                        libX11
                        libXrandr
                    ];

                    postPatch = ''
                        substituteInPlace src/system.cpp \
                         --replace-fail "usr/share/hwdata/pci.ids" \
                                        "${pkgs.hwdata}/share/hwdata/pci.ids"
                        echo "" >> CMakeLists.txt
                        echo "target_link_libraries(lapifetch X11 Xrandr)" >> CMakeLists.txt
                    '';

                    meta = with pkgs.lib; {
                        description = "Bunny-themed Linux fetch utility";
                        homepage = "https://github.com/asunyan-dev/lapifetch";
                        license = licenses.mit;
                        platforms = platforms.linux;
                    };
                };
            }
        );
}