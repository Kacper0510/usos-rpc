{
  description = "USOS timetable integration for Discord";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      llvm-compiler = pkgs.llvmPackages_22;

      battery-embed-src = pkgs.fetchFromGitHub {
        owner = "batterycenter";
        repo = "embed";
        rev = "v1.2.19";
        hash = "sha256-P5DuCD3C8NCfPJn+n4xFD6bTT4oxafPq9IuRc6deM3Q=";
      };

      discord-rpc-src = pkgs.applyPatches {
        src = pkgs.fetchFromGitHub {
          owner = "discord";
          repo = "discord-rpc";
          rev = "963aa9f3e5ce81a4682c6ca3d136cddda614db33";
          hash = "sha256-vz2djopNQNbCIT+HwrIc0UVVXn/UPa0rZSaO6/knhDA=";
        };
        patches = [ ./discord-rpc.patch ];
      };

      usos-rpc =
        {
          stdenv,
          lib,
          cmake,
          ninja,
          pkg-config,
          curlMinimal,
          tomlplusplus,
          rapidjson,
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
            ];
          };

          nativeBuildInputs = [
            cmake
            ninja
            pkg-config
          ];
          buildInputs = [
            curlMinimal
            tomlplusplus
            rapidjson
          ];
          cmakeFlags =
            let
              cmakeSrc = name: path: lib.cmakeOptionType "path" "FETCHCONTENT_SOURCE_DIR_${name}" (toString path);
            in
            [
              (cmakeSrc "BATTERY-EMBED" battery-embed-src)
              (cmakeSrc "DISCORD-RPC" discord-rpc-src)
              (lib.cmakeFeature "git_commit_string" (self.dirtyShortRev or self.shortRev))
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
      packages.${system} = {
        usos-rpc-gcc = pkgs.callPackage usos-rpc { };
        usos-rpc-llvm = pkgs.callPackage usos-rpc {
          inherit (llvm-compiler) stdenv;
        };
        default = self.packages.${system}.usos-rpc-llvm;
      };

      devShells.${system}.default =
        pkgs.mkShell.override
          {
            # clangd works better than C++ Intellisense extension in VS Code
            inherit (llvm-compiler) stdenv;
          }
          {
            inputsFrom = [ self.packages.${system}.default ];
            packages = [
              llvm-compiler.clang-tools # Necessary for clangd to find the C++ standard library headers
            ];
          };
    };
}
