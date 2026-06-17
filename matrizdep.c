/*
 * TG — Matrizes Esparsas | Grupo 2
 * Estrutura de Dados I | Prof. Rovilson
 *
 * Mapa de Dependências de Disciplinas com Lista Encadeada
 * Estrutura: Lista Encadeada de Nós Esparsos (ordenada por linha, coluna)
 *
 * Funcionalidades obrigatórias:
 *   criar_matriz, inserir_dependencia, remover_dependencia,
 *   tem_dependencia, listar_prerequisitos, listar_dependentes,
 *   transpor, imprimir_matriz, liberar_matriz
 *
 * Extras (surpreenda-me):
 *   - Detecção de ciclos por DFS (qualquer comprimento, não só diretos)
 *   - Ordenação Topológica (ordem válida de cursagem das disciplinas)
 *   - Nomes simbólicos para as disciplinas
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 *  ESTRUTURAS DE DADOS
 * ============================================================ */

typedef struct No {
    int linha;
    int coluna;
    int valor;          /* sempre 1 — dependência existe */
    struct No *prox;
} No;

typedef struct {
    int num_disciplinas;
    int total_nao_nulos;
    No *inicio;         /* lista encadeada ordenada por (linha, coluna) */
    char **nomes;       /* nomes simbólicos: nomes[i] = nome da disciplina i */
} MatrizDep;

/* ============================================================
 *  FUNÇÕES AUXILIARES
 * ============================================================ */

/* Exibe o nome ou o índice entre colchetes */
static void exibir_nome(const MatrizDep *m, int idx) {
    if (m->nomes && m->nomes[idx])
        printf("[%d] %s", idx, m->nomes[idx]);
    else
        printf("[%d]", idx);
}

/* ============================================================
 *  OPERAÇÕES OBRIGATÓRIAS
 * ============================================================ */

/*
 * criar_matriz(n)
 * Aloca a estrutura para n disciplinas.
 * Retorna ponteiro para MatrizDep ou NULL em falha.
 */
MatrizDep *criar_matriz(int n) {
    MatrizDep *m = (MatrizDep *)malloc(sizeof(MatrizDep));
    if (!m) { fprintf(stderr, "Erro: malloc falhou em criar_matriz\n"); return NULL; }

    m->num_disciplinas = n;
    m->total_nao_nulos = 0;
    m->inicio = NULL;

    /* aloca vetor de nomes e inicializa com NULL */
    m->nomes = (char **)calloc(n, sizeof(char *));
    if (!m->nomes) { fprintf(stderr, "Erro: calloc falhou para nomes\n"); free(m); return NULL; }

    return m;
}

/*
 * inserir_dependencia(matriz, origem, destino)
 * Insere nó ordenado por (linha=origem, coluna=destino).
 * Retorna 0 em sucesso, -1 se já existir ou inválido.
 */
int inserir_dependencia(MatrizDep *m, int origem, int destino) {
    if (!m) return -1;
    if (origem < 0 || origem >= m->num_disciplinas ||
        destino < 0 || destino >= m->num_disciplinas) {
        printf("Erro: indices fora do intervalo [0, %d).\n", m->num_disciplinas);
        return -1;
    }
    if (origem == destino) {
        printf("Erro: disciplina nao pode depender de si mesma.\n");
        return -1;
    }

    /* percorre buscando posicao de insercao e verificando duplicata */
    No *ant = NULL;
    No *cur = m->inicio;
    while (cur) {
        if (cur->linha == origem && cur->coluna == destino) {
            printf("Aviso: dependencia (%d -> %d) ja existe.\n", origem, destino);
            return -1;
        }
        /* comparacao lexicografica (linha, coluna) */
        if (cur->linha > origem || (cur->linha == origem && cur->coluna > destino))
            break;
        ant = cur;
        cur = cur->prox;
    }

    No *novo = (No *)malloc(sizeof(No));
    if (!novo) { fprintf(stderr, "Erro: malloc falhou em inserir_dependencia\n"); return -1; }
    novo->linha  = origem;
    novo->coluna = destino;
    novo->valor  = 1;
    novo->prox   = cur;

    if (ant) ant->prox = novo;
    else     m->inicio = novo;

    m->total_nao_nulos++;
    return 0;
}

