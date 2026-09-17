/*
    DUPLA:
    João Gabriel Almeida de Lucena - RA 281255
    João Murilo Coutinho Martins - RA 249798
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <limits.h>

/*
 * ============================================================
 *  ESTRATÉGIA USADA:
 * 
 *  Closest Pairs - Sweep Line
 *
 *  Fase 1:
 *      encontra a menor distância quadrática D*
 *
 *  Fase 2:
 *      enumera todos os pares cuja distância quadrática é D*
 *
 *  Distâncias são sempre comparadas ao quadrado (por eficiência):
 *
 *      D(p,q) = (xp-xq)^2 + (yp-yq)^2
 *
 *  Não usamos ponto flutuante para comparar distâncias.
 * ============================================================
 */

// guarda as coordenadas do ponto e o indice original dele no input.
// o id continua sendo o mesmo mesmo depois da ordenacao.

typedef struct {
    int64_t x;
    int64_t y;
    int id;
} Point;

// representa um par de avioes usando os ids originais.

typedef struct {
    int a;
    int b;
} Pair;

// vetor dinamico usado para guardar os pares encontrados.

typedef struct {
    Pair *data;
    size_t size;
    size_t capacity;
} PairVector;

// inicializa o vetor de respostas.

static void pair_vector_init(PairVector *v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

// adiciona um par ao vetor de respostas.
// deixamos o menor id sempre em a para facilitar a ordenacao depois.

static void pair_vector_push(PairVector *v, int a, int b) {
    if (a > b) {
        int tmp = a;
        a = b;
        b = tmp;
    }

    if (v->size == v->capacity) {
        size_t new_capacity =
            (v->capacity == 0) ? 1024 : v->capacity * 2;

        Pair *new_data =
            realloc(v->data, new_capacity * sizeof(Pair));

        if (!new_data) {
            fprintf(stderr, "Erro: memoria insuficiente.\n");
            exit(EXIT_FAILURE);
        }

        v->data = new_data;
        v->capacity = new_capacity;
    }

    v->data[v->size].a = a;
    v->data[v->size].b = b;
    v->size++;
}

static void pair_vector_free(PairVector *v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

// calcula a distancia ao quadrado entre dois pontos.
// assim evitamos sqrt durante a comparacao das distancias.

static int64_t squared_distance(const Point *a, const Point *b) {
    int64_t dx = a->x - b->x;
    int64_t dy = a->y - b->y;

    return dx * dx + dy * dy;
}

// qsort usado na ordenacao de aviões
// ordena os pontos pela coordenada x.
// em caso de empate usamos y e depois o id.

static int compare_points(const void *pa, const void *pb) {
    const Point *a = (const Point *)pa;
    const Point *b = (const Point *)pb;

    if (a->x < b->x) return -1;
    if (a->x > b->x) return 1;

    if (a->y < b->y) return -1;
    if (a->y > b->y) return 1;

    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;

    return 0;
}

// cada no da AVL guarda um ponto ativo e os dois filhos.
// a arvore fica ordenada por (y, id), e nao por x.

typedef struct AVLNode {
    Point *point;

    int height;

    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

// compara dois pontos usando a chave da AVL: primeiro y e depois id.
// o id garante que pontos com o mesmo y continuem tendo chaves diferentes.

static int compare_key(const Point *a, const Point *b) {
    if (a->y < b->y) return -1;
    if (a->y > b->y) return 1;

    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;

    return 0;
}


// retorna a altura do no. um no inexistente tem altura 0.

static int avl_height(AVLNode *node) {
    return node ? node->height : 0;
}


static int max_int(int a, int b) {
    return a > b ? a : b;
}


// atualiza a altura depois de uma insercao, remocao ou rotacao.

static void update_height(AVLNode *node) {
    if (node) {
        node->height =
            1 + max_int(
                avl_height(node->left),
                avl_height(node->right)
            );
    }
}


// mede o balanceamento do no.
// em uma AVL valida o valor fica entre -1 e 1.

static int balance_factor(AVLNode *node) {
    if (!node) return 0;

    return avl_height(node->left) -
           avl_height(node->right);
}

// rotacao para a direita quando a arvore fica pesada para a esquerda.

static AVLNode *rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *t = x->right;

    x->right = y;
    y->left = t;

    update_height(y);
    update_height(x);

    return x;
}

// rotacao para a esquerda quando a arvore fica pesada para a direita.

static AVLNode *rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *t = y->left;

    y->left = x;
    x->right = t;

    update_height(x);
    update_height(y);

    return y;
}

// verifica o fator de balanceamento e aplica as rotacoes necessarias.
// aqui sao tratados os casos LL, RR, LR e RL.

static AVLNode *avl_balance(AVLNode *node) {
    if (!node)
        return NULL;

    update_height(node);

    int balance = balance_factor(node);

    if (balance > 1) {

        if (balance_factor(node->left) < 0)
            node->left = rotate_left(node->left);

        return rotate_right(node);
    }

    if (balance < -1) {

        if (balance_factor(node->right) > 0)
            node->right = rotate_right(node->right);

        return rotate_left(node);
    }

    return node;
}

// cria um no da AVL para um ponto que entrou na janela ativa.

static AVLNode *avl_create_node(Point *point) {
    AVLNode *node = malloc(sizeof(AVLNode));

    if (!node) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        exit(EXIT_FAILURE);
    }

    node->point = point;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;

    return node;
}

// insere um ponto na AVL e depois rebalanceia a arvore.

static AVLNode *avl_insert(AVLNode *root, Point *point) {
    if (!root)
        return avl_create_node(point);

    int cmp = compare_key(point, root->point);

    if (cmp < 0)
        root->left = avl_insert(root->left, point);

    else if (cmp > 0)
        root->right = avl_insert(root->right, point);

    return avl_balance(root);
}

// encontra o menor elemento da AVL, usado como sucessor na remocao.

static AVLNode *avl_min_node(AVLNode *root) {
    AVLNode *current = root;

    while (current && current->left)
        current = current->left;

    return current;
}

// remove um ponto da AVL e rebalanceia os nos afetados.
// usamos isso quando um ponto sai da janela da sweep line.

static AVLNode *avl_remove(AVLNode *root, Point *point) {
    if (!root)
        return NULL;

    int cmp = compare_key(point, root->point);

    if (cmp < 0) {
        root->left = avl_remove(root->left, point);
    }

    else if (cmp > 0) {
        root->right = avl_remove(root->right, point);
    }

    else {

        if (!root->left || !root->right) {

            AVLNode *child =
                root->left ? root->left : root->right;

            free(root);

            return child;
        }

        else {

            AVLNode *successor =
                avl_min_node(root->right);

            root->point = successor->point;

            root->right =
                avl_remove(
                    root->right,
                    successor->point
                );
        }
    }

    return avl_balance(root);
}

// vetor temporario para guardar os pontos encontrados em uma faixa de y.

typedef struct {
    Point **data;
    size_t size;
    size_t capacity;
} PointVector;


static void point_vector_init(PointVector *v) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

static void point_vector_push(PointVector *v, Point *p) {
    if (v->size == v->capacity) {

        size_t new_capacity =
            (v->capacity == 0) ? 16 : v->capacity * 2;

        Point **new_data =
            realloc(
                v->data,
                new_capacity * sizeof(Point *)
            );

        if (!new_data) {
            fprintf(stderr, "Erro: memoria insuficiente.\n");
            exit(EXIT_FAILURE);
        }

        v->data = new_data;
        v->capacity = new_capacity;
    }

    v->data[v->size++] = p;
}


static void point_vector_free(PointVector *v) {
    free(v->data);

    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

// procura o primeiro ponto com y maior ou igual ao limite informado.
// esta funcao ficou no codigo, mas a busca usada atualmente e avl_collect_range.

static AVLNode *avl_lower_bound_y(
    AVLNode *root,
    int64_t y
) {
    AVLNode *answer = NULL;

    while (root) {

        if (root->point->y >= y) {
            answer = root;
            root = root->left;
        }
        else {
            root = root->right;
        }
    }

    return answer;
}

// percorre somente as partes da AVL que podem conter pontos entre low e high.

static void avl_collect_range(
    AVLNode *root,
    int64_t low,
    int64_t high,
    PointVector *result
) {
    if (!root)
        return;

    int64_t y = root->point->y;

    if (y > low)
        avl_collect_range(
            root->left,
            low,
            high,
            result
        );

    if (y >= low && y <= high)
        point_vector_push(result, root->point);

    if (y < high)
        avl_collect_range(
            root->right,
            low,
            high,
            result
        );
}

// libera todos os nos da AVL.

static void avl_free(AVLNode *root) {
    if (!root)
        return;

    avl_free(root->left);
    avl_free(root->right);

    free(root);
}

// calcula a raiz inteira de d.
// sqrt fornece uma aproximacao inicial e os while corrigem o resultado.

static int64_t integer_sqrt(int64_t d) {
    int64_t r = (int64_t)sqrt((long double)d);

    while ((r + 1) > 0 &&
           (r + 1) <= d / (r + 1))
        r++;

    while (r > d / r)
        r--;

    return r;
}

// primeira fase: encontra a menor distancia ao quadrado.
// os pontos sao percorridos da esquerda para a direita e a AVL guarda os ativos.

static int64_t find_minimum_distance(Point *p, size_t n) {

    const int64_t INF = INT64_MAX;

    AVLNode *active = NULL;

    size_t left = 0;

    int64_t D = INF;

    // o primeiro ponto entra na janela antes de comecarmos a varredura.
    active = avl_insert(active, &p[0]);

    // percorremos os pontos da esquerda para a direita.
    // faz sweep line
    for (size_t right = 1; right < n; right++) {

        Point *current = &p[right];

        if (D == INF) {

            AVLNode *node =
                avl_min_node(active);

            if (node) {
                D = squared_distance(
                    current,
                    node->point
                );
            }
        }

        else {
            // removemos os pontos que ja estao longe demais no eixo x.
            // se dx^2 > D, a distancia ja seria maior que D mesmo com dy = 0.
            while (left < right) {

                int64_t dx =
                    current->x - p[left].x;

                if (dx * dx <= D)
                    break;

                active =
                    avl_remove(
                        active,
                        &p[left]
                    );

                left++;
            }

            // qualquer candidato precisa ter |dy| <= sqrt(D).
            // por isso consultamos somente essa faixa de y.
            int64_t limit = integer_sqrt(D);

            int64_t low =
                current->y - limit;

            int64_t high =
                current->y + limit;

            PointVector candidates;

            point_vector_init(&candidates);

            // pegamos na AVL somente os pontos dentro da faixa vertical.
            // depois calculamos a distancia exata desses candidatos.
            avl_collect_range(
                active,
                low,
                high,
                &candidates
            );

            for (size_t k = 0;
                 k < candidates.size;
                 k++) {

                int64_t d =
                    squared_distance(
                        current,
                        candidates.data[k]
                    );

                // encontramos uma distancia menor, entao ela passa a ser o novo limite.
                if (d < D)
                    D = d;

                if (D == 0) {
                    point_vector_free(&candidates);
                    avl_free(active);
                    return 0;
                }
            }

            point_vector_free(&candidates);
        }

        // D pode ter diminuido durante a busca dos candidatos.
        // por isso limpamos a janela novamente antes de inserir current.
        while (left < right) {

            int64_t dx =
                current->x - p[left].x;

            if (dx * dx <= D)
                break;

            active =
                avl_remove(
                    active,
                    &p[left]
                );

            left++;
        }

        active =
            avl_insert(
                active,
                current
            );
    }

    avl_free(active);

    return D;
}

// segunda fase: com D ja conhecido, encontra todos os pares com distancia exatamente D.

static void enumerate_minimum_pairs(
    Point *p,
    size_t n,
    int64_t D,
    PairVector *result
) {
    AVLNode *active = NULL;

    size_t left = 0;

    int64_t limit = integer_sqrt(D);

    for (size_t right = 0;
         right < n;
         right++) {

        Point *current = &p[right];

        while (left < right) {

            int64_t dx =
                current->x - p[left].x;

            if (dx * dx <= D)
                break;

            active =
                avl_remove(
                    active,
                    &p[left]
                );

            left++;
        }

        int64_t low =
            current->y - limit;

        int64_t high =
            current->y + limit;

        PointVector candidates;

        point_vector_init(&candidates);

        // como D ja e fixo, procuramos todos os pontos que ainda podem formar
        // um par de distancia D com current.
        avl_collect_range(
            active,
            low,
            high,
            &candidates
        );
        for (size_t k = 0;
             k < candidates.size;
             k++) {

            Point *other =
                candidates.data[k];

            int64_t d =
                squared_distance(
                    current,
                    other
                );

            // nesta fase queremos somente os pares que empatam com a distancia minima.
            if (d == D) {

                pair_vector_push(
                    result,
                    current->id,
                    other->id
                );
            }
        }

        point_vector_free(&candidates);
        active =
            avl_insert(
                active,
                current
            );
    }

    avl_free(active);
}

// CASO ESPECIFICO: distancia 0
// caso especial: se D = 0, existem grupos de pontos com a mesma coordenada.
// nesse caso podemos ter O(n^2) respostas, entao tratamos os grupos diretamente.

static void enumerate_zero_distance_pairs(
    Point *p,
    size_t n,
    PairVector *result
) {
    size_t begin = 0;

    while (begin < n) {

        size_t end = begin + 1;

        while (end < n &&
               p[end].x == p[begin].x &&
               p[end].y == p[begin].y) {

            end++;
        }
        // todos os pares dentro do grupo possuem distancia 0.
        for (size_t i = begin;
             i < end;
             i++) {

            for (size_t j = i + 1;
                 j < end;
                 j++) {

                pair_vector_push(
                    result,
                    p[i].id,
                    p[j].id
                );
            }
        }

        begin = end;
    }
}

// ordena os pares pelo primeiro id e depois pelo segundo.
// assim a saida fica em ordem lexicografica.

static int compare_pairs(
    const void *pa,
    const void *pb
) {
    const Pair *a = (const Pair *)pa;
    const Pair *b = (const Pair *)pb;

    if (a->a < b->a) return -1;
    if (a->a > b->a) return 1;

    if (a->b < b->b) return -1;
    if (a->b > b->b) return 1;

    return 0;
}

// le a entrada, ordena os pontos, executa as duas fases e imprime as respostas.

int main(void) {

    size_t n;

    if (scanf("%zu", &n) != 1)
        return EXIT_FAILURE;

    if (n < 2) {
        return EXIT_SUCCESS;
    }

    Point *points =
        malloc(n * sizeof(Point));

    if (!points) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < n; i++) {

        if (scanf(
                "%" SCNd64 " %" SCNd64,
                &points[i].x,
                &points[i].y
            ) != 2) {

            free(points);
            return EXIT_FAILURE;
        }

        points[i].id = (int)i;
    }

    qsort(
        points,
        n,
        sizeof(Point),
        compare_points
    );

    int64_t D =
        find_minimum_distance(
            points,
            n
        );

    PairVector result;

    pair_vector_init(&result);

    if (D == 0) {

        enumerate_zero_distance_pairs(
            points,
            n,
            &result
        );

    } else {

        enumerate_minimum_pairs(
            points,
            n,
            D,
            &result
        );
    }

    // os pares podem ter sido encontrados em uma ordem diferente da pedida.
    // por isso ordenamos as respostas antes de imprimir.
    qsort(
        result.data,
        result.size,
        sizeof(Pair),
        compare_pairs
    );
    for (size_t i = 0;
         i < result.size;
         i++) {

        printf(
            "(%d,%d)\n",
            result.data[i].a,
            result.data[i].b
        );
    }
    pair_vector_free(&result);
    free(points);

    return EXIT_SUCCESS;
}