//
//  main.c
//  chanel
//
//  Created by Паршиков Мирон on 19.11.17.
//  Copyright © 2017 Паршиков Мирон. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

long lenght;
int EOF_flag, BACKGROUND_MODE;

struct array{
    char ** mas;
    int count;
    int channel_count;
    int processes_count;
    int *flags;
};

void push_word (char **mas, char *str, int j){
    mas[j] = (char*)malloc(strlen(str) + 1);
    strcpy(mas[j], str);
}

void mas_print (struct array words){
    int i = 0;
    if (words.mas) {
        while (i < words.count) {
            if (words.mas[i] != NULL) {
                printf("%s\n", words.mas[i]);
            }
            i++;
        }
    }
}

void struct_mas_delete (struct array words){
    int i = 0;
    if (words.mas) {
        while (i < words.count) {
            if (words.mas[i] != NULL) {
                free(words.mas[i]);
            }
            i++;
        }
        free(words.mas);
    }
}

char* input(FILE* fp) {
    int i = 0;
    char c, *s = (char*)malloc(30*sizeof(char));
    c = fgetc(fp);
    while (c != EOF && c != '\n') {
        s[i] = c;
        i++;
        if (i % 30 == 0) {
            s = (char*)realloc(s, (i + 30)*sizeof(char));
        }
        c = getc(fp);
    }
    if (c == EOF) {
        EOF_flag = 1;
    }
    if (i == 0) {
        return NULL;
    }
    s[i] = '\0';
    return s;
}

char* glue_together (char *s, int *i){
    char  *word = (char*)malloc(30*sizeof(char));
    int j = 0;
    while ((s[*i] != '\n') && (s[*i] != ' ') && (s[*i] != '|') && (s[*i] != EOF) && (s[*i] != '\0') && (s[*i] != '>') && (s[*i] != '<') && (s[*i] != ';') && (s[*i] != '&')) {
        if (s[*i] == '\"') {
            (*i)++;
            while ((s[*i] != '\n') && (s[*i] != EOF) && (s[*i] != '\"') && (s[*i] != '\0')) {
                word[j] = s[*i];
                j++;
                if (j % 30 == 0) {
                    word = (char*)realloc(word, (j + 30)*sizeof(char));
                }
                (*i)++;
            }
            if (s[*i] != '\"') {
                free(word);
                printf("Ошибка : не хватает закрывающей кавычки.\n");
                return NULL;
            }
            if (s[*i] == '\"') {
                (*i)++;
            }
        }
        else {
            word[j] = s[*i];
            j++;
            if (j % 30 == 0) {
                word = (char*)realloc(word, (j + 30)*sizeof(char));
            }
            (*i)++;
        }
    }
    word[j] = '\0';
    if (strlen(word) > 0) {
        return word;
    }
    return NULL;
}

struct array string_analys (char* s, int *i){
    struct array words;
    char **massiv = (char**)malloc(10*sizeof(char*)), *word;
    int j = 0, channels = 0, processes = 1, *flags = (int*)malloc(10*sizeof(int)), k = 0;
    while (*i < lenght) {
        while (s[*i] == ' ') {
            (*i)++;
        }
        if (s[*i] == '&' || s[*i] == ';' || s[*i] == '|'){
            printf("Ошибка при вводе строки.\n");
            words.mas = NULL;
            return words;
        }
        word = glue_together(s, i);
        if (word != NULL) {
            push_word(massiv, word, j);
            j++;
            free(word);
        }
        while (s[*i] == ' ' && (s[*i] != '\0')) {
            (*i)++;
        }
        if (j % 10 == 0) {
            massiv = (char**)realloc(massiv, (j + 10)* sizeof(char*));
        }
        flags[k] = 0;
        if (s[*i] == '>'){
            if (s[*i+1] == '>'){
                word = (char*)malloc(3*sizeof(char));
                word[0] = '>';
                word[1] = '>';
                word[2] = '\0';
                (*i)++;
            }
            else{
                word = (char*)malloc(2*sizeof(char));
                word[0] = '>';
                word[1] = '\0';
            }
            push_word(massiv, word, j);
            j++;
            (*i)++;
            free(word);
        }
        if (s[*i] == '<'){
            word = (char*)malloc(2*sizeof(char));
            word[0] = '<';
            word[1] = '\0';
            push_word(massiv, word, j);
            j++;
            (*i)++;
            free(word);
        }
        if (s[*i] == '|') {
            massiv[j] = NULL;
            j++;
            if (j % 10 == 0) {
                massiv = (char**)realloc(massiv, (j + 10)* sizeof(char*));
            }
            if (s[*i+1] == '|'){
                massiv[j] = NULL;
                j++;
                *i+=2;
                processes++;
                flags[k] = 1;
                k++;
            }
            else{
                channels++;
                (*i)++;
            }
        }
        if (s[*i] == ';'){
            (*i)++;
            break;
        }
        if (s[*i] == '&') {
            if (s[*i+1] == '&'){
                massiv[j] = NULL;
                j++;
                if (j % 10 == 0) {
                    massiv = (char**)realloc(massiv, (j + 10)* sizeof(char*));
                }
                massiv[j] = NULL;
                j++;
                *i+=2;
                processes++;
                flags[k] = 2;
                k++;
            }
            else{
                (*i)++;
                BACKGROUND_MODE = 1;
                break;
            }
        }
        if (k % 10 == 0){
            flags = (int*)realloc(flags, (k+10)*sizeof(int));
        }
        if (j % 10 == 0) {
            massiv = (char**)realloc(massiv, (j + 10)* sizeof(char*));
        }
    }
    massiv[j] = NULL;
    words.processes_count = processes;
    words.mas = massiv;
    words.count = j;
    words.channel_count = channels;
    words.flags = flags;
    return words;
}

