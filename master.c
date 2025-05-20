#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

const int N = 300;
int child_pid = -1;

void timeout_handler(int sig)
{
    if (child_pid > 0)
    {
        printf("\nTempo limite excedido! Encerrando o processo filho.\n");
        kill(child_pid, SIGKILL);
    }
}

int main()
{
    float quota;
    float timeout;
    int maxmemoria;
    char nome[N];
    float resto_quota;

    // Coleta única das configurações
    printf("Digite a quota de computação em segundos: ");
    scanf("%f", &quota);

    printf("Digite o tempo limite em segundos: ");
    scanf("%f", &timeout);

    printf("Digite o máximo de memória (em KB): ");
    scanf("%d", &maxmemoria);
    getchar(); // Limpa o \n deixado pelo último scanf

    while (1){
        printf("\nDigite o nome do binário a ser executado (ou '0' para sair): ");
        fgets(nome, N, stdin);
        nome[strcspn(nome, "\n")] = 0;

        if (strcmp(nome, "0") == 0){
            printf("Encerrando o programa.\n");
            break;
        }

        time_t inicio_exec = time(NULL);

        int pid = fork();
        if (pid == 0){
            execlp(nome, nome, (char *)NULL);
            perror("Erro ao executar o comando");
            //exit(EXIT_FAILURE);
        }
        else if (pid > 0){
            child_pid = pid;
            signal(SIGALRM, timeout_handler);
            alarm((unsigned int)timeout);

            int status;
            struct rusage usage;
            float soma;

            waitpid(pid, &status, 0);
            alarm(0); // Cancela o alarme se o processo terminar antes do timeout

            if (getrusage(RUSAGE_CHILDREN, &usage) == 0){
                printf("\n=== Uso de CPU ===\n");
                printf("Modo usuário: %ld.%06ld segundos\n",
                       usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
                printf("Modo sistema: %ld.%06ld segundos\n",
                       usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
                printf("=================\n");

                soma = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0 +
                       usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
                resto_quota = quota - soma;

                printf("Soma dos tempos de CPU (usuário + sistema): %.6f segundos\n", soma);

                if (resto_quota < 0){
                    printf("O tempo de execução excedeu a quota de CPU.\n");
                    kill(pid, SIGKILL);
                }
                else{
                    printf("Restam %.6f segundos da quota de CPU.\n", resto_quota);
                }

                printf("Memória máxima utilizada: %ld KB\n", usage.ru_maxrss);
                if (usage.ru_maxrss > maxmemoria){
                    printf("A memória utilizada excedeu o máximo permitido de: %d KBs\n", maxmemoria);
                    kill(pid, SIGKILL);
                    exit(1);
                }
                else{
                    printf("A memória utilizada está dentro do limite.\n");
                }

                time_t fim_exec = time(NULL);
                printf("Tempo decorrido: %ld segundos\n", fim_exec - inicio_exec);
            }
            else{
                perror("Erro ao obter uso de CPU");
            }
        }
        else
        {
            perror("Erro ao criar processo");
        }
    }

    return 0;
}
