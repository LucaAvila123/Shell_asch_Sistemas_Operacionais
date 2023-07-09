#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void sigint_handler(int signum){
    printf("\nNão adianta me enviar o sinal por Ctrl-C. Estou vacinado!!\n");
}

void sigtstp_handler(int signum){
    printf("\nNão adianta me enviar o sinal por Ctrl-Z. Estou vacinado!!\n");
}

void sigquit_handler(int signum){
    printf("\nNão adianta me enviar o sinal por Ctrl-\\. Estou vacinado!!\n");
}

void main(){
    if (signal(SIGINT, sigint_handler) == SIG_ERR
       || signal(SIGTSTP, sigtstp_handler) == SIG_ERR
       || signal(SIGQUIT, sigquit_handler) == SIG_ERR) {
        perror("Erro ao registrar o manipulador de sinal");
        return;
    }

    int total_argumentos = 0;
    char** argv = (char**) malloc(100*sizeof(char*));
    char operacao[100];
    char* leitura;
    char* nome_shell = strdup("asch");

    char* nome_diretorio;
    char** caminho_diretorio = (char**) malloc(100*sizeof(char*));
    for(int i=0; i<100; i++){
        caminho_diretorio[i] = (char*) malloc(100*sizeof(char));
        caminho_diretorio[i] = NULL;
    }
    int total_caminho = 0;
    int abriu = 0;

    // isso vai ser usado para os comandos especiais
    char*** matriz_comandos = (char***) malloc(5*sizeof(char*));
    for(int i=0; i<5; i++){
        matriz_comandos[i] = (char**) malloc(100*sizeof(char*));
        matriz_comandos[i] = NULL;
    }
    int indice_matriz = 0;

    // a execução do shell só termina recebendo um comando pra isso
    while(1){
        for(int i=0; i<100; i++){
            caminho_diretorio[i] = NULL;
        }

        printf("%s> ", nome_shell);
        scanf("%[^\n]", operacao);
        getchar();

        leitura = strtok(operacao, " ");
        
        for(int i=0; leitura != NULL; i++){
            total_argumentos = i;
            argv[i] = strdup(leitura);
            leitura = strtok(NULL, " ");
        }
        total_argumentos++;

        /** BLOCO DE OPERAÇÕES INTERNAS **/
        if(!strcmp(argv[0], "cd")){
            if(total_argumentos > 2){
                printf("Erro: número excessivo de argumentos\n");
                continue;
            }

            // condição de retorno para o diretório inicial
            if(argv[1] == NULL){
                total_caminho = 0;
                for(int i=0; i<100; i++)
                    caminho_diretorio[i] = NULL;
                chdir(".");
                free(nome_shell);
                nome_shell = strdup("asch");

                continue;
            }

            if(!strcmp(argv[1], ".")) continue;
            
            // condição de retorno para pasta anterior (fecha e reabre diretório com outro nome)
            if(!strcmp(argv[1], "..")){

                if(total_caminho != 0){
                    caminho_diretorio[total_caminho-1] = NULL;
                    total_caminho--;
                    free(nome_diretorio);
                    nome_diretorio = NULL;

                    if(total_caminho != 0) {
                        nome_diretorio = strdup(caminho_diretorio[0]);
                        for(int i=1; caminho_diretorio[i] != NULL; i++){
                            strcat(nome_diretorio, "/");
                            strcat(nome_diretorio, caminho_diretorio[i]);
                        }

                        free(nome_shell);
                    }
                    nome_shell = strdup("asch");
                    
                    if(total_caminho != 0) {
                        strcat(nome_shell, "/");
                        strcat(nome_shell, nome_diretorio);
                    }

                    chdir(nome_diretorio);
                }

                continue;
            }

            // abre o diretório escolhido (se puder)
            if(total_caminho != 0){
                strcat(nome_diretorio, "/");
                strcat(nome_diretorio, argv[1]);
            }
            else{
                nome_diretorio = strdup(argv[1]);
            }


            abriu = chdir(nome_diretorio);

            if(abriu == -1){
                printf("Erro: diretório inexistente\n");
                free(nome_diretorio);
                continue;
            }

            caminho_diretorio[total_caminho] = strdup(argv[1]);
            total_caminho++;

            strcat(nome_shell, "/");
            strcat(nome_shell, argv[1]);

            continue;
        }

        else if(!strcmp(argv[0], "exit")){
            wait(NULL);
            exit(1);
        }

        /** BLOCO DE OPERAÇÃO EXTERNA **/
        int total_comandos = 0, comeco = 0;
        for(int i=0; i<total_argumentos; i++, comeco++){
            if(!strcmp(argv[i], "<3")){
                matriz_comandos[total_comandos][comeco] = NULL;
                comeco = 0;
                total_comandos++;
            }
            else{
                matriz_comandos[total_comandos][comeco] = strdup(argv[i]);
            }
        }
        pid_t pid = fork();
        if(pid == 0){
            execvp(matriz_comandos[0][0], matriz_comandos[0]);
            return;
        }
        else if(pid > 0){
            // se tem % depois do comando é porque é um comando só
            if(matriz_comandos[0][total_argumentos-1][0] == '%'){
                wait(NULL);
                continue;
            }
            pid = fork();
            if(pid == 0){
                execvp(matriz_comandos[1][0], matriz_comandos[1]);
                return;
            }
            else{
                pid = fork();
                if(pid == 0){
                    execvp(matriz_comandos[2][0], matriz_comandos[2]);
                }
                else{
                    pid = fork();
                    if(pid == 0){
                        execvp(matriz_comandos[3][0], matriz_comandos[3]);
                    }
                    else{
                        pid = fork();
                        if(pid == 0){
                            execvp(matriz_comandos[4][0], matriz_comandos[4]);
                        } 
                    }
                      
                }
            }
        }

        // essa parte comentada tinha dado certo
        // esse fork é necessario para deixar o processo em background
        /*pid_t pid = fork();
        if(pid == 0){
            execvp(argv[0], argv);
            return;
        }
        else if(pid > 0){
            // printf("%c\n", argv[total_argumentos-1][0]);
            // printf("%d\n", argv[total_argumentos-1][0]);
            // a tentativa de usar %% pra comparar string não deu certo
            if(argv[total_argumentos-1][0] == '%'){
                wait(NULL);
            }
            

                
        }
        else{
            printf("Erro no fork\n");
        }*/
    }
}