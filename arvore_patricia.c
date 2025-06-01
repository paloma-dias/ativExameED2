// Bibliotecas basicas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // Para usar true/false

// Define o tamanho do "alfabeto" (ASCII completo)
#define TAMANHO_ALFABETO 256

// --- Estrutura do No  da Arvore ---
// Esta arvore eh binaria, baseada em bits
typedef struct NoPatricia {
    bool ehFolha;           // Diz se este no eh uma folha (guarda palavra)

    // Se for folha:
    char* palavraGuardada; // A palavra completa fica aqui
    bool foiDeletada;     // Marca se a palavra foi "removida"

    // Se NAO for folha (no interno):
    int indiceDoBit;       // Qual bit da palavra usar para decidir o caminho (0 ou 1)
    struct NoPatricia* filhos[2];  // Ponteiros para os filhos (esquerda=0, direita=1)
} NoPatricia;

// --- Estrutura Principal da Arvore ---
typedef struct {
    NoPatricia* raiz;       // Ponteiro para o primeiro no da arvore
    int contagemNos;      // Quantos nos existem no total
    int contagemPalavras; // Quantas palavras validas existem
    int contagemDeletadas;// Quantas palavras foram marcadas como removidas
} ArvorePatricia;

// --- Funcoes Auxiliares ---

// Pega o valor (0 ou 1) de um bit especifico dentro de uma palavra
int pegarValorBit(const char* aPalavra, int indiceBit) {
    int indiceByte = indiceBit / 8; // Acha em qual byte (letra) esta o bit
    int offsetNoByte = 7 - (indiceBit % 8); // Acha a posicao do bit dentro do byte

    // Se o indice do bit for maior que a palavra, retorna 0 (convencao)
    if (indiceByte >= strlen(aPalavra)) {
        return 0;
    }

    // Pega o valor do bit (0 ou 1) usando operacoes bitwise
    return (aPalavra[indiceByte] >> offsetNoByte) & 1;
}

// Acha o indice do primeiro bit que eh diferente entre duas palavras (strings)
int acharPrimeiroBitDiferente(const char* string1, const char* string2) {
    int indiceAtualBit = 0;
    int tamanho1 = strlen(string1);
    int tamanho2 = strlen(string2);
    int menorTamanho = (tamanho1 < tamanho2) ? tamanho1 : tamanho2;

    // Compara byte por byte (letra por letra)
    for (int i = 0; i < menorTamanho; i++) {
        if (string1[i] != string2[i]) {
            // Achou um byte diferente, agora acha qual bit dentro dele eh diferente
            unsigned char diferenca = string1[i] ^ string2[i]; // XOR mostra os bits diferentes
            // Procura o bit diferente mais a esquerda (mais significativo)
            for (int j = 7; j >= 0; j--) {
                if ((diferenca >> j) & 1) {
                    return (i * 8) + (7 - j); // Retorna o indice global do bit
                }
            }
        }
        indiceAtualBit += 8; // Avanca 8 bits (1 byte)
    }

    // Se chegou aqui, uma palavra eh prefixo da outra.
    // A diferenca comeca no primeiro bit depois da parte comum.
    return menorTamanho * 8;
}

// Cria um novo no do tipo folha (para guardar uma palavra)
NoPatricia* criarNoFolha(const char* aPalavra) {
    // Aloca memoria para o no
    NoPatricia* novoNo = (NoPatricia*)malloc(sizeof(NoPatricia));
    if (novoNo == NULL) {
        fprintf(stderr, "Erro: Falha ao alocar memoria para no folha\n");
        exit(EXIT_FAILURE); // Aborta o programa em caso de erro grave de memoria
    }

    novoNo->ehFolha = true; // Marca como folha
    novoNo->palavraGuardada = strdup(aPalavra); // Copia a palavra para dentro do no
    novoNo->foiDeletada = false; // Comeca como nao deletada

    return novoNo;
}

