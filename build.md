# Build

Voraussetzungen: CMake, Ninja, GLFW, OpenGL.

Aus dem Projektverzeichnis `path3d_c` heraus:

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

Das Binary liegt danach unter `build/path3d`.

## Starten

```sh
./build/path3d
```

## Rebuild

Nach Code-Änderungen genügt:

```sh
cmake --build build
```

Neu konfigurieren (`cmake -S . -B build -G Ninja`) muss man nur, wenn sich
`CMakeLists.txt` ändert, neue Dateien hinzukommen oder `build/` gelöscht wurde.
