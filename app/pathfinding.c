#include "pathfinding.h"

#include <limits.h>
#include <stdlib.h>

/* ==================================================================
 * A*-Pfadsuche (Port von pathfinding.go)
 * ================================================================== */

// Die 4 orthogonalen Bewegungsrichtungen.
static Point directions[4] = {
    {0, -1},  // hoch
    {0, 1},   // runter
    {-1, 0},  // links
    {1, 0},   // rechts
};

// Suchdaten eines Punktes (intern; entspricht Go's Node).
typedef struct {
    int   g, f;
    Point parent;
    int   hasParent; // false beim Startknoten
} PathNode;

// Manhattan-Distanz als Heuristik (4 Richtungen).
static int heuristic(Point a, Point b) {
    int absX = a.x - b.x;
    if (absX < 0) absX = -absX;
    int absY = a.y - b.y;
    if (absY < 0) absY = -absY;
    return absX + absY;
}

// Prueft, ob ein Punkt im Gitter liegt und nicht blockiert ist.
static int is_valid(Point p, int width, int height, HashMap *walls) {
    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height &&
           !hm_contains(walls, &p);
}

// Sortier-Kontext fuer qsort: die nodes-Map des aktuellen Solvers.
static HashMap *sort_nodes = NULL;

static int point_by_f_cmp(const void *pa, const void *pb) {
    const Point *a = (const Point *) pa;
    const Point *b = (const Point *) pb;
    const PathNode *na = (const PathNode *) hm_get(sort_nodes, a);
    const PathNode *nb = (const PathNode *) hm_get(sort_nodes, b);
    int fa = na ? na->f : INT_MAX;
    int fb = nb ? nb->f : INT_MAX;
    return (fa > fb) - (fa < fb);
}

static int arr_contains_point(void *arr, Point p) {
    const Point *data = (const Point *) arr;
    size_t n = arr_len(arr);
    for (size_t i = 0; i < n; i++) {
        if (data[i].x == p.x && data[i].y == p.y) return 1;
    }
    return 0;
}

// Laeuft ueber die Parent-Verweise vom Ziel zurueck zum Start; der Pfad
// wird umgedreht zurueckgegeben (arr_* von Point).
static Point *reconstruct_path(HashMap *nodes, Point current) {
    void *path = arr_create(16, sizeof(Point));
    path = arr_push(path, &current);
    for (;;) {
        const PathNode *n = (const PathNode *) hm_get(nodes, &current);
        if (!n || !n->hasParent) break;
        current = n->parent;
        path = arr_push(path, &current);
    }
    // Pfad wurde rueckwaerts gebaut, also umdrehen.
    size_t cnt = arr_len(path);
    Point *arr = (Point *) path;
    for (size_t i = 0; i < cnt / 2; i++) {
        Point tmp = arr[i];
        arr[i] = arr[cnt - 1 - i];
        arr[cnt - 1 - i] = tmp;
    }
    return path;
}

Solver *solver_new(HashMap *walls, int width, int height, Point start, Point goal) {
    Solver *s = (Solver *) calloc(1, sizeof(Solver));
    s->walls  = walls;
    s->width  = width;
    s->height = height;
    s->goal   = goal;
    s->closedSet = hm_create(width * height + 4, sizeof(Point), sizeof(int),
                             NULL, NULL);
    s->nodes = hm_create(width * height + 4, sizeof(Point), sizeof(PathNode),
                         NULL, NULL);

    // Alle begehbaren Punkte vorab mit "unendlich" anlegen, damit in der
    // Schleife kein Sonderfall fuer neue Knoten noetig ist.
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Point p = {x, y};
            if (!hm_contains(walls, &p)) { // begehbar
                PathNode n = {INT_MAX, INT_MAX, {0, 0}, 0};
                hm_put(s->nodes, &p, &n);
            }
        }
    }
    // Der Startknoten bekommt direkt seine echten Werte.
    PathNode st = {0, heuristic(start, goal), {0, 0}, 0};
    hm_put(s->nodes, &start, &st);

    s->openSet = arr_create(64, sizeof(Point));
    s->openSet = arr_push(s->openSet, &start);
    s->scratchPath = NULL;
    return s;
}

void solver_next(Solver *s) {
    if (s->done) return;

    // Aufsteigend nach F sortieren: kleinstes F an Index 0. Jeder Punkt im
    // OpenSet hat garantiert einen Eintrag in nodes.
    sort_nodes = s->nodes;
    qsort(s->openSet, arr_len(s->openSet), sizeof(Point), point_by_f_cmp);
    sort_nodes = NULL;

    s->current = *((Point *) s->openSet); // Index 0

    // Ziel erreicht?
    if (s->current.x == s->goal.x && s->current.y == s->goal.y) {
        s->path = reconstruct_path(s->nodes, s->current);
        s->done = 1;
        return;
    }

    arr_pop(s->openSet, 0);

    int one = 1;
    hm_put(s->closedSet, &s->current, &one);

    const PathNode *curNode = (const PathNode *) hm_get(s->nodes, &s->current);
    int currentG = curNode ? curNode->g : INT_MAX;

    for (int d = 0; d < 4; d++) {
        Point neighbor;
        neighbor.x = s->current.x + directions[d].x;
        neighbor.y = s->current.y + directions[d].y;
        if (!is_valid(neighbor, s->width, s->height, s->walls)) continue;
        // Bereits endgueltig verarbeitet -> ueberspringen.
        if (hm_contains(s->closedSet, &neighbor)) continue;

        // Ein Schritt kostet immer 1. Existiert garantiert: alle begehbaren
        // Punkte wurden vorab angelegt.
        int tentativeG = currentG + 1;
        PathNode *n = (PathNode *) hm_get(s->nodes, &neighbor);

        // Kuerzerer Weg gefunden als bisher bekannt? ("unendlich" = erster
        // Fund).
        if (n && tentativeG < n->g) {
            n->g = tentativeG;
            n->f = tentativeG + heuristic(neighbor, s->goal);
            n->parent = s->current;
            n->hasParent = 1;

            if (!arr_contains_point(s->openSet, neighbor)) {
                s->openSet = arr_push(s->openSet, &neighbor);
            }
        }
    }

    if (arr_len(s->openSet) == 0) s->done = 1; // Kein Pfad gefunden
}

Point *solver_current_path(Solver *s, size_t *out_len) {
    if (s->done) {
        if (out_len) *out_len = s->path ? arr_len(s->path) : 0;
        return (Point *) s->path;
    }
    // Temporaerer Pfad (Start -> Current); vorherigen scratch freigeben.
    if (s->scratchPath) {
        arr_free(s->scratchPath);
        s->scratchPath = NULL;
    }
    s->scratchPath = reconstruct_path(s->nodes, s->current);
    if (out_len) *out_len = arr_len(s->scratchPath);
    return (Point *) s->scratchPath;
}

void solver_free(Solver *s) {
    if (!s) return;
    if (s->openSet)     arr_free(s->openSet);
    if (s->path)        arr_free(s->path);
    if (s->scratchPath) arr_free(s->scratchPath);
    if (s->closedSet)   hm_free(s->closedSet);
    if (s->nodes)       hm_free(s->nodes);
    free(s);
}

Point *pathfinding_find_path(HashMap *walls, int width, int height,
                             Point start, Point goal) {
    Solver *s = solver_new(walls, width, height, start, goal);
    while (!s->done) solver_next(s);
    Point *result = (Point *) s->path;
    s->path = NULL; // Eigentum an den Aufrufer uebertragen
    solver_free(s);
    return result;
}