// Cria um novo no interno (que nao guarda palavra, so decide caminho)
NoPatricia* criarNoInterno(int indiceBit, NoPatricia* filhoEsquerda, NoPatricia* filhoDireita) {
    // Aloca memoria
    NoPatricia* novoNo = (NoPatricia*)malloc(sizeof(NoPatricia));
    if (novoNo == NULL) {
        fprintf(stderr, "Erro: Falha ao alocar memoria para no interno\n");
        exit(EXIT_FAILURE);
    }

    novoNo->ehFolha = false; // Marca como interno
    novoNo->indiceDoBit = indiceBit; // Guarda o indice do bit que ele vai testar
    novoNo->filhos[0] = filhoEsquerda; // Define o filho da esquerda (bit 0)
    novoNo->filhos[1] = filhoDireita; // Define o filho da direita (bit 1)

    return novoNo;
}

// Inicializa uma arvore Patricia (cria a estrutura principal vazia)
ArvorePatricia inicializarArvore() {
    ArvorePatricia novaArvore;
    novaArvore.raiz = NULL; // Comeca sem raiz
    novaArvore.contagemNos = 0;
    novaArvore.contagemPalavras = 0;
    novaArvore.contagemDeletadas = 0;
    return novaArvore;
}

// --- Funcao de Insercao (Recursiva) ---

// Funcao auxiliar que faz a insercao de verdade, descendo na arvore
NoPatricia* inserirRecursivo(NoPatricia* noAtual, const char* aPalavra, ArvorePatricia* aArvore) {
    // Caso 1: Chegou a um ponto vazio (NULL), cria um no folha aqui
    if (noAtual == NULL) {
        aArvore->contagemNos++;
        aArvore->contagemPalavras++;
        return criarNoFolha(aPalavra);
    }

    // Caso 2: O no atual eh uma folha
    if (noAtual->ehFolha) {
        // Verifica se a palavra que queremos inserir eh a mesma que ja esta na folha
        if (strcmp(aPalavra, noAtual->palavraGuardada) == 0) {
            // Se for a mesma e estava marcada como deletada, reativa ela
            if (noAtual->foiDeletada) {
                noAtual->foiDeletada = false;
                aArvore->contagemPalavras++;
                aArvore->contagemDeletadas--;
            }
            return noAtual; // Nao precisa fazer mais nada
        }

        // Se a palavra eh diferente, precisamos transformar esta folha em um no interno
        // Acha o primeiro bit onde as duas palavras (a nova e a da folha) diferem
        int bitDiferente = acharPrimeiroBitDiferente(aPalavra, noAtual->palavraGuardada);

        // Cria um novo no folha para a palavra que estamos inserindo
        NoPatricia* novaFolha = criarNoFolha(aPalavra);
        aArvore->contagemNos++;
        aArvore->contagemPalavras++;

        // Cria um novo no interno que vai apontar para as duas folhas (a antiga e a nova)
        // O indice do bit desse no interno eh o bit onde elas diferem
        int valorBitNova = pegarValorBit(aPalavra, bitDiferente);
        NoPatricia* novoNoInterno;

        // Decide quem vai para a esquerda (0) e quem vai para a direita (1)
        if (valorBitNova == 0) {
            novoNoInterno = criarNoInterno(bitDiferente, novaFolha, noAtual);
        } else {
            novoNoInterno = criarNoInterno(bitDiferente, noAtual, novaFolha);
        }

        aArvore->contagemNos++; // Contabiliza o novo no interno
        return novoNoInterno; // Retorna o novo no interno que substituiu a folha original
    }

    // Caso 3: O no atual eh interno
    // Pega o valor do bit da palavra no indice que este no testa
    int valorBit = pegarValorBit(aPalavra, noAtual->indiceDoBit);

    // Chama a funcao recursivamente para o filho correspondente (0 ou 1)
    noAtual->filhos[valorBit] = inserirRecursivo(noAtual->filhos[valorBit], aPalavra, aArvore);

    return noAtual; // Retorna o no atual (pode ter tido filhos modificados)
}

