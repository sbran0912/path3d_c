#include <stdio.h>
#include <stdlib.h>

#include "lib_core.h"
#include "lib3d.h"
#include "pathfinding.h"

/* ====================================================================
 * path3d - Grundgeruest fuer spaetere Pfadsuche (C-Port von main.go).
 *
 * Ein Bodengitter aus 40x40 Feldern. Auf zufaellig verteilten Feldern
 * steht je ein Wuerfel, der genau ein Feld bedeckt. A*-Suche vom Feld
 * (0,0) zum Feld (39,39); die Suche laeuft Schritt fuer Schritt und
 * wird animiert dargestellt.
 *
 * Konvention (wie im Go-Original): col zeigt in X-Richtung, row in
 * Z-Richtung. Das Gitter liegt zentriert um den Ursprung in der
 * XZ-Ebene (y=0).
 * ================================================================== */

#define EDGE_CELLS  40                            /* Felder pro Achse */
#define CELL_SIZE   1.0                           /* Kantenlaenge eines Feldes */
#define EDGE_LENGTH (EDGE_CELLS * CELL_SIZE)      /* Kantenlaenge des Gitters */
#define CUBE_SIZE   0.9                           /* Wuerfelkante */

/* --- Globale Szenen-/Loop-Daten (Ersatz fuer die Go-Closure) --- */
static Solver    *g_solver     = NULL;
static Scene      g_scene;
static Mesh       g_cubeMesh;
static Mesh       g_gridMesh;
static ShapeStyle g_gridStyle;
static ShapeStyle g_cubeStyle;
static void      *instances    = NULL; /* arr_* von Instance */

/* cellCenter rechnet Weltkoordinaten der Feldmitte fuer ein Feld
 * (row, col). col -> X-Achse, row -> Z-Achse, Feldmitte +0.5-Verschiebung. */
static void cellCenter(int row, int col, double *x, double *z) {
    double half = EDGE_LENGTH / 2.0;
    *x = -half + ((double) col + 0.5) * CELL_SIZE;
    *z = -half + ((double) row + 0.5) * CELL_SIZE;
}

/* random_grid erzeugt eine Map mit besetzten Feldern (Wand) fuer ein
 * width x height-Grid mit zufaellig verteilten Waenden. */
static void random_grid(int width, int height, double wallChance,
                        HashMap *walls) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (l3d_random_float(0.0, 1.0) < wallChance) {
                Point p = {x, y};
                int one = 1;
                hm_put(walls, &p, &one);
            }
        }
    }
}

/* --- Kontext fuer den cube_cb-Callback --- */
typedef struct {
    void      *instances; /* arr_* von Instance */
    ColorState col;
} CubeCtx;

static void *append_cube(void *instances, double x, double z, ColorState col) {
    Instance in;
    in.model = mat4_translate(x, CUBE_SIZE / 2.0, z);
    in.color = col;
    return arr_push(instances, &in);
}

static void cube_cb(const void *key, void *val, void *ud) {
    (void) val;
    CubeCtx *ctx = (CubeCtx *) ud;
    const Point *p = (const Point *) key;
    double x = 0.0, z = 0.0;
    cellCenter(p->x, p->y, &x, &z);
    ctx->instances = append_cube(ctx->instances, x, z, ctx->col);
}

/* draw_cubes haengt fuer jede Gruppe (Waende, Closed-/Open-Set, Pfad)
 * eine Wuerfel-Instanz an und liefert das (evtl. verschobene) Array zurueck. */
