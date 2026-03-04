{
  description = "aga engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        nativeLibs = with pkgs; [
          libxkbcommon
          vulkan-loader
          vulkan-headers
          libx11
          libxcursor
          libxi
          libxinerama
          libxrandr
        ];
      in {
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            clang-tools
            cmake
            emscripten
            gcc
            git
            ninja
            pkg-config
            unzip
          ] ++ nativeLibs;

          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath nativeLibs;
        };

        packages.default = pkgs.stdenv.mkDerivation {
          pname = "aga-engine";
          version = "0.1.0";
          src = self;
          nativeBuildInputs = with pkgs; [ cmake ninja pkg-config unzip ];
          buildInputs = nativeLibs;
          cmakeFlags = [ "-DAGA_BUILD_DEMO=ON" ];
        };
      });
}