// Funcao principal para inserir uma palavra na arvore
void inserir(ArvorePatricia* aArvore, const char* aPalavra) {
    // Chama a funcao recursiva comecando pela raiz
    aArvore->raiz = inserirRecursivo(aArvore->raiz, aPalavra, aArvore);
}

// --- Funcao de Busca (Recursiva) ---

// Funcao auxiliar que faz a busca de verdade, descendo na arvore
bool buscarRecursivo(NoPatricia* noAtual, const char* aPalavra) {
    // Caso 1: Chegou a um ponto vazio, palavra nao encontrada
    if (noAtual == NULL) {
        return false;
    }

    // Caso 2: Chegou a uma folha
    if (noAtual->ehFolha) {
        // Verifica se a palavra na folha eh a mesma que buscamos E se nao esta deletada
        return (strcmp(aPalavra, noAtual->palavraGuardada) == 0 && !noAtual->foiDeletada);
    }

    // Caso 3: No interno
    // Pega o bit da palavra para decidir o caminho
    int valorBit = pegarValorBit(aPalavra, noAtual->indiceDoBit);

    // Continua a busca no filho correspondente
    return buscarRecursivo(noAtual->filhos[valorBit], aPalavra);
}

// Funcao principal para buscar uma palavra na arvore
bool buscar(ArvorePatricia* aArvore, const char* aPalavra) {
    return buscarRecursivo(aArvore->raiz, aPalavra);
}

// --- Funcao de Remocao (Logica - Apenas Marca) ---

// Funcao auxiliar recursiva que encontra a palavra e marca como deletada
bool removerRecursivo(NoPatricia* noAtual, const char* aPalavra, ArvorePatricia* aArvore) {
    // Caso 1: Chegou a um ponto vazio, palavra nao encontrada
    if (noAtual == NULL) {
        return false;
    }

    // Caso 2: Chegou a uma folha
    if (noAtual->ehFolha) {
        // Verifica se eh a palavra correta E se ja nao estava deletada
        if (strcmp(aPalavra, noAtual->palavraGuardada) == 0 && !noAtual->foiDeletada) {
            noAtual->foiDeletada = true; // Marca como deletada
            aArvore->contagemPalavras--; // Atualiza contadores
            aArvore->contagemDeletadas++;
            return true; // Sucesso
        }
        return false; // Nao era a palavra ou ja estava deletada
    }

    // Caso 3: No interno
    // Pega o bit e decide o caminho
    int valorBit = pegarValorBit(aPalavra, noAtual->indiceDoBit);

    // Continua a busca no filho apropriado
    return removerRecursivo(noAtual->filhos[valorBit], aPalavra, aArvore);
}

// Funcao principal para "remover" (marcar) uma palavra
bool removerPalavra(ArvorePatricia* aArvore, const char* aPalavra) {
    return removerRecursivo(aArvore->raiz, aPalavra, aArvore);
}

// --- Funcao de Impressao em Ordem ---

// Funcao auxiliar recursiva para imprimir as palavras
void imprimirOrdemRecursivo(NoPatricia* noAtual) {
    if (noAtual == NULL) {
        return;
    }

    // Se for folha, imprime a palavra (se nao estiver deletada)
    if (noAtual->ehFolha) {
        if (!noAtual->foiDeletada) {
            printf("%s\n", noAtual->palavraGuardada);
        }
        return;
    }

    // Se for no interno, visita primeiro o filho da esquerda (0), depois o da direita (1)
    // Isso garante a ordem alfabetica (lexicografica) por causa da natureza binaria/bit a bit
    imprimirOrdemRecursivo(noAtual->filhos[0]);
    imprimirOrdemRecursivo(noAtual->filhos[1]);
}