/*
 * remover_dependencia(matriz, origem, destino)
 * Remove o nó correspondente e libera memória.
 * Retorna 0 em sucesso, -1 se não existir.
 */
int remover_dependencia(MatrizDep *m, int origem, int destino) {
    if (!m) return -1;

    No *ant = NULL;
    No *cur = m->inicio;
    while (cur) {
        if (cur->linha == origem && cur->coluna == destino) {
            if (ant) ant->prox = cur->prox;
            else     m->inicio = cur->prox;
            free(cur);
            m->total_nao_nulos--;
            return 0;
        }
        /* lista ordenada: se passamos do ponto, nao existe */
        if (cur->linha > origem || (cur->linha == origem && cur->coluna > destino))
            break;
        ant = cur;
        cur = cur->prox;
    }
    printf("Aviso: dependencia (%d -> %d) nao encontrada.\n", origem, destino);
    return -1;
}

/*
 * tem_dependencia(matriz, origem, destino)
 * Retorna 1 se existe, 0 se nao existe.
 */
int tem_dependencia(const MatrizDep *m, int origem, int destino) {
    if (!m) return 0;
    No *cur = m->inicio;
    while (cur) {
        if (cur->linha == origem && cur->coluna == destino) return 1;
        if (cur->linha > origem || (cur->linha == origem && cur->coluna > destino))
            break;
        cur = cur->prox;
    }
    return 0;
}

/*
 * listar_prerequisitos(matriz, disciplina)
 * Lista todas as disciplinas das quais esta depende (linha == disciplina).
 */
void listar_prerequisitos(const MatrizDep *m, int disciplina) {
    if (!m) return;
    printf("Pre-requisitos de ");
    exibir_nome(m, disciplina);
    printf(":\n");

    int count = 0;
    No *cur = m->inicio;
    while (cur) {
        if (cur->linha == disciplina) {
            printf("  -> ");
            exibir_nome(m, cur->coluna);
            printf("\n");
            count++;
        }
        if (cur->linha > disciplina) break;
        cur = cur->prox;
    }
    if (count == 0) printf("  (nenhum pre-requisito cadastrado)\n");
}

/*
 * listar_dependentes(matriz, disciplina)
 * Lista disciplinas que dependem desta (coluna == disciplina).
 */
void listar_dependentes(const MatrizDep *m, int disciplina) {
    if (!m) return;
    printf("Disciplinas que dependem de ");
    exibir_nome(m, disciplina);
    printf(":\n");

    int count = 0;
    No *cur = m->inicio;
    while (cur) {
        if (cur->coluna == disciplina) {
            printf("  -> ");
            exibir_nome(m, cur->linha);
            printf("\n");
            count++;
        }
        cur = cur->prox;
    }
    if (count == 0) printf("  (nenhuma disciplina depende desta)\n");
}

/*
 * transpor(matriz)
 * Retorna nova MatrizDep com linhas e colunas trocadas.
 * Caller e responsavel por liberar_matriz no resultado.
 */
MatrizDep *transpor(const MatrizDep *m) {
    if (!m) return NULL;
    MatrizDep *t = criar_matriz(m->num_disciplinas);
    if (!t) return NULL;

    /* copia nomes */
    for (int i = 0; i < m->num_disciplinas; i++) {
        if (m->nomes[i]) {
            t->nomes[i] = (char *)malloc(strlen(m->nomes[i]) + 1);
            if (t->nomes[i]) strcpy(t->nomes[i], m->nomes[i]);
        }
    }

    /* reinserir com linha e coluna trocados — insercao ordenada ja trata */
    No *cur = m->inicio;
    while (cur) {
        inserir_dependencia(t, cur->coluna, cur->linha);
        cur = cur->prox;
    }
    return t;
}

/*
 * imprimir_matriz(matriz)
 * Exibe a matriz completa n×n com 0s e 1s.
 */
void imprimir_matriz(const MatrizDep *m) {
    if (!m) return;
    int n = m->num_disciplinas;

    printf("\nMatriz de dependencias (%dx%d):\n", n, n);
    printf("     ");
    for (int j = 0; j < n; j++) printf("%3d", j);
    printf("\n     ");
    for (int j = 0; j < n; j++) printf("---");
    printf("\n");

    for (int i = 0; i < n; i++) {
        printf("%3d |", i);
        for (int j = 0; j < n; j++) {
            /* verifica se (i,j) existe na lista */
            int val = 0;
            /* busca linear na linha i */
            No *p = m->inicio;
            while (p && p->linha <= i) {
                if (p->linha == i && p->coluna == j) { val = 1; break; }
                p = p->prox;
            }
            printf("%3d", val);
        }
        printf("\n");
    }
    printf("\nTotal de dependencias cadastradas: %d\n", m->total_nao_nulos);
}

/*
 * liberar_matriz(matriz)
 * Libera todos os nos, nomes e a struct principal.
 */
void liberar_matriz(MatrizDep *m) {
    if (!m) return;

    No *cur = m->inicio;
    while (cur) {
        No *prox = cur->prox;
        free(cur);
        cur = prox;
    }

    if (m->nomes) {
        for (int i = 0; i < m->num_disciplinas; i++)
            free(m->nomes[i]); /* free(NULL) e' seguro */
        free(m->nomes);
    }

    free(m);
}

/*
 * pode_cursar(matriz, disciplina, cursadas, num_cursadas)
 * Verifica se todas as dependencias de 'disciplina' estao em cursadas[].
 * Retorna 1 se pode, 0 se nao pode.
 */
int pode_cursar(const MatrizDep *m, int disciplina,
                const int *cursadas, int num_cursadas) {
    if (!m) return 0;

    No *cur = m->inicio;
    while (cur) {
        if (cur->linha == disciplina) {
            int encontrado = 0;
            for (int k = 0; k < num_cursadas; k++) {
                if (cursadas[k] == cur->coluna) { encontrado = 1; break; }
            }
            if (!encontrado) {
                printf("  Pre-requisito nao satisfeito: ");
                exibir_nome(m, cur->coluna);
                printf("\n");
                return 0;
            }
        }
        if (cur->linha > disciplina) break;
        cur = cur->prox;
    }
    return 1;
}

/* ============================================================
 *  EXTRAS — DETECÇÃO DE CICLOS (DFS) E ORDENAÇÃO TOPOLÓGICA
 * ============================================================
 *
 * A verificação obrigatória do enunciado detecta apenas ciclos
 * DIRETOS (A->B e B->A). Esta implementação vai além: usa DFS
 * para detectar ciclos de QUALQUER comprimento (A->B->C->A)
 * e, se o grafo for acíclico, gera a ORDENAÇÃO TOPOLÓGICA —
 * uma sequência válida de cursagem de todas as disciplinas.
 *
 * Estados DFS: 0=nao visitado, 1=em pilha (cinza), 2=concluido
 */

/* Auxiliar DFS recursiva. Retorna 1 se encontrou ciclo. */
static int dfs_visita(const MatrizDep *m, int v,
                      int *estado, int *pilha_ciclo, int *topo_pilha,
                      int *ordem_topo, int *idx_topo) {
    estado[v] = 1; /* marca como "em pilha" */

    No *cur = m->inicio;
    while (cur) {
        if (cur->linha == v) {
            int viz = cur->coluna;
            if (estado[viz] == 1) {
                /* ciclo detectado: viz esta na pilha atual */
                if (pilha_ciclo) {
                    pilha_ciclo[(*topo_pilha)++] = viz;
                    pilha_ciclo[(*topo_pilha)++] = v;
                }
                return 1;
            }
            if (estado[viz] == 0) {
                if (dfs_visita(m, viz, estado, pilha_ciclo, topo_pilha,
                               ordem_topo, idx_topo))
                    return 1;
            }
        }
        if (cur->linha > v) break;
        cur = cur->prox;
    }

    estado[v] = 2; /* concluido */
    if (ordem_topo) ordem_topo[(*idx_topo)--] = v;
    return 0;
}

/*
 * detectar_ciclo(matriz)
 * Usa DFS completa. Retorna 1 se existe ciclo, 0 se nao existe.
 * Imprime o ciclo encontrado se houver.
 */
int detectar_ciclo(const MatrizDep *m) {
    if (!m) return 0;
    int n = m->num_disciplinas;

    int *estado    = (int *)calloc(n, sizeof(int)); /* 0,1,2 */
    int *ciclo_buf = (int *)malloc(2 * n * sizeof(int));
    int topo = 0;
    if (!estado || !ciclo_buf) {
        free(estado); free(ciclo_buf);
        return 0;
    }

    int encontrou = 0;
    for (int i = 0; i < n && !encontrou; i++) {
        if (estado[i] == 0)
            encontrou = dfs_visita(m, i, estado, ciclo_buf, &topo, NULL, NULL);
    }

    if (encontrou) {
        printf("CICLO DETECTADO! Caminho parcial envolvido: ");
        for (int k = 0; k < topo; k++) {
            exibir_nome(m, ciclo_buf[k]);
            if (k < topo - 1) printf(" -> ");
        }
        printf("\n");
    } else {
        printf("Nenhum ciclo detectado. O grafo e aciclico.\n");
    }

    free(estado);
    free(ciclo_buf);
    return encontrou;
}

/*
 * ordenacao_topologica(matriz)
 * So faz sentido se o grafo for aciclico.
 * Exibe uma ordem valida de cursagem de todas as disciplinas.
 */
void ordenacao_topologica(const MatrizDep *m) {
    if (!m) return;
    int n = m->num_disciplinas;

    int *estado     = (int *)calloc(n, sizeof(int));
    int *ordem      = (int *)malloc(n * sizeof(int));
    if (!estado || !ordem) { free(estado); free(ordem); return; }

    int idx = n - 1; /* preenchemos da direita para a esquerda */

    int ciclo = 0;
    for (int i = 0; i < n && !ciclo; i++) {
        if (estado[i] == 0)
            ciclo = dfs_visita(m, i, estado, NULL, NULL, ordem, &idx);
    }

    if (ciclo) {
        printf("Ordenacao topologica impossivel: grafo possui ciclo.\n");
    } else {
        printf("Ordem topologica de cursagem (da mais basica para a mais avancada):\n");
        for (int k = 0; k < n; k++) {
            printf("  %d. ", k + 1);
            exibir_nome(m, ordem[k]);
            printf("\n");
        }
    }

    free(estado);
    free(ordem);
}

/* ============================================================
 *  GERENCIAMENTO DE NOMES
 * ============================================================ */

int definir_nome(MatrizDep *m, int disciplina, const char *nome) {
    if (!m || disciplina < 0 || disciplina >= m->num_disciplinas) return -1;
    free(m->nomes[disciplina]);
    m->nomes[disciplina] = (char *)malloc(strlen(nome) + 1);
    if (!m->nomes[disciplina]) return -1;
    strcpy(m->nomes[disciplina], nome);
    return 0;
}

/* ============================================================
 *  MENU INTERATIVO
 * ============================================================ */

static void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void exibir_menu(void) {
    printf("\n========================================\n");
    printf("  SISTEMA DE DEPENDENCIAS DE DISCIPLINAS\n");
    printf("========================================\n");
    printf(" 1. Inserir dependencia\n");
    printf(" 2. Remover dependencia\n");
    printf(" 3. Verificar dependencia\n");
    printf(" 4. Listar pre-requisitos de uma disciplina\n");
    printf(" 5. Listar dependentes de uma disciplina\n");
    printf(" 6. Transpor matriz\n");
    printf(" 7. Imprimir matriz\n");
    printf(" 8. Verificar se disciplina pode ser cursada\n");
    printf(" 9. Definir nome de disciplina\n");
    printf("10. Listar todos os nomes\n");
    printf("--- EXTRAS (DFS / Topologica) -----------\n");
    printf("11. Detectar ciclos (DFS completa)\n");
    printf("12. Ordenacao topologica\n");
    printf("-----------------------------------------\n");
    printf(" 0. Sair\n");
    printf("========================================\n");
    printf("Opcao: ");
}

