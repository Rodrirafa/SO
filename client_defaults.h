/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   client_defaults.h
 * Author: rodrigo
 *
 * Created on 24 de Outubro de 2018, 15:55
 */

#ifndef CLIENT_DEFAULTS_H
#define CLIENT_DEFAULTS_H
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

#endif /* CLIENT_DEFAULTS_H */