static void *draw_cubes(void *arr, Solver *solver) {
    CubeCtx ctx;

    /* Besetzte Grid-Felder (Waende) in rot */
    ctx.instances = arr;
    ctx.col = parse_hex("#ff0000");
    hm_foreach(solver->walls, cube_cb, &ctx);
    arr = ctx.instances;

    /* Bereits verarbeitete Punkte (orange) */
    ctx.instances = arr;
    ctx.col = parse_hex("#ffcc33");
    hm_foreach(solver->closedSet, cube_cb, &ctx);
    arr = ctx.instances;

    /* Noch offene Punkte (hellblau) */
    ctx.instances = arr;
    ctx.col = parse_hex("#87CEEB");
    {
        Point *open = (Point *) solver->openSet;
        size_t n = arr_len(solver->openSet);
        for (size_t i = 0; i < n; i++) {
            double x = 0.0, z = 0.0;
            cellCenter(open[i].x, open[i].y, &x, &z);
            arr = append_cube(arr, x, z, ctx.col);
        }
    }
    ctx.instances = arr;

    /* Bisher gefundener Pfad (gruen) */
    ctx.col = parse_hex("#87eba2");
    {
        size_t n = 0;
        Point *path = solver_current_path(solver, &n);
        if (path) {
            for (size_t i = 0; i < n; i++) {
                double x = 0.0, z = 0.0;
                cellCenter(path[i].x, path[i].y, &x, &z);
                arr = append_cube(arr, x, z, ctx.col);
            }
        }
    }

    return arr;
}

/* Loop (Ersatz fuer die draw-Closure in Go). */
static void draw(void) {
    if (!g_solver->done) {
        solver_next(g_solver);
        if (g_solver->done) {
            size_t n = 0;
            Point *path = solver_current_path(g_solver, &n);
            if (path) {
                printf("Pfad gefunden (%d Schritte): ", (int) n);
                for (size_t i = 0; i < n; i++) {
                    printf("[%d, %d] ", path[i].x, path[i].y);
                }
                printf("\n");
            } else {
                printf("Kein Pfad möglich.\n");
            }
        }
    }

    l3d_begin_scene(g_scene);

    /* Bodengitter (XZ-Ebene bei y=0) aus dem geteilten Mesh. */
    l3d_draw_mesh(g_gridMesh, g_gridStyle, NULL);

    /* Alle Wuerfel in EINEM Instancing-Draw-Call. */
    arr_delete(instances); /* Laenge auf 0, Kapazitaet bleibt */
    instances = draw_cubes(instances, g_solver);

    l3d_draw_mesh_instanced(g_cubeMesh, g_cubeStyle,
                           (const Instance *) instances,
                           (int) arr_len(instances));
}

int main(void) {
    random_init(); /* gleicher Seed-Mechanismus wie lib_core */

    /* Zufaelliges Gitter; Start und Ziel duerfen keine Waende sein. */
    HashMap *walls = hm_create(4096, sizeof(Point), sizeof(int), NULL, NULL);
    random_grid(EDGE_CELLS, EDGE_CELLS, 0.3, walls);

    Point start = {0, 0};
    Point goal = {EDGE_CELLS - 1, EDGE_CELLS - 1};
    hm_remove(walls, &start);
    hm_remove(walls, &goal);

    g_solver = solver_new(walls, EDGE_CELLS, EDGE_CELLS, start, goal);

    g_scene = scene_new();
    g_scene.cam.pos    = vec3(17, 24, 32);
    g_scene.cam.target = vec3(0, 0, 0);
    g_scene.cam.up     = vec3(0, 1, 0);
    g_scene.cam.far    = 500;

    /* Meshes EINMAL erzeugen und wiederverwenden. */
    g_cubeMesh = make_cube_mesh(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE);
    g_gridMesh = make_grid_mesh(EDGE_LENGTH, EDGE_CELLS);

    g_gridStyle.stroke    = color_hex("#777774");
    g_gridStyle.lineWidth = 1;

    g_cubeStyle.stroke    = color_hex("#6b5200");
    g_cubeStyle.fill      = color_hex("#ffffff"); /* wird pro Instanz ueberschrieben */
    g_cubeStyle.lineWidth = 1;
    g_cubeStyle.mode      = STYLE_BOTH;

    instances = arr_create((size_t) EDGE_CELLS * EDGE_CELLS, sizeof(Instance));

    if (!l3d_init()) {
        fprintf(stderr, "path3d: Initialisierung fehlgeschlagen\n");
        return 1;
    }

    l3d_set_batch_sort(1);
    l3d_start_animation(draw);

    /* Aufraeumen (nach dem Schliessen des Fensters). */
    arr_free(instances);
    mesh_free(&g_cubeMesh);
    mesh_free(&g_gridMesh);
    solver_free(g_solver);
    hm_free(walls);

    return 0;
}
