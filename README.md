# Documentação - Implementação de Árvore Patricia (Trie Compactada Binária)

**Disciplina:** Estrutura de Dados 2

**Professor:** Samuel Zanferdini Oliva

**Alunos:**
*   Paloma Amaral - RA: 841931
*   Eduardo Lourenço - RA: 841932


## 1. Introdução

Este documento descreve a implementação de uma Árvore Patricia em linguagem C. O objetivo principal é fornecer uma estrutura de dados eficiente para o armazenamento e manipulação de um dicionário de palavras (strings).

A implementação segue o conceito de uma Trie Compactada Binária (também conhecida como Radix Tree Binária ou Crit-bit Tree), conforme discutido em aula, onde a compactação é alcançada através da eliminação de nós com apenas um filho e as decisões de navegação são baseadas em bits individuais das chaves (palavras).

## 2. Estrutura da Árvore Patricia Implementada

A estrutura de dados principal é definida por `ArvorePatricia`, que contém um ponteiro para o nó raiz (`raiz`) e contadores para estatísticas (`contagemNos`, `contagemPalavras`, `contagemDeletadas`).

```c
typedef struct {
    NoPatricia* raiz;       // Ponteiro para o primeiro no da arvore
    int contagemNos;      // Quantos nos existem no total
    int contagemPalavras; // Quantas palavras validas existem
    int contagemDeletadas;// Quantas palavras foram marcadas como removidas
} ArvorePatricia;
```

O nó da árvore, `NoPatricia`, pode ser de dois tipos:

*   **Nó Folha (`ehFolha = true`):** Armazena a palavra completa (`palavraGuardada`) e um indicador booleano (`foiDeletada`) para suportar remoção lógica.
*   **Nó Interno (`ehFolha = false`):** Não armazena palavras. Contém o índice do bit (`indiceDoBit`) que será usado para decidir o caminho (0 ou 1) e um array de dois ponteiros (`filhos[2]`) para os nós descendentes.

```c
typedef struct NoPatricia {
    bool ehFolha;           // Diz se este no eh uma folha (guarda palavra)

    // Se for folha:
    char* palavraGuardada; // A palavra completa fica aqui
    bool foiDeletada;     // Marca se a palavra foi "removida"

    // Se NAO for folha (no interno):
    int indiceDoBit;       // Qual bit da palavra usar para decidir o caminho (0 ou 1)
    struct NoPatricia* filhos[2];  // Ponteiros para os filhos (esquerda=0, direita=1)
} NoPatricia;
```

Essa estrutura binária baseada em bits permite uma forma de compactação, pois apenas os bits que diferenciam as palavras são utilizados para criar novos nós internos.

## 3. Funcionalidades Implementadas

O programa tem no menu interativo:

### 3.1. Inserção (`inserir`, `inserirRecursivo`)

A inserção percorre a árvore bit a bit, utilizando a função `pegarValorBit` para determinar o caminho (0 ou 1) em cada nó interno, com base no `indiceDoBit` do nó.

*   Se o caminho leva a um ponteiro `NULL`, um novo nó folha é criado com a palavra.
*   Se o caminho leva a um nó folha existente:
    *   Se a palavra for idêntica, verifica se estava marcada como deletada e a reativa, se necessário.
    *   Se a palavra for diferente, a função `acharPrimeiroBitDiferente` encontra o primeiro bit onde as palavras divergem. Um novo nó interno é criado nesse ponto de divergência, e a folha original e a nova folha (com a palavra a ser inserida) tornam-se filhas desse novo nó interno.
*   Se o caminho leva a um nó interno, a função continua recursivamente pelo filho apropriado (0 ou 1).

### 3.2. Busca (`buscar`, `buscarRecursivo`)

A busca opera de forma similar à inserção, navegando pela árvore com base nos bits da palavra buscada e nos `indiceDoBit` dos nós internos.

*   Se a busca chega a `NULL`, a palavra não existe.
*   Se a busca chega a um nó interno, continua recursivamente pelo filho correspondente ao bit da palavra.
*   Se a busca chega a um nó folha, compara a palavra buscada com a `palavraGuardada` na folha. Retorna `true` apenas se as palavras forem idênticas E o nó não estiver marcado como deletado (`foiDeletada == false`).

### 3.3. Remoção (`removerPalavra`, `removerRecursivo`)

A implementação utiliza **remoção lógica (lazy deletion)**.

A função de remoção busca a palavra na árvore de forma semelhante à função `buscar`.

*   Se a palavra é encontrada em um nó folha e não está marcada como deletada, a flag `foiDeletada` é setada para `true`.
*   Os contadores `contagemPalavras` e `contagemDeletadas` são atualizados.

### 3.4. Impressão em Ordem (`imprimirEmOrdem`, `imprimirOrdemRecursivo`)

A impressão utiliza uma travessia recursiva da árvore.

*   Para garantir a ordem em uma árvore binária baseada em bits, a recursão visita primeiro o filho esquerdo (correspondente ao bit 0) e depois o filho direito (correspondente ao bit 1).
*   Quando um nó folha é visitado, a `palavraGuardada` é impressa somente se a flag `foiDeletada` for `false`.

### 3.5. Exibição (`imprimirStats`, `calcularProfundidadeRecursivo`)

Esta função calcula e exibe:

*   Número total de nós na árvore (`contagemNos`).
*   Número de palavras ativas (não deletadas) (`contagemPalavras`).
*   Número de palavras marcadas como removidas (`contagemDeletadas`).
*   Profundidade máxima da árvore (altura).
*   Profundidade média das palavras ativas (soma das profundidades das folhas ativas / número de folhas ativas).
*   Uso estimado de memória (calculado de forma simplificada como `contagemNos * sizeof(NoPatricia)`).

### 3.6. Liberação de Memória (`liberarArvore`, `liberarNoRecursivo`)

Ao final da execução do programa (ou quando o usuário escolhe sair), toda a memória alocada dinamicamente para os nós e para as strings das palavras nas folhas é liberada através de uma travessia pós-ordem recursiva.
