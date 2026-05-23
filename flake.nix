{
  description = "Development environment for fossil";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages_latest;
      in
      {
        devShells.default = pkgs.mkShell.override { stdenv = llvm.stdenv; } {

          nativeBuildInputs = [
            llvm.clang-tools
            llvm.clang
            llvm.llvm
            pkgs.cmake
            pkgs.gdb
            pkgs.pkg-config
          ];

          buildInputs = [
            llvm.libcxx
            pkgs.gtest
            pkgs.gbenchmark
          ];

          packages = [
            (pkgs.python3.withPackages (
              ps: with ps; [
                pip
                setuptools
                numpy
                matplotlib
              ]
            ))
            pkgs.ripgrep
          ];

          shellHook = ''
            export CXXFLAGS="$NIX_CFLAGS_COMPILE"
          '';
        };
      }
    );
}
