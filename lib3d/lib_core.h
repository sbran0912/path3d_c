#ifndef CORE_H
#define CORE_H

#define _POSIX_C_SOURCE 199309L

#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------
 * Zufall
 * ------------------------------------------------------------------ */
void random_init(void);
int random(int start, int end);
float randomFloat(float start, float end);

/* ------------------------------------------------------------------
 * Zahlen-Helfer
 * ------------------------------------------------------------------ */
float limit_num(float number, float limit);
float constrain_num(float value, float min, float max); /* begrenzt value auf [min, max] */
float map_num(float n, float range_old, float range_new); /* skaliert n auf einen neuen Bereich */

/* ------------------------------------------------------------------
 * String-Liste
 * ------------------------------------------------------------------ */
typedef struct {
    char **array;
    size_t capacity;
    size_t count;
} ListStr;

ListStr lists_create(size_t capacity);
void lists_append(ListStr *l, char val[]);
void lists_free(ListStr *l);
void lists_reduce(ListStr *l);
ListStr split(char string[], const char delimiter);

/* ------------------------------------------------------------------
 * 2D-Float-Matrix
 * ------------------------------------------------------------------ */
typedef struct {
    float **array;
    size_t rows;
    size_t cols;
} ListFloat2d;

ListFloat2d listf2d_create(size_t rows, size_t cols, float val);
void listf2d_free(ListFloat2d *l);

/* ------------------------------------------------------------------
 * Wachsendes Array (grosszuegig gepuffert, Elemente werden kopiert).
 * Achtung: arr_push / arr_reduce koennen den Speicher verschieben -
 * den Rueckgabewert immer zurueckspeichern!
 * ------------------------------------------------------------------ */
void  *arr_create(size_t capacity, size_t elemsize);
size_t arr_len(void *arr);
size_t arr_cap(void *arr);
void  *arr_push(void *arr, void *val);
void   arr_pop(void *arr, size_t idx);
void  *arr_reduce(void *arr);
void   arr_delete(void *arr);  /* setzt Laenge auf 0 (Kapazitaet bleibt) */
void   arr_free(void *arr);

/* ------------------------------------------------------------------
 * Einfach verkettete Liste
 * ------------------------------------------------------------------ */
typedef struct Node Node;

typedef struct {
    Node *head;
    Node *tail;
    size_t size;
} LinkedList;

void llist_push(LinkedList *list, void *data, size_t datasize);
void llist_pop(LinkedList *list);

/* ------------------------------------------------------------------
 * 2D-Vektor (float)
 * ------------------------------------------------------------------ */
typedef struct {
    float x, y;
} Vector2;

Vector2 vec2_scale(Vector2 v, float n);
Vector2 vec2_div(Vector2 v, float n);
Vector2 vec2_add(Vector2 v1, Vector2 v2);
Vector2 vec2_sub(Vector2 v1, Vector2 v2);
Vector2 vec2_norm(Vector2 v);
Vector2 vec2_limit(Vector2 v, float max);
Vector2 vec2_setMag(Vector2 v, float magnitude);
float   vec2_dot(Vector2 v1, Vector2 v2);
float   vec2_cross(Vector2 v1, Vector2 v2);
Vector2 vec2_perp(Vector2 v);
float   vec2_mag(Vector2 v);
float   vec2_magsq(Vector2 v);
float   vec2_dist(Vector2 v1, Vector2 v2);
Vector2 vec2_rotate(Vector2 v, Vector2 base, float n);
float   vec2_heading(Vector2 v);
float   vec2_angle(Vector2 v1, Vector2 v2);
float   vec2_minDist(Vector2 p, Vector2 start_a, Vector2 end_a);
Vector2 vec2_random(void);

/* ------------------------------------------------------------------
 * Hash-Map (generisch, Ersatz fuer Go's map[K]V)
 *
 * Werte und Schluessel werden kopiert (keysize/valsize). hash/eq duerfen
 * NULL sein - dann wird FNV-1a ueber die Schluesselbytes bzw. memcmp
 * verwendet.
 * ------------------------------------------------------------------ */
typedef uint64_t (*HashMap_HashFn)(const void *key);
typedef int (*HashMap_EqFn)(const void *keyA, const void *keyB);

typedef struct HmBucket HmBucket;

typedef struct HashMap {
    HmBucket **buckets;
    size_t     nbuckets;
    size_t     count;
    size_t     keysize;
    size_t     valsize;
    HashMap_HashFn hash;
    HashMap_EqFn   eq;
} HashMap;

HashMap *hm_create(int capacity, size_t keysize, size_t valsize,
                   HashMap_HashFn hash, HashMap_EqFn eq);
void     hm_free(HashMap *m);
void     hm_clear(HashMap *m);
size_t   hm_size(HashMap *m);
int      hm_contains(HashMap *m, const void *key);
void    *hm_get(HashMap *m, const void *key);   /* NULL, wenn abwesend */
void     hm_put(HashMap *m, const void *key, const void *val);
void     hm_remove(HashMap *m, const void *key);
void     hm_foreach(HashMap *m, void (*cb)(const void *key, void *val, void *ud),
                    void *ud);

#ifdef __cplusplus
}
#endif

#endif /* CORE_H */
