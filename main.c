#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>

#define MAX_CMD_LEN 300
#define MAX_ARGS 20

int main() {
    char linha[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    int i = 0;

    // Lê uma linha com o comando completo
    printf("Digite o comando a ser executado (ex: firefox, ls -l): ");
    fgets(linha, sizeof(linha), stdin);

    // Remove o '\n' no final (se houver)
    linha[strcspn(linha, "\n")] = 0;

    // Divide a linha em argumentos
    char *token = strtok(linha, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // Termina o vetor com NULL

    pid_t pid = fork();

    if (pid < 0) {
        perror("Erro ao criar processo");
        return 1;
    }

    if (pid == 0) {
        // Processo filho: executa o comando com execvp
        execvp(args[0], args);
        perror("Erro ao executar o comando");
        exit(1);
    } else {
        // Processo pai: espera o filho e mede uso de CPU
        int status;
        struct rusage usage;

        waitpid(pid, &status, 0);

        if (getrusage(RUSAGE_CHILDREN, &usage) == 0) {
            printf("\n=== Uso de CPU pelo processo filho ===\n");
            printf("Modo usuário: %ld.%06ld segundos\n",
                   usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
            printf("Modo sistema: %ld.%06ld segundos\n",
                   usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
        } else {
            perror("Erro ao obter uso de CPU");
        }
    }

    return 0;
}
