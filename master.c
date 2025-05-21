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

float creditos = 0.0;
const float CUSTO_CPU_POR_SEGUNDO = 2.0;
const float CUSTO_MEMORIA_POR_KB = 0.01;

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
    float maxmemoria;
    char nome[N];
    float resto_quota;
    float resto_memoria;

    printf("Bem vindo ao programa de monitoramento de processos!\n");

    printf("    ________  ________\n");
    printf("   / ____/  |/  / ___/\n");
    printf("  / /_  / /|_/ /\\__ \\ \n");
    printf(" / __/ / /  / /___/ / \n");
    printf("/_/   /_/  /_//____/\n\n");

    printf("Digite a quantidade inicial de créditos: ");
    if (scanf("%f", &creditos) != 1 || creditos < 0.0) return 1;

    printf("Digite a quota de computação em segundos: ");
    if (scanf("%f", &quota) != 1) return 1;

    printf("Digite o tempo limite em segundos: ");
    if (scanf("%f", &timeout) != 1) return 1;

    printf("Digite o máximo de memória (em KB): ");
    if (scanf("%f", &maxmemoria) != 1) return 1;
    getchar();

    while (1)
    {
        printf("\nCréditos disponíveis: %.2f\n", creditos);
        printf("Digite o nome do binário a ser executado (ou '0' para sair): ");
        if (!fgets(nome, N, stdin)) break;
        nome[strcspn(nome, "\n")] = 0;

        if (strcmp(nome, "0") == 0)
        {
            printf("Encerrando o programa.\n");
            if (child_pid > 0)
            {
                kill(child_pid, SIGKILL);
                waitpid(child_pid, NULL, 0);
            }
            break;
        }

        if (creditos <= 0.0)
        {
            printf("\nVocê não possui créditos suficientes para executar mais programas.\n");
            break;
        }

        time_t inicio_exec = time(NULL);

        int pid = fork();
        if (pid == 0)
        {
            char *argv[] = {nome, NULL};
            execvp(nome, argv);
            perror("Erro ao executar o comando");
            exit(127);
        }
        else if (pid > 0)
        {
            child_pid = pid;
            signal(SIGALRM, timeout_handler);
            alarm((unsigned int)timeout);

            int status;
            struct rusage usage;
            float soma;

            waitpid(pid, &status, 0);
            alarm(0);
            child_pid = -1;

            if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
            {
                printf("O binário '%s' não foi encontrado ou não pôde ser executado.\n", nome);
                continue;
            }

            if (getrusage(RUSAGE_CHILDREN, &usage) == 0)
            {
                printf("\n╔══════════════════════════════════════╗");
                printf("\n║              USO DE CPU              ║");
                printf("\n╠══════════════════════════════════════╣");
                printf("\n║ Modo usuário: %ld.%06ld segundos      ║",
                       usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
                printf("\n║ Modo sistema: %ld.%06ld segundos      ║",
                       usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
                soma = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0 +
                       usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
                resto_quota = quota - soma;
                resto_memoria = maxmemoria - usage.ru_maxrss;
                printf("\n║ Tempo total: %.6f segundos       ║", soma);

                float custo_cpu = soma * CUSTO_CPU_POR_SEGUNDO;
                float custo_memoria = usage.ru_maxrss * CUSTO_MEMORIA_POR_KB;
                float custo_total = custo_cpu + custo_memoria;

                if (custo_total > creditos)
                {
                    printf("\n╠══════════════════════════════════════╣");
                    printf("\n║ Créditos insuficientes!              ║");
                    printf("\n║ Necessário: %.2f | Disponível: %.2f   ║", custo_total, creditos);
                    printf("\n╚══════════════════════════════════════╝\n");
                    printf("Encerrando o programa.\n");
                    break;
                }

                creditos -= custo_total;

                printf("\n║ Tempo restante: %.6f segundos    ║", resto_quota);
                printf("\n╚══════════════════════════════════════╝");

                printf("\n╔════════════════════════════════╗");
                printf("\n║         USO DE MEMÓRIA         ║");
                printf("\n╠════════════════════════════════╣");
                printf("\n║ Memória usada:     %ld KB    ║", usage.ru_maxrss);
                printf("\n║ Memória limite:    %.0f KB    ║", maxmemoria);

                if (usage.ru_maxrss > maxmemoria)
                {
                    printf("\n║ Memória restante: %.0f      ║", resto_memoria);
                    printf("\n║ Excedeu o limite de memória!   ║");
                    printf("\n╚════════════════════════════════╝\n");
                    printf("Encerrando o programa.\n");
                    break;
                }
                else
                {
                    printf("\n║ Memória restante:   %.0f KB   ║", resto_memoria);
                    printf("\n╚════════════════════════════════╝");
                }

                printf("\n╔════════════════════════════════════╗");
                printf("\n║           CUSTO DA EXECUÇÃO        ║");
                printf("\n╠════════════════════════════════════╣");
                printf("\n║ CPU:     %.2f                      ║", custo_cpu);
                printf("\n║ Memória: %.2f                     ║", custo_memoria);
                printf("\n║ Total:   %.2f                     ║", custo_total);
                printf("\n║ Créditos restantes: %.2f          ║", creditos);
                printf("\n╚════════════════════════════════════╝");

                time_t fim_exec = time(NULL);
                printf("\n╔══════════════════════════════╗");
                printf("\n║ TEMPO TOTAL DE EXECUÇÃO      ║");
                printf("\n╠══════════════════════════════╣");
                printf("\n║ Tempo decorrido:  %ld segundos ║", fim_exec - inicio_exec);
                printf("\n╚══════════════════════════════╝\n");
            }
            else
            {
                perror("Erro ao obter uso de CPU");
            }
        }
        else
        {
            perror("Erro ao criar processo");
        }
    }
    getchar();
    return 0;
}