int repeat_count (char *s){
    int j = 0, count = 0;
    while (s[j] != '\0') {
        if (s[j] == ';' || s[j] == '&') {
            count++;
        }
        j++;
    }
    return count;
}

int which_file_to_open (char **mas, int i, int *flag){
    int file=0;
    while (mas[i+2] != NULL && (strcmp(mas[i+2], ">") != 0 || strcmp(mas[i+2], ">>") != 0 || strcmp(mas[i+2], "<") != 0)) {
        i++;
    }
    if (strcmp(mas[i], ">") == 0) {
        file = open(mas[i+1], O_WRONLY|O_TRUNC|O_CREAT, 0666);
        *flag = 1;
    }
    if (strcmp(mas[i], ">>") == 0) {
        file = open(mas[i+1], O_WRONLY|O_CREAT, 0666);
        lseek(file, 0, SEEK_END);
        *flag = 1;
    }
    if (strcmp(mas[i], "<") == 0) {
        file = open(mas[i+1], O_RDONLY, 0);
        *flag = 0;
    }
    return file;
}

void process_born (char** mas, int ppid, int k){
    int i = 0;
    int file = 0;
    int flag = 2;
    if (mas[0] != NULL) { //проверка на случай, если в первом же слове ошибка в количестве кавычек
        while (mas[i] != NULL) {
            if (strcmp(mas[i], ">") == 0 || strcmp(mas[i], ">>") == 0 || strcmp(mas[i], "<") == 0) {
                file = which_file_to_open(mas, i, &flag);
                mas[i] = NULL;
                i++;
                while (mas[i] != NULL)
                    i++;
                break;
            }
            i++;
        }
        if (fork() == 0) {
            setpgid(getpid(), ppid);
            if (flag == 1) {
                dup2(file, 1);
            }
            if (flag == 0) {
                dup2(file, 0);
            }
            execvp(mas[k], mas + k);
            perror("execvp failed");
            exit(1);
        }
    }
}

void conveyor (char **mas, int channel_count){
    if (fork() == 0) {
        int k = 0, s, j = 0;// j - для количества программ, k - смещение в массиве слов (чтобы находить откуда брать новое имя программы и данные)
        int flag = 2;
        int file = 0;
        int fd[2];
        while (j < channel_count + 1) {
            s = k;
            while (mas[s] != NULL) {
                if (strcmp(mas[s], ">") == 0 || strcmp(mas[s], ">>") == 0 || strcmp(mas[s], "<") == 0) {
                    file = which_file_to_open(mas, s, &flag);
                    mas[s] = NULL;
                    s++;
                    while (mas[s] != NULL)
                        s++;
                    break;
                }
                s++;
            }
            if (mas[k] == NULL) {
                printf("ERROR\n");
                break;
            }
            pipe(fd);
            switch (fork()) {
                case 0:{
                    if (flag == 1) {
                        dup2(file, 1);
                    }
                    else
                        if (flag == 0){
                        dup2(file, 0);
                    }
                    else
                        if (j+1 != channel_count + 1) {
                            dup2(fd[1], 1);
                        }
                    close(fd[1]);
                    close(fd[0]);
                    execvp(mas[k], mas + k);
                    perror("execvp failed");
                    exit(1);
                }
                    break;
            }
            if (flag == 1 || flag == 0) {
                k = s;
            }
            while (mas[k] != NULL) {
                k++;
            }
            k++;
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            j++;
            flag = 2;
        }
        while (wait(0)!= -1) {
            ;
        }
        exit(0);
    }
}

void COMMON (struct array words){
    int pid;
    if ((pid = fork()) == 0) {
        signal(SIGCHLD, SIG_DFL);
        int k = 0, j = 0, status, skip_flag = 0;
        for (int i = 0; i < words.processes_count; i++) {
            if (k < words.count && words.mas[k] != NULL) {
                words.channel_count = 0;
                while ( j + 1< words.count) {
                    if ((words.mas[j] == NULL)&&(words.mas[j+1]==NULL)) break;
                    if ((words.mas[j] == NULL)&&(words.mas[j+1]!=NULL)) {
                        words.channel_count++;
                    }
                    j++;
                }
                j+=2;
                if (skip_flag != 1) {
                    if (words.channel_count != 0) {
                        conveyor(words.mas+k, words.channel_count);
                    }
                    else{
                        process_born(words.mas, 0, k);
                    }
                    wait(&status);
                    switch (words.flags[i]) {
                        case 1:{
                            if (status > 0) skip_flag = 0;
                            else skip_flag = 1;
                        }
                            break;
                        case 2:{
                            if (status > 0) skip_flag = 1;
                            else skip_flag = 0;
                        }
                            break;
                    }
                }
                if (i - 1 >= 0) {
                    if (words.flags[i-1] == 1 && words.flags[i] == 2 && skip_flag == 1) {
                        skip_flag = 0;
                    }
                    else
                    if (words.flags[i-1] == 2 && words.flags[i] == 1 && skip_flag == 1) {
                        skip_flag = 0;
                    }
                    else
                    if (words.flags[i-1] == 2 && words.flags[i] == 1 && skip_flag == 0) {
                        skip_flag = 1;
                    }
                }
                while (k+1< words.count) {
                    if ((words.mas[k] == NULL)&&(words.mas[k+1]==NULL)) break;
                    k++;
                }
                k+=2;
            }
            else break;
        }
        exit(0);
    }
    waitpid(pid, NULL, 0);
}

void BACKGROUND (struct array words, int* pids, int* i){
    if ((pids[*i] = fork()) == 0) {
        setpgid(getpid(), getpid());
        signal(SIGCHLD, SIG_DFL);
        if (fork() == 0) {
            signal(SIGCHLD, SIG_IGN);
            signal(SIGINT, SIG_IGN);
            int k = 0, j = 0, status , skip_flag = 0;
            for (int i = 0; i < words.processes_count; i++) {
                if (k < words.count && words.mas[k] != NULL) {
                    words.channel_count = 0;
                    while ( j + 1< words.count) {
                        if ((words.mas[j] == NULL)&&(words.mas[j+1]==NULL)) break;
                        if ((words.mas[j] == NULL)&&(words.mas[j+1]!=NULL)) {
                            words.channel_count++;
                        }
                        j++;
                    }
                    j+=2;
                    if (skip_flag != 1) {
                        if (words.channel_count != 0) {
                            conveyor(words.mas+k, words.channel_count);
                        }
                        else{
                            process_born(words.mas, getppid(), k);
                        }
                        wait(&status);
                        switch (words.flags[i]) {
                            case 1:{
                                if (status > 0) skip_flag = 0;
                                else skip_flag = 1;
                            }
                                break;
                            case 2:{
                                if (status > 0) skip_flag = 1;
                                else skip_flag = 0;
                            }
                                break;
                        }
                    }
                    if (i - 1 >= 0) {
                        if (words.flags[i-1] == 1 && words.flags[i] == 2 && skip_flag == 1) {
                            skip_flag = 0;
                        }
                        else
                        if (words.flags[i-1] == 2 && words.flags[i] == 1 && skip_flag == 1) {
                            skip_flag = 0;
                        }
                        else
                        if (words.flags[i-1] == 2 && words.flags[i] == 1 && skip_flag == 0) {
                            skip_flag = 1;
                        }
                    }
                    while (k+1< words.count) {
                        if ((words.mas[k] == NULL)&&(words.mas[k+1]==NULL)) break;
                        k++;
                    }
                    k+=2;
                }
                else break;
            }
            exit(0);
        }
        wait(0);
        exit(0);
    }
    (*i)++;
}

void kill_son (int s){
    signal(SIGCHLD, kill_son);
    wait(0);
}

int main(int argc, const char * argv[]) {
    signal(SIGCHLD, kill_son);
    int i;
    char *string;
    struct array words;
    FILE *fp;
    if (argc < 2) {
        fp = stdin;
    }
    else{
        fp = fopen(argv[1], "r");
    }
    int f = 0, *pids = (int*)malloc(10*sizeof(int));
    while (EOF_flag != 1) {
        printf(">");
        string = input(fp);
        i = 0;
        if (string != NULL) {
            lenght = strlen(string);
            for (int j = 0; j < repeat_count(string) + 1; j++) {
                BACKGROUND_MODE = 0;
                words = string_analys(string, &i);
                if (words.mas != NULL && words.mas[0] != NULL) {
                    if (strcmp("cd", words.mas[0]) == 0) {
                        chdir(words.mas[1]);
                        struct_mas_delete(words);
                        break;
                    }
                    if (BACKGROUND_MODE == 1) {
                        BACKGROUND(words, pids, &f);
                        if ((f % 10) == 0) {
                            pids = (int*)realloc(pids, (f+10)*sizeof(int));
                        }
                        struct_mas_delete(words);
                        free(words.flags);
                        continue;
                    }
                    COMMON(words);
                    struct_mas_delete(words);
                    free(words.flags);
                }
                else break;
            }
        }
        free(string);
    }
    for (int i = 0; i < f; i++) {
        printf("%d\n", pids[i]);
        kill(-pids[i], SIGKILL);
    }
    free(pids);
    fclose(fp);
    return 0;
}



















