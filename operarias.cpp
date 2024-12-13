#include "operarias.h"


void* Thread_operaria::threadFunc(void* arg) {
    Thread_operaria* operaria = static_cast<Thread_operaria*>(arg);
    operaria->listarTexto();
    return nullptr;
}

ifstream Thread_operaria::abrirArquivo() const {
    string caminho = "fileset/" + nomeArquivo;
    ifstream arquivo(caminho);

    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir arquivo: " << caminho << endl;
        exit(EXIT_FAILURE);
    }

    return arquivo;
}

void Thread_operaria::listarTexto() {
    ifstream arquivo = abrirArquivo();
    string linha;
    int contador = 0;
 
    while (getline(arquivo, linha)) {  
        if (linha.find(termo) != string::npos) { 
            contador++;
        }
    }

    palavras = contador;
}

void Thread_operaria::iniciarThread() {
    pthread_create(&thread_id, nullptr, threadFunc, this);
}


void Thread_operaria::esperarThread() {
    pthread_join(thread_id, nullptr);
}

int Thread_operaria::executar(string arquivo, string nomeTermo) {
    nomeArquivo = arquivo;
    termo = nomeTermo;
    iniciarThread();
    esperarThread();
    return palavras;
}
