{
  description = "fastlex: branchless constexpr ASCII character classification";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages_latest;
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "fastlex";
          version = "0.1.0";
          src = pkgs.lib.cleanSource ./.;

          nativeBuildInputs = [
            pkgs.cmake
            pkgs.pkg-config
          ];

          buildInputs = [
            pkgs.gtest
            pkgs.gbenchmark
          ];

          cmakeFlags = [
            "-DFASTLEX_BUILD_TESTS=ON"
            "-DFASTLEX_BUILD_BENCHMARKS=ON"
            "-DFASTLEX_BUILD_EXAMPLES=ON"
          ];

          doCheck = true;
          checkPhase = ''
            runHook preCheck
            ctest --output-on-failure
            runHook postCheck
          '';
        };

        checks.default = self.packages.${system}.default;

        devShells.default = pkgs.mkShell.override { stdenv = llvm.stdenv; } {

          nativeBuildInputs = [
            llvm.clang-tools
            llvm.clang
            llvm.llvm
            pkgs.gcc14
            pkgs.cmake
            pkgs.ninja
            pkgs.gdb
            pkgs.pkg-config
          ];

          buildInputs = [
            llvm.libcxx
            pkgs.gtest
            pkgs.gbenchmark
          ];

          shellHook = ''
            export CXXFLAGS="$NIX_CFLAGS_COMPILE"
          '';
        };
      }
    );
}
