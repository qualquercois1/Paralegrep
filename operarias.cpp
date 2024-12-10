#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <cstring>
#include <cstdlib>

using namespace std;

class Thread_operaria {
    private:
        pthread_t   thread_id;
        string      nomeArquivo;
        string      termo;
        int         palavras = 0;

        static pthread_mutex_t lock;

        static void* threadFunc(void* arg) {
            Thread_operaria* operaria = static_cast<Thread_operaria*>(arg);
            operaria->listarTexto();
            return nullptr;
        }

    public:
        Thread_operaria(const string& arquivo, const string& buscaTermo) : nomeArquivo(arquivo), termo(buscaTermo) {
            thread_id = pthread_self();
        }      

        ifstream abrirArquivo() const {
            string caminho = "fileset/" + nomeArquivo;
            ifstream arquivo(caminho);

            if (!arquivo.is_open()) {
                cerr << "Erro ao abrir arquivo: " << caminho << endl;
                exit(EXIT_FAILURE);
            }

            return arquivo;
        }

        void listarTexto() {
            ifstream arquivo = abrirArquivo();
            string linha;
            int contador = 0;

            pthread_mutex_lock(&lock);  
            while (getline(arquivo, linha)) {  
                if (linha.find(termo) != string::npos) { 
                    contador++;
                }
            }
            pthread_mutex_unlock(&lock); 

            palavras = contador;
            cout << contador << endl;
        }        

        void iniciarThread() {
            pthread_create(&thread_id, nullptr, threadFunc, this);
        }

        void esperarThread() {
            pthread_join(thread_id, nullptr);
        }

        void executar() {
            iniciarThread();
            esperarThread();
        }
};

pthread_mutex_t Thread_operaria::lock = PTHREAD_MUTEX_INITIALIZER;

int main() {
    Thread_operaria thread1("arq1.txt", "sistema");
    Thread_operaria thread2("arq2.txt", "comando");

    thread1.iniciarThread();
    thread2.iniciarThread();

    thread1.esperarThread();
    thread2.esperarThread();

    return 0;
}