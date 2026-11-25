/* 
João Gabriel Almeida de Lucena - RA: 281255
João Murilo Coutinho Martins - RA 249798 
*/

/*
 * Estratégia usada (algoritmo guloso):
 *   Ordenamos as cartas com um critério de comparação entre pares:
 *   a carta "a" deve vir antes da carta "b" se a concatenação "ab"
 *   formar um número maior que "ba". Depois de ordenar, basta imprimir
 *   as cartas em sequência.
 *
 * Por que funciona:
 *   - Se em um arranjo existirem duas cartas vizinhas x, y com yx > xy,
 *     trocá-las aumenta o número (o resto não muda e o trecho tem o mesmo
 *     tamanho). Logo, no arranjo ótimo nenhum par vizinho está "invertido",
 *     ou seja, ele está ordenado por esse critério (argumento de troca)
 *   - O critério é transitivo: ab > ba equivale a
 *     a / (10^|a| - 1) > b / (10^|b| - 1), isto é, comparar números reais
 *     fixos associados a cada carta. Por isso a ordenação é bem definida.
 *
 * Complexidade: O(n log n) comparações, cada uma em O(d), onde d <= 9
 * é a quantidade de dígitos de uma carta.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CARTAS 100
#define TAM_CARTA 10  /* maior carta: 100000000 (9 dígitos) + '\0' */
#define TAM_CONCAT 20 /* duas cartas: 18 dígitos + '\0' (arredondamos para 20 bytes) */

/*
 * Comparador usado pelo qsort
 */
int comparar_cartas(const void *p, const void *q) {
    const char *a = (const char *)p;
    const char *b = (const char *)q;
    char ab[TAM_CONCAT];
    char ba[TAM_CONCAT];

    /* monta as duas concatenações possíveis */
    snprintf(ab, TAM_CONCAT, "%s%s", a, b);
    snprintf(ba, TAM_CONCAT, "%s%s", b, a);

    return strcmp(ba, ab);
}

int main(void) {
    int n;
    char cartas[MAX_CARTAS][TAM_CARTA];

    if (scanf("%d", &n) != 1) {
        return 1;
    }

    /* lê cada carta como string, limitando a 9 caracteres
       para não estourar o espaço de cada linha */
    for (int i = 0; i < n; i++) {
        if (scanf("%9s", cartas[i]) != 1) {
            return 1;
        }
    }

    /* ordena as cartas pelo critério guloso (ab > ba) */
    qsort(cartas, n, TAM_CARTA, comparar_cartas);

    /* imprime as cartas na ordem obtida, sem separadores */
    for (int i = 0; i < n; i++) {
        printf("%s", cartas[i]);
    }
    printf("\n");

    return 0;
}