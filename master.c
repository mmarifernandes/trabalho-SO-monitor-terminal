#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

const int N = 300;

int main() {
    float quota;
    float timeout;
    int maxmemoria;
    char nome[N];
    char comando[N];
    float resto_quota;
    printf("Digite a quota de computação em segundos: ");
    scanf("%f", &quota);

    printf("Digite o tempo limite em segundos: ");
    scanf("%f", &timeout);

    printf("Digite o máximo de memória (em KB): ");
    scanf("%d", &maxmemoria);

    printf("Digite o nome do binário a ser executado: ");
    getchar();
    fgets(nome, N, stdin);
    nome[strcspn(nome, "\n")] = 0;

    int pid = fork();
    if (pid == 0) {
        // Processo filho: executa o comando digitado
        execlp(nome, nome, (char *)NULL);
        perror("Erro ao executar o comando");
        //exit(EXIT_FAILURE);
    } else if (pid > 0) {
        
        int status;
        struct rusage usage;
        float soma;

        waitpid(pid, &status, 0);
        printf("Restam %f segundos da sua quota de cpu", quota);
        if (getrusage(RUSAGE_CHILDREN, &usage) == 0) {
            printf("\n=== Uso de CPU ===\n");
            printf("Modo usuário: %ld.%06ld segundos\n",
               usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
            printf("Modo sistema: %ld.%06ld segundos\n",
               usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
            printf("=================\n");

            soma = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0 + usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
            printf("Soma dos tempos de CPU (usuário + sistema): %.6f segundos\n", soma);

            resto_quota = quota - soma;
            if (resto_quota < 0) {
                printf("O tempo de execução excedeu a quota de CPU.\n");
            } else {
                printf("Restam %.6f segundos da quota de CPU.\n", resto_quota);
            }
            printf("Memória máxima utilizada: %ld KB\n", usage.ru_maxrss);
            if (usage.ru_maxrss > maxmemoria) {
                printf("A memória utilizada excedeu o máximo permitido de: %d KBs\n", maxmemoria);
            } else {
                printf("A memória utilizada está dentro do limite.\n");
            }

        } else {
            perror("Erro ao obter uso de CPU");
        }
        wait(NULL);
    } else {
        perror("Erro ao criar processo");
    }

    return 0;
}

