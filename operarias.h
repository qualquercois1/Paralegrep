#ifndef __THREAD_OPERARIA_H
#define __THREAD_OPERARIA_H

#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <cstring>
#include <cstdlib>

using namespace std;

class Thread_operaria {
private:
    pthread_t thread_id;
    string nomeArquivo;
    string termo;
    int palavras = 0;

    static void* threadFunc(void* arg);

public:

    ifstream abrirArquivo() const;
    void listarTexto();
    void iniciarThread();
    void esperarThread();
    int executar(string arquivo, string nomeTermo);
};

#endif