// Funcao principal para imprimir todas as palavras validas em ordem
void imprimirEmOrdem(ArvorePatricia* aArvore) {
    if (aArvore->raiz == NULL) {
        printf("Arvore esta vazia.\n");
        return;
    }

    printf("Palavras em ordem alfabetica:\n");
    imprimirOrdemRecursivo(aArvore->raiz);
}

// --- Funcoes de Estatisticas ---

// Funcao auxiliar recursiva para calcular profundidade (nivel) das palavras
void calcularProfundidadeRecursivo(NoPatricia* noAtual, int nivel, int* somaTotalNiveis, int* contagemFolhasValidas, int* nivelMaximo) {
    if (noAtual == NULL) {
        return;
    }

    // Se for folha valida (nao deletada)
    if (noAtual->ehFolha) {
        if (!noAtual->foiDeletada) {
            *somaTotalNiveis += nivel; // Acumula o nivel
            (*contagemFolhasValidas)++; // Conta mais uma folha
            if (nivel > *nivelMaximo) { // Atualiza o nivel maximo se necessario
                *nivelMaximo = nivel;
            }
        }
        return;
    }

    // Se for no interno, continua nos filhos (aumentando o nivel)
    calcularProfundidadeRecursivo(noAtual->filhos[0], nivel + 1, somaTotalNiveis, contagemFolhasValidas, nivelMaximo);
    calcularProfundidadeRecursivo(noAtual->filhos[1], nivel + 1, somaTotalNiveis, contagemFolhasValidas, nivelMaximo);
}

// Funcao para mostrar as estatisticas da arvore
void imprimirStats(ArvorePatricia* aArvore) {
    printf("\n=== Estatisticas da Arvore Patricia ===\n");
    printf("Numero total de nos: %d\n", aArvore->contagemNos);
    printf("Numero de palavras ativas: %d\n", aArvore->contagemPalavras);
    printf("Numero de palavras removidas (marcadas): %d\n", aArvore->contagemDeletadas);

    if (aArvore->raiz == NULL) {
        printf("Arvore vazia.\n");
        return;
    }

    // Calcula profundidade media e maxima
    int somaNiveis = 0;
    int numFolhas = 0;
    int maxNivel = 0;
    calcularProfundidadeRecursivo(aArvore->raiz, 0, &somaNiveis, &numFolhas, &maxNivel);

    if (numFolhas > 0) {
        float mediaNivel = (float)somaNiveis / numFolhas;
        printf("Profundidade media das palavras: %.2f\n", mediaNivel);
        printf("Profundidade maxima (altura): %d\n", maxNivel);
    }

    // Estimativa simples de memoria (pode ser melhorada)
    size_t usoMemoria = aArvore->contagemNos * sizeof(NoPatricia);
    printf("Uso estimado de memoria (so nos): %lu bytes\n", (unsigned long)usoMemoria);
}

// --- Funcoes para Liberar Memoria ---

// Funcao auxiliar recursiva para liberar a memoria de cada no
void liberarNoRecursivo(NoPatricia* noAtual) {
    if (noAtual == NULL) {
        return;
    }

    if (noAtual->ehFolha) {
        free(noAtual->palavraGuardada); // Libera a string da palavra na folha
    } else {
        // Libera os filhos primeiro
        liberarNoRecursivo(noAtual->filhos[0]);
        liberarNoRecursivo(noAtual->filhos[1]);
    }

    // Libera o proprio no
    free(noAtual);
}

// Funcao principal para liberar toda a memoria da arvore
void liberarArvore(ArvorePatricia* aArvore) {
    liberarNoRecursivo(aArvore->raiz);
    // Reseta os dados da arvore
    aArvore->raiz = NULL;
    aArvore->contagemNos = 0;
    aArvore->contagemPalavras = 0;
    aArvore->contagemDeletadas = 0;
}

// --- Funcao para Carregar de Arquivo ---

