#ifndef PATHFINDING_H
#define PATHFINDING_H

/* ==================================================================
 * pathfinding - C-Port von pathfinding.go (A*-Suche im 2D-Gitter).
 *
 * Statt Go-Slices/-Maps werden die Datenstrukturen aus lib3d
 * (lib3d_util.c) verwendet:
 *   - OpenSet / Path  -> dynamisches Array arr_* (Ersatz fuer Slice)
 *   - walls / ClosedSet / nodes -> generische HashMap hm_* (Ersatz
 *     fuer Go's map)
 * ================================================================== */

#include "lib3d_util.h"

#ifdef __cplusplus
extern "C" {
#endif

// Point repraesentiert eine Koordinate im 2D-Gitter.
typedef struct {
    int x, y;
} Point;

// Solver haelt den Zustand der A*-Suche. OpenSet, ClosedSet, Current,
// Path und Done sind oeffentlich, damit der Renderer direkt darauf
// zugreifen kann - es gibt keinen Step-Zwischenschritt mehr.
typedef struct Solver {
    void     *openSet;    // arr_* von Point - noch offene Punkte
    HashMap  *closedSet;  // Point -> int - bereits verarbeitete Punkte
    Point     current;    // gerade verarbeiteter Punkt
    void     *path;       // arr_* von Point; NULL bis das Ziel erreicht ist
    int       done;       // Suche abgeschlossen

    HashMap  *walls;      // besetzte Felder (Point -> int)
    int       width;
    int       height;
    Point     goal;
    HashMap  *nodes;      // Suchdaten je Punkt (intern)
    void     *scratchPath; // temporaerer arr_* beim Pfadbau (intern)
} Solver;

// NewSolver legt den Suchzustand an. walls gehoert dem Aufrufer und
// wird NICHT freigegeben.
Solver *solver_new(HashMap *walls, int width, int height, Point start, Point goal);

// Next fuehrt genau einen Expansionsschritt der Suche aus.
void solver_next(Solver *s);

// CurrentPath liefert den anzuzeigenden Pfad: waehrend der Suche den
// bisherigen Teilpfad Start -> Current, nach Abschluss den kompletten
// Endpfad (NULL, wenn keine Loesung existiert). Aktuelle Laenge in
// *out_len.
Point *solver_current_path(Solver *s, size_t *out_len);

// Gibt den Solver samt interner Arrays/Maps frei (walls NICHT).
void solver_free(Solver *s);

// FindPath sucht per A* den kuerzesten Weg von start nach goal im Grid.
// Bequemlichkeits-Wrapper; liefert ein arr_* von Point oder NULL.
Point *pathfinding_find_path(HashMap *walls, int width, int height,
                             Point start, Point goal);

#ifdef __cplusplus
}
#endif

#endif // PATHFINDING_H
