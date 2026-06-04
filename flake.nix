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
          libffi
          libxkbcommon
          vulkan-loader
          vulkan-headers
          wayland
          wayland-protocols
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
            wayland-scanner
          ] ++ nativeLibs;

          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath nativeLibs;
        };
      });
}
