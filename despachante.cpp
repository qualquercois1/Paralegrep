#include <iostream>
#include <pthread.h>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <map>
#include <unordered_set>

#include "operarias.h"

#define N_Operarias 4

using namespace std;

pthread_mutex_t mutex_filas = PTHREAD_MUTEX_INITIALIZER;

class Thread_despachante {
private:
    pthread_t                   thread_id;
    queue<string>               fila_arquivos;
    queue<Thread_operaria>      fila_threads;
    map<string, time_t>         arquivos_processados; 
    string                      termo;
    vector<pair<string, int>>   resultados;
    unordered_set<string>       arquivos_na_fila;

public:
    Thread_despachante() {
        thread_id = pthread_self();
    }

void listaArquivos() {
    const string diretorio = "fileset";
    DIR* dir = opendir(diretorio.c_str());
    if (!dir) {
        cerr << "Erro ao abrir o diretório: " << diretorio << endl;
        return;
    }

    struct dirent* entrada;
    while ((entrada = readdir(dir)) != nullptr) {
        if (strcmp(entrada->d_name, ".") != 0 && strcmp(entrada->d_name, "..") != 0) {
            string nome_arquivo = entrada->d_name;  // Usando apenas o nome do arquivo
            string caminho_completo = diretorio + "/" + nome_arquivo;

            struct stat info;
            if (stat(caminho_completo.c_str(), &info) == 0) {
                pthread_mutex_lock(&mutex_filas);
                
                // Verifica se o arquivo foi modificado
                if (arquivos_processados[caminho_completo] != info.st_mtime) {
                    // Se o arquivo foi alterado, remova da fila e coloque novamente
                    queue<string> temp_queue;
                    bool arquivo_encontrado = false;

                    // Remover o arquivo antigo da fila, se necessário
                    while (!fila_arquivos.empty()) {
                        string arquivo = fila_arquivos.front();
                        fila_arquivos.pop();
                        if (arquivo == nome_arquivo && !arquivo_encontrado) {
                            arquivo_encontrado = true;  // Encontramos o arquivo, então não o adicionamos de volta
                        } else {
                            temp_queue.push(arquivo);  // Mantém os outros arquivos
                        }
                    }

                    // Coloca o arquivo modificado novamente na fila
                    temp_queue.push(nome_arquivo);
                    fila_arquivos = temp_queue;

                    // Atualizar o timestamp do arquivo processado
                    arquivos_processados[caminho_completo] = info.st_mtime;

                    // A partir daqui, o processamento dos dados novos/alterados será feito
                    // Sem limpar os resultados anteriores, mas apenas adicionando os novos
                }
                
                pthread_mutex_unlock(&mutex_filas);
            }
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

    static void* threadExecutaOperaria(void* arg) {
        auto* despachante = static_cast<Thread_despachante*>(arg);
        despachante->executaOperaria();
        return nullptr;
    }

    void executaOperaria() {
        while (true) {
            pthread_mutex_lock(&mutex_filas);

            if (fila_threads.empty() || fila_arquivos.empty()) {
                pthread_mutex_unlock(&mutex_filas);
                sleep(1); // Evita espera ativa
                continue;
            }

            string arquivo = fila_arquivos.front();
            fila_arquivos.pop();

            Thread_operaria thread_op = fila_threads.front();
            fila_threads.pop();
            pthread_mutex_unlock(&mutex_filas);

            int ocorrencias = thread_op.executar(arquivo, termo);

            pthread_mutex_lock(&mutex_filas);
            fila_threads.push(thread_op);
            resultados.push_back({arquivo, ocorrencias});
            pthread_mutex_unlock(&mutex_filas);

            cout << arquivo << ", Ocorrências: " << ocorrencias << endl;
        }
    }

    void administra(string Termo) {
        termo = Termo;
        criaOperarias();

        pthread_t threads[N_Operarias];
        for (int i = 0; i < N_Operarias; ++i) {
            pthread_create(&threads[i], nullptr, threadExecutaOperaria, this);
        }

        while (true) {
            listaArquivos(); // Verifica se há novos ou alterados
            sleep(5);       // Aguarda antes de verificar novamente
            for(auto a : resultados) {
                cout << "debug: " << a.first << " " << a.second << endl;
            }  
        }

        for (int i = 0; i < N_Operarias; ++i) {
            pthread_join(threads[i], nullptr);
        }
        cout << "//////////" << endl;
    }
};

int main() {
    Thread_despachante despac1;
    string termo = "sistema";
    despac1.administra(termo);

    return 0;
}
