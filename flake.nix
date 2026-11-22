{
  description = "USOS timetable integration for Discord";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      llvm-compiler = pkgs.llvmPackages_22;
      usos-rpc =
        {
          stdenv,
          lib,
          cmake,
          ninja,
        }:
        stdenv.mkDerivation {
          pname = "usos-rpc";
          version = "1.1.0";
          src = lib.fileset.toSource {
            root = ./.;
            fileset = lib.fileset.unions [
              ./CMakeLists.txt
              ./src
              ./resources
              ./discord-rpc.patch
            ];
          };

          nativeBuildInputs = [
            cmake
            ninja
          ];
          buildInputs = [
            # TODO: move some dependencies here so that everything builds purely
          ];
          enableParallelBuilding = true;

          installPhase = ''
            mkdir -p $out/bin
            cp ../bin/* $out/bin/
          '';

          meta.mainProgram = "usos-rpc";
        };
    in
    {
      packages.${system} = pkgs.lib.fix (self: {
        usos-rpc-gcc = pkgs.callPackage usos-rpc { };
        usos-rpc-llvm = pkgs.callPackage usos-rpc {
          inherit (llvm-compiler) stdenv;
        };
        default = self.usos-rpc-llvm;
      });
      devShells.${system}.default =
        pkgs.mkShell.override
          {
            # clangd works better than C++ Intellisense extension in VS Code
            inherit (llvm-compiler) stdenv;
          }
          {
            packages = with pkgs; [
              cmake
              ninja
              openssl # TODO: remove after moving curl to dependencies

              llvm-compiler.clang-tools # Necessary for clangd to find the C++ standard library headers
            ];
          };
    };
}
