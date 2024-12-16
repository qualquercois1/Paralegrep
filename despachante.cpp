#include <iostream>
#include <pthread.h>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <filesystem>

#include "operarias.h"

#define N_Operarias 4

using namespace std;
namespace fs = std::filesystem;

pthread_mutex_t mutex_filas = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    string nome_arq;
    filesystem::file_time_type data_ultima_alteracao;
}Arquivo;

class Thread_despachante {
private:
    pthread_t                   thread_id;
    queue<Arquivo>              fila_arquivos;
    queue<Thread_operaria>      fila_threads;
    string                      termo;
    vector<pair<string, int>>   resultados;


public:
    Thread_despachante() {
        thread_id = pthread_self();
    }

    void criaArquivos() {
        const string diretorio = "fileset";
        DIR* dir = opendir(diretorio.c_str());
        if (!dir) {
            cerr << "Erro ao abrir o diretório: " << diretorio << endl;
            return;
        }

        struct dirent* entrada;
        while ((entrada = readdir(dir)) != nullptr) {
            if (strcmp(entrada->d_name, ".") != 0 && strcmp(entrada->d_name, "..") != 0) {
                string nome_arquivo = entrada->d_name; 
                string caminho_completo = diretorio + "/" + nome_arquivo;


                Arquivo temp;
                temp.nome_arq = nome_arquivo;
                temp.data_ultima_alteracao = fs::last_write_time(caminho_completo);
                fila_arquivos.push(temp);
            }
        }

        closedir(dir);
    }

    void criaOperarias() {
        pthread_mutex_lock(&mutex_filas);
        for (int i = 0; i < N_Operarias; i++) {
            fila_threads.push(Thread_operaria());
        }
        pthread_mutex_unlock(&mutex_filas);
    }

    void atualizaArquivos() {
        //limpa a fila
        while(!fila_arquivos.empty()) {
            fila_arquivos.pop();
        }

        const string diretorio = "fileset";
        DIR* dir = opendir(diretorio.c_str());
        if (!dir) {
            cerr << "Erro ao abrir o diretório: " << diretorio << endl;
            return;
        }

        struct dirent* entrada;
        while ((entrada = readdir(dir)) != nullptr) {
            if (strcmp(entrada->d_name, ".") != 0 && strcmp(entrada->d_name, "..") != 0) {
                string nome_arquivo = entrada->d_name; 
                string caminho_completo = diretorio + "/" + nome_arquivo;


                Arquivo temp;
                temp.nome_arq = nome_arquivo;
                temp.data_ultima_alteracao = fs::last_write_time(caminho_completo);
                fila_arquivos.push(temp);
            }
        }

        closedir(dir);
    }


    void listaArquivos(queue<Arquivo> &fila_original) {
        auto copia_a = fila_arquivos;
        auto copia_b = fila_original;
        int tam_a = fila_arquivos.size();
        int tam_b = fila_original.size();
        if(tam_a > tam_b || tam_a < tam_b) {
            //verifica se foi adicionado ou excluido um arquivo
            fila_original = fila_arquivos;
        } else {
            //verifica se algum arquivo foi alterado pela data de modificação
            while(!fila_arquivos.empty()) {
                if(fila_arquivos.front().data_ultima_alteracao != fila_original.front().data_ultima_alteracao) {
                    fila_arquivos = copia_a;
                    fila_original = fila_arquivos;
                    return;
                }
                fila_arquivos.pop();
                fila_original.pop();
            }
            fila_arquivos = copia_a;
            fila_original = copia_b;
        }
    }

    bool verificaResultado(string str) {
        for(auto a : resultados) {
            if(str == a.first) return true;
        }
        return false;
    }

    void inserirResultado(string str, int n) {
        for(auto &a : resultados) {
            if(str == a.first) {
                a.second = n;
            }
        }
    }

    void executaOperaria() {

        auto copia = fila_arquivos;
        while (!fila_arquivos.empty()) {

            pthread_mutex_lock(&mutex_filas);
            Arquivo arquivo = fila_arquivos.front();
            fila_arquivos.pop();
            pthread_mutex_unlock(&mutex_filas);

            Thread_operaria operaria = fila_threads.front();
            fila_threads.pop();

            int n = operaria.executar(arquivo.nome_arq, termo);

            if(verificaResultado(arquivo.nome_arq)) inserirResultado(arquivo.nome_arq, n);
            else resultados.push_back(make_pair(arquivo.nome_arq,n));

            pthread_mutex_lock(&mutex_filas);
            fila_threads.push(operaria);
            pthread_mutex_unlock(&mutex_filas);
            
        }
        pthread_mutex_unlock(&mutex_filas);
        usleep(100000);
        fila_arquivos = copia;
    }

    void administra(string Termo) {
        termo = Termo;
        criaOperarias();
        criaArquivos();

        while(true) {
            auto aux = fila_arquivos;
            atualizaArquivos();
            listaArquivos(aux);
            executaOperaria();
            for(auto a : resultados) {
                cout << a.first << " " << a.second << endl;
            }
            cout << endl;
            sleep(3);
        }

    }
};

int main() {
    Thread_despachante despac1;
    string termo = "sistema";
    despac1.administra(termo);

    return 0;
}
