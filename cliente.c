/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cliente.c
 * Author: rodrigo
 *
 * Created on 21 de Novembro de 2018, 14:56
 */

#include <stdio.h>
#include <stdlib.h>
#include "medit_defaults.h"
#include "client_defaults.h"
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_FIFO
WINDOW *vin;
int main(int argc, char** argv) {
    Text_Interaction_Vars itv;
    Settings settings;
    int fdgeral,fdcliente;
    
    itv.fifoname = getpid();
    mkfifo(itv.fifoname(),0777);
    
    settings.columns = atoi(getenv("MEDIT_MAXCOLUMNS"));  
    settings.lines = atoi(getenv("MEDIT_MAXLINES"));     // estes valores podem mudar ou não no getopt() caso tenham sido incluidos 
                                                        // na linha de comando que executou o servidor
    //fprintf(stderr,"D\n");
    strncpy(settings.pipe_name,getenv("FIFO"),100);
    settings.timeout=atoi(getenv("MEDIT_TIMEOUT"));
    
    fdgeral = open(settings.pipe_name, O_WRONLY);
    fdcliente = open(itv.fifoname);
    
    initscr();
    start_color();                         //codigo de inicialização da window
    init_pair(1,COLOR_WHITE,COLOR_BLACK);
    vin = newwin(0,0,0,0);
    scrollok(vin,TRUE);                     //deixar com que a janela seja scrollable, que o texto possa ser todo representado
    wbkgd(vin,COLOR_PAIR(1));           //as cores do pair 1 serao as da janela
    keypad(stdscr,TRUE);
    cbreak();
    //wprintw(vin, "Olá %d %d\n",atoi(getenv("COLUMNS")),atoi(getenv("LINES")));
    wrefresh(vin);
    
    return (EXIT_SUCCESS);
}