int main(void) {
    int n;
    printf("Quantas disciplinas? ");
    scanf("%d", &n);
    limpar_buffer();

    if (n <= 0 || n > 100) {
        printf("Numero invalido de disciplinas.\n");
        return 1;
    }

    MatrizDep *m = criar_matriz(n);
    if (!m) return 1;

    /* nomes padrao */
    for (int i = 0; i < n; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Disc%d", i);
        definir_nome(m, i, buf);
    }

    int opcao;
    while (1) {
        exibir_menu();
        if (scanf("%d", &opcao) != 1) { limpar_buffer(); continue; }
        limpar_buffer();

        if (opcao == 0) break;

        int a, b;
        switch (opcao) {

        case 1:
            printf("Origem (0-%d): ", n - 1); scanf("%d", &a); limpar_buffer();
            printf("Destino (0-%d): ", n - 1); scanf("%d", &b); limpar_buffer();
            if (inserir_dependencia(m, a, b) == 0)
                printf("Dependencia (%d -> %d) inserida.\n", a, b);
            break;

        case 2:
            printf("Origem (0-%d): ", n - 1); scanf("%d", &a); limpar_buffer();
            printf("Destino (0-%d): ", n - 1); scanf("%d", &b); limpar_buffer();
            if (remover_dependencia(m, a, b) == 0)
                printf("Dependencia (%d -> %d) removida.\n", a, b);
            break;

        case 3:
            printf("Origem (0-%d): ", n - 1); scanf("%d", &a); limpar_buffer();
            printf("Destino (0-%d): ", n - 1); scanf("%d", &b); limpar_buffer();
            printf("Dependencia (%d -> %d): %s\n", a, b,
                   tem_dependencia(m, a, b) ? "EXISTE" : "NAO EXISTE");
            break;

        case 4:
            printf("Disciplina (0-%d): ", n - 1); scanf("%d", &a); limpar_buffer();
            listar_prerequisitos(m, a);
            break;

        case 5:
            printf("Disciplina (0-%d): ", n - 1); scanf("%d", &a); limpar_buffer();
            listar_dependentes(m, a);
            break;

        case 6: {
            MatrizDep *t = transpor(m);
            if (t) {
                printf("Matriz transposta:\n");
                imprimir_matriz(t);
                liberar_matriz(t);
            }
            break;
        }

        case 7:
            imprimir_matriz(m);
            break;

        case 8: {
            printf("Disciplina a verificar (0-%d): ", n - 1);
            scanf("%d", &a); limpar_buffer();
            printf("Quantas disciplinas ja foram cursadas? ");
            int nc; scanf("%d", &nc); limpar_buffer();
            int *cursadas = (int *)malloc(nc * sizeof(int));
            if (!cursadas) { printf("Erro de memoria.\n"); break; }
            for (int k = 0; k < nc; k++) {
                printf("  Cursada %d (0-%d): ", k + 1, n - 1);
                scanf("%d", &cursadas[k]);
            }
            limpar_buffer();
            if (pode_cursar(m, a, cursadas, nc))
                printf("Disciplina %d PODE ser cursada.\n", a);
            else
                printf("Disciplina %d NAO pode ser cursada ainda.\n", a);
            free(cursadas);
            break;
        }

        case 9: {
            printf("Disciplina (0-%d): ", n - 1); scanf("%d", &a); limpar_buffer();
            printf("Nome: ");
            char nome[128];
            fgets(nome, sizeof(nome), stdin);
            nome[strcspn(nome, "\n")] = '\0';
            if (definir_nome(m, a, nome) == 0)
                printf("Nome definido.\n");
            break;
        }

        case 10:
            printf("Nomes das disciplinas:\n");
            for (int i = 0; i < n; i++) {
                printf("  ");
                exibir_nome(m, i);
                printf("\n");
            }
            break;

        case 11:
            detectar_ciclo(m);
            break;

        case 12:
            ordenacao_topologica(m);
            break;

        default:
            printf("Opcao invalida.\n");
        }
    }

    liberar_matriz(m);
    printf("Memoria liberada. Ate logo!\n");
    return 0;
}