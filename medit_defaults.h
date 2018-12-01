
/* 
 * File:   medit_defaults.h
 * Author: rodrigo
 *
 * Created on 24 de Outubro de 2018, 15:55
 */

#ifndef MEDIT_DEFAULTS_H
#define MEDIT_DEFAULTS_H
/*
#define MEDIT_MAX_LINES 15
#define MEDIT_MAX_COLUMNS 45
#define MEDIT_TIMEOUT 10
#define MEDIT_MAX_USERS 3*/

typedef struct{
    int lines;
    int columns;
    int timeout;
    char pipe_name[100];
}Settings;
#endif /* MEDIT_DEFAULTS_H */

