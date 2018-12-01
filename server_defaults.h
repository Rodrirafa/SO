
/* 
 * File:   server_defaults.h
 * Author: rodrigo
 *
 * Created on 24 de Outubro de 2018, 15:55
 */

#ifndef SERVER_DEFAULTS_H
#define SERVER_DEFAULTS_H
/*
#define NUMBER_OF_INTERACTIVE_NAMED_PIPES 3
#define FILENAME "medit.txt"
#define FIFONAME "fifogeral" */
#include <ncurses.h>
typedef struct{
    char tabela[15][45];
}Text_Interaction_Vars;
typedef struct{
    char msg[50];
    char fifoname[50];
    char interactionfifo[50];
    char username[50];
}Users;

#endif /* SERVER_DEFAULTS_H */

