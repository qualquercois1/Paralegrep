#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define N_THREADS 4
pthread_mutex_t lock;

typedef struct {
    pthread_t thread_id;
    char nomeArquivo[20];
    char termo[20];
    int palavras;
} Thread_operaria;

FILE *abrirArquivo(char *nomeArquivo) {
    
    FILE *arquivo = fopen(nomeArquivo, "r");
    if(!arquivo) {
        perror("Erro ao abrir arquivo\n");
        exit(1);
    } else {
        printf("Abriu!!\n");
    }
    return arquivo;
}

void* listarTexto(void* arg) {
    Thread_operaria *temp = (Thread_operaria *)arg;
    char nomeArq[1024];
    strcpy(nomeArq, temp->nomeArquivo);

    char *dir = "fileset/";
    char *nome = malloc(strlen(dir)+strlen(nomeArq)+1);
    strcpy(nome, dir); 
    strcat(nome, nomeArq);

    FILE *arquivo = abrirArquivo(nome);
    int i = 0;
    char linha[1024];

    pthread_mutex_lock(&lock);
    sleep(1);
    while(fgets(linha, sizeof(linha), arquivo)) {
        if(strstr(linha, temp->termo)) {
            i++;
        }
    }
    printf("%s: %d\n", nome,i);
    free(nome);
    fclose(arquivo);

    pthread_mutex_unlock(&lock);
    return NULL;
}


char** listarArquivos(const char *pasta, int *quantidade) {
    struct dirent *entrada;
    DIR *diretorio = opendir(pasta);
    if (diretorio == NULL) {
        perror("Erro ao abrir o diretório");
        return NULL;
    }

    char **matriz = NULL;
    *quantidade = 0;

    while ((entrada = readdir(diretorio)) != NULL) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        matriz = realloc(matriz, (*quantidade + 1) * sizeof(char *));
        if (matriz == NULL) {
            perror("Erro ao alocar memória");
            closedir(diretorio);
            return NULL;
        }

        matriz[*quantidade] = malloc(strlen(entrada->d_name) + 1);
        if (matriz[*quantidade] == NULL) {
            perror("Erro ao alocar memória para o nome do arquivo");
            closedir(diretorio);
            return NULL;
        }
        strcpy(matriz[*quantidade], entrada->d_name);
        (*quantidade)++;
    }

    closedir(diretorio);
    return matriz;
}

void liberarMatriz(char **matriz, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        free(matriz[i]);
    }
    free(matriz);
}
