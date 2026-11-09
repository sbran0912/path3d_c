# Build

Voraussetzungen: CMake, Ninja, GLFW, OpenGL sowie beide Compiler (clang und gcc).

Es gibt zwei parallele Builds, die sich nicht gegenseitig stören:

| Build       | Compiler | Ausgabe         | Artefakt-Verzeichnis |
| ----------- | -------- | --------------- | -------------------- |
| CMake/Ninja | clang    | `build/path3d`  | `build/`             |
| Make        | gcc      | `bin/path3d`    | `bin/`, `obj/`       |

## CMake + Ninja (clang)

Aus dem Projektverzeichnis `path3d_c` heraus:

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

Das Binary liegt danach unter `build/path3d`.

Nach Code-Änderungen genügt:

```sh
cmake --build build
```

Neu konfigurieren (`cmake -S . -B build -G Ninja`) muss man nur, wenn sich
`CMakeLists.txt` ändert, neue Dateien hinzukommen oder `build/` gelöscht wurde.

> Hinweis: Der Compiler ist in `CMakeLists.txt` fest auf `clang` gesetzt. Ein
> bereits mit einem anderen Compiler konfiguriertes `build/` muss vorher
> gelöscht werden (`rm -rf build`). Überschreiben lässt sich der Compiler mit
> `-DCMAKE_C_COMPILER=...`.

## Make (gcc)

```sh
make          # baut bin/path3d
make clean    # entfernt bin/ und obj/
```

## Starten

```sh
./build/path3d     # clang-Build
./bin/path3d       # gcc-Build
```

Beide öffnen ein GLFW/OpenGL-Fenster und brauchen daher eine grafische Umgebung.

## Editor / clangd

Der CMake-Build erzeugt `build/compile_commands.json` (clang), auf die der
Symlink `compile_commands.json` im Projektwurzelverzeichnis zeigt. Damit findet
clangd die Compile-Flags automatisch. Der Symlink ist nur gültig, solange
`build/` existiert.