// Carrega palavras de um arquivo texto para a arvore
int carregarDoArquivo(ArvorePatricia* aArvore, const char* nomeArquivo) {
    FILE* filePointer = fopen(nomeArquivo, "r"); // Tenta abrir o arquivo
    if (filePointer == NULL) {
        printf("Erro ao abrir o arquivo %s\n", nomeArquivo);
        return 0;
    }

    char palavraLida[256]; // Buffer para ler cada palavra
    int contador = 0;

    // Le palavra por palavra do arquivo
    while (fscanf(filePointer, "%255s", palavraLida) == 1) {
        inserir(aArvore, palavraLida); // Insere na arvore
        contador++;
    }

    fclose(filePointer); // Fecha o arquivo
    return contador; // Retorna quantas palavras foram lidas
}

// --- Menu e Funcao Principal (main) ---

// Mostra as opcoes do menu
void mostrarMenu() {
    printf("\n=== Menu Arvore Patricia (Binaria/Bit a Bit) ===\n");
    printf("1. Inserir palavra\n");
    printf("2. Buscar palavra\n");
    printf("3. Remover palavra (marcar)\n");
    printf("4. Imprimir palavras (ordem alfabetica)\n");
    printf("5. Mostrar estatisticas\n");
    printf("6. Carregar palavras de arquivo .txt\n");
    printf("0. Sair\n");
    printf("Escolha uma opcao: ");
}

// Funcao principal do programa
int main() {
    ArvorePatricia minhaArvore = inicializarArvore(); // Cria a arvore vazia
    int opcaoUser;
    char inputPalavra[256];
    char inputArquivo[256];

    printf("Bem-vindo ao programa da Arvore Patricia (Binaria)!\n");

    // Loop do menu ate o usuario escolher 0 para sair
    do {
        mostrarMenu();
        scanf("%d", &opcaoUser);
        getchar(); // Limpa o Enter que sobrou do scanf

        switch (opcaoUser) {
            case 1: // Inserir
                printf("Digite a palavra para inserir: ");
                scanf("%255s", inputPalavra);
                inserir(&minhaArvore, inputPalavra);
                printf("Palavra 	'%s' inserida!\n", inputPalavra);
                break;

            case 2: // Buscar
                printf("Digite a palavra para buscar: ");
                scanf("%255s", inputPalavra);
                if (buscar(&minhaArvore, inputPalavra)) {
                    printf("Palavra 	'%s' ENCONTRADA.\n", inputPalavra);
                } else {
                    printf("Palavra 	'%s' NAO encontrada.\n", inputPalavra);
                }
                break;

            case 3: // Remover (Marcar)
                printf("Digite a palavra para remover: ");
                scanf("%255s", inputPalavra);
                if (removerPalavra(&minhaArvore, inputPalavra)) {
                    printf("Palavra 	'%s' marcada como removida.\n", inputPalavra);
                } else {
                    printf("Palavra 	'%s' nao encontrada ou ja removida.\n", inputPalavra);
                }
                break;

            case 4: // Imprimir
                imprimirEmOrdem(&minhaArvore);
                break;

            case 5: // Estatisticas
                imprimirStats(&minhaArvore);
                break;

            case 6: // Carregar Arquivo
                printf("Digite o nome do arquivo .txt: ");
                scanf("%255s", inputArquivo);
                int numCarregadas = carregarDoArquivo(&minhaArvore, inputArquivo);
                printf("%d palavras carregadas do arquivo '%s'.\n", numCarregadas, inputArquivo);
                break;

            case 0: // Sair
                printf("Encerrando...\n");
                break;

            default:
                printf("Opcao invalida! Tente de novo.\n");
        }
    } while (opcaoUser != 0);

    // Libera toda a memoria usada pela arvore antes de terminar
    liberarArvore(&minhaArvore);
    printf("Memoria liberada. Programa finalizado.\n");

    return 0; // Sucesso
}

