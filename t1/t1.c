#include <stdio.h>
#include <stdlib.h>

typedef struct ponto {
    int x;
    int y;
    int id;
} ponto;


int main() {
    int n_aero;
    ponto *coords;
    int D = 10;

    scanf("%d", &n_aero);

    coords = malloc((size_t)n_aero * sizeof(ponto));

    if (coords == NULL) {
        return 1;
    }

    for (int a = 0; a < n_aero; a++) {
        scanf("%d %d", &coords[a].x, &coords[a].y);
        coords[a].id = a;
    }

    printf("%d\n", n_aero);

    for (int a = 0; a < n_aero; a++) {
        printf("%d %d %d\n", coords[a].x, coords[a].y, coords[a].id);
    }

    free(coords);

    return 0;
}
