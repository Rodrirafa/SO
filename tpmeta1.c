
/* 
 * File:   tpmeta1.c
 * Author: rodrigo
 *
 * Created on 24 de Outubro de 2018, 15:54
 */

#include <stdio.h>
#include <stdlib.h>
#include "medit_defaults.h"
#include "server_defaults.h"
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>

WINDOW *vin;

void show_settings(Settings settings, char filenamepath[], int max_users, int npipes);
int verify_user(char filename[], char user[]);
void show_users();
void printmatrix(Settings settings, char tabela[settings.lines][settings.columns]);
void fillmatrix(Settings settings,char tabela[settings.lines][settings.columns]);
void edit(Settings settings, char tabela[settings.lines][settings.columns]);

int main(int argc, char** argv) {
    
   //---------------------Variáveis----------------------//
    
    char inputstring[50];      // string onde é guardada a string de comando da linha de comandos do servidor     
    char command[10];          // 1º parametro dos comandos da linha de comandos do servidor 
    char parametro[20];        // 2º parametro dos comandos da linha de comandos do servidor 
    char filepathname[100];   //pathname do ficheiro com usernames  
    //char caststring[20];    // ?
    char intfifostring[10]; //string para criação de fifos de interação
    char tempstring[20];    //string temporaria para operacoes
    int getoptresult;         // resultado da operação getopt()
    //int setenvresult;      // resultado da operação setenv
    int max_users;          //nº máximo de utilizadores
    int npipes;             // nº de pipes de interação
    int fdservidor;         // file descriptor do fifo para aceitação de clientes por parte do servidor
                              // vetor com file descriptors dos fifos para escrita com os clientes 
    Settings settings;      // Settings que armazenam os settings do servidor
    Text_Interaction_Vars itv; //variáveis para manipular a edição de texto
    Users tmp;
    
   //----------------------------------------------------//
    
   //--------- Variáveis de Ambiente --------------------//
    
    settings.columns = atoi(getenv("MEDIT_MAXCOLUMNS"));  
    settings.lines = atoi(getenv("MEDIT_MAXLINES"));     // estes valores podem mudar ou não no getopt() caso tenham sido incluidos 
    max_users = atoi(getenv("MEDIT_MAXUSERS"));    // na linha de comando que executou o servidor
    //fprintf(stderr,"D\n");
    strncpy(settings.pipe_name,getenv("FIFO"),100);
    settings.timeout=atoi(getenv("MEDIT_TIMEOUT"));
    strncpy(filepathname,getenv("FILE"),100);
    npipes = atoi(getenv("INTERACTIVE"));
    
   //--------------------------------------------------//
   
   //---------Obtenção de parâmetros da consola----------//
    
    opterr = 0;
    
    while((getoptresult = getopt(argc,argv,"c:l:f:n:p:m:t:")) != -1) //código adaptado da página sobre getopt() de gnu.org
    {
        switch(getoptresult)
        {
            case 'f':
                strncpy(filepathname,optarg,100);
                break;
            case 'c':
                settings.columns = atoi(optarg);
                break;
            case 'l':
                settings.lines = atoi(optarg);
                break;
            case 'p':
                strncpy(settings.pipe_name,optarg,100);
                break;
            case 'n':
                npipes = atoi(optarg);
                break;
            case 'm':
                max_users = atoi(optarg);
                break;
            case 't':
                settings.timeout = atoi(optarg);
                break;
            case '?'://getopt() devolve '?' se faltar um argumento de opção ou se o utilizador utilizou outro carater para alem dos carateres de opções 
                if (optopt == 'f' || optopt =='l' || optopt =='c' || optopt =='n' || optopt =='m' || optopt =='t' || optopt =='p'){
                    wprintw(vin, "Option -%c requires an argument.\n", optopt);
                }
                else if (isprint (optopt))  //se o carater de opção é printable
                {
                    wprintw(vin, "Unknown option '-%c'.\n", optopt);
                }
                else{
                    wprintw(vin,"Unknown option character '\\x%x'.\n",optopt);
                }
                wprintw(vin,"Entrei no case ?\n");
                wrefresh(vin);
                wgetch(vin);
                delwin(vin);
                endwin();
                return 1;
        }
    }
    
   //---------------------------------------------------------------------------//
    
   //------------Inicialização de varíáveis / pipes ----------//
    
    int fdinteractionfifos[npipes];     //file descriptors para ler dos clientes (FIFOs de interação)
    int ammountclientsfifos[npipes];   // array com o numero de clientes que cada fifo tem.
    int fdclientfifos[max_users];      //file descriptors para escrever para os clientes
    Users users[max_users];
    strncpy(tempstring,"intfifo",8);
    mkfifo(settings.pipe_name,0777);
    fdservidor = open(settings.pipe_name,O_RDWR);   // fifo aberto para leitura e escrita mas apenas será usado para leitura, a não ser em ocasiões especiais
    
    for(int i = 0; i < npipes; i++)
    {
        ammountclientsfifos[i] = 0;
        snprintf(intfifostring,9,"%s%d",tempstring,i);
        fprintf(stderr,"%s\n",intfifostring);
        mkfifo(intfifostring,0777);
        fdinteractionfifos[i] = open(intfifostring,O_RDWR);
    }
    
    for(int i = 0;i<max_users;i++)          //inicialliza o array de file descriptors para escrita com clientes a -1.
    {
        fdclientfifos[i] = -1;  
    }
    
   //--------------------------------------------------------// 
    
   //------------------Inicialização Janela------------//
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
    
   //---------------------------------------------------//
    
   //------------Verificação de utilizador 1ª meta------------//
    
    /*
    fillmatrix(settings,itv.tabela);
    wrefresh(vin);
    if(verify_user(filepathname,"ze")==1)
    {
        wprintw(vin,"O utilizador ze existe.\n");
    }
    else
        wprintw(vin, "O utilizador ze nao existe.\n");
     */
    
   //------------------------------------------------------------//
    
   //------------Código de obtenção de comandos------------//
    fd_set read_tmp, read_set;
    
    FD_ZERO(read_set);
    
    FD_SET(fdservidor, &read_set);  //fifo de receção
    FD_SET(0,&read_set);            //stdin
    
    for(int i=0;i<npipes;i++)
    {
        FD_SET(intfifos[i],&read_set);
    }
    while(1){
        FD_ZERO(&read_tmp);
        read_tmp = read_set;
        switch(select(npipes + 2, &read_tmp, NULL, NULL, NULL)) {
            case -1:
                if(errno=EINTR) continue;
                else {
                    perror("Unexpected select() error!");
                    unlink(fdservidor);
                    for(int i = 0; i < npipes;i++) 
                    {
                        snprintf(intfifostring,9,"%s%d",tempstring,i);
                        //strncat(intfifostring,c,7);
                        unlink(intfifostring);
                    }
                    return (EXIT_FAILURE);					
                }
                break;
            default:
                    break;
        }
        if(FD_ISSET(0, &read_tmp)){
            do{
                wprintw(vin,"Introduza um comando:\n->");
                wrefresh(vin);
                wscanw(vin,"%[^\n]s",&inputstring);
                sscanf(inputstring, "%s %s",command,parametro);
                if(strcmp(command,"settings")==0)
                {
                    show_settings(settings,filepathname,max_users,npipes);
                }
                if(strcmp(command,"load")==0)
                {
                    wprintw(vin,"ficheiro: %s\n",parametro);
                    wprintw(vin,"Funcao load por implementar\n");
                }
                if(strcmp(command,"save")==0)
                {
                    wprintw(vin,"ficheiro: %s\n",parametro);
                    wprintw(vin,"Funcao save por implementar\n");
                }
                if(strcmp(command,"free")==0)
                {
                    wprintw(vin,"ficheiro: %s\n",parametro);
                    wprintw(vin,"Funcao free por implementar\n");
                } 
                if(strcmp(command,"users")==0)
                {
                    show_users(filepathname);
                }
                if(strcmp(command,"edit")==0)
                {
                    edit(settings,itv.tabela);
                }
                if(strcmp(command,"text")==0)
                {
                    printmatrix(settings,itv.tabela);
                }

            }while(strncmp(command,"shutdown",8)!=0);
        }
        else if(FD_ISSET(fdservidor,&read_tmp))
        {
            if(!read(fdservidor,&tmp,sizeof(Users)))
            {
                wprintw(vin,"Error on read on server fifo\n");
            }
            else if(verify_user(tmp.username)){
                wprintw(vin,"Utilizador %s é valido\n",tmp.username);
                users[0].username = tmp.username;
                users[0].fifoname = tmp.fifoname;
                fdclientfifos[0] = open(users[0].fifoname,O_WRONLY);
                users[0].interactionfifo = select
                write(fdclientfifos[0],)
            }
            else
                kill(users[0].fifoname,SIGUSR1);
               
        }
        
    }
    
   //------------------------------------------------------------//
    unlink(settings.pipe_name);
    strncpy(intfifostring,"intfifo",7);
    for(int i = 0; i < npipes;i++) 
    {
        snprintf(intfifostring,9,"%s%d",tempstring,i);
        //strncat(intfifostring,c,7);
        unlink(intfifostring);
    }
    wprintw(vin, "Prima uma tecla para sair\n");
    wrefresh(vin);
    
    wgetch(vin);
    delwin(vin);
    endwin();
    return (EXIT_SUCCESS);
}
void show_settings(Settings settings, char filepathname[],int max_users, int npipes){
    wprintw(vin,"\n-----Parametros do Servidor-----\nCaminho da Base de Dados: %s\nNumero de linhas: %d\n"
            "Numero de colunas: %d\nNumero Maximo de utilizadores: %d\nNumero de named pipes Interacao: %d\n"
            "Caminho do named pipe principal: %s\nTimeout (em segundos) de edicao de linha: %d\n",
            filepathname,settings.lines,settings.columns,max_users,npipes,settings.pipe_name,settings.timeout);
}
int verify_user(char filename[],char user[]){
    char nome[50];
    char c;
    int i=0;
    int file = open(filename,O_RDONLY);
    if(file ==-1)
        wprintw(vin,"Erro ao abrir ficheiro\n");
    else
        wprintw(vin,"Ficheiro %s aberto com sucesso!\n",filename);
    while(read(file,&c,1)>0)
    {
        if(c!='\n'){         
            nome[i] = c;
            i++;
        }
        else
        {
            nome[i]='\0';
            i=0;
            if(strncmp(nome,user,8)==0)
                return 1;    
        }
        
    }
    return 0;
}
void show_users(char filename[])
{
    char nome[50];
    char c;
    int i=0,j=1;
    int file = open(filename,O_RDONLY);
    if(file ==-1)
        wprintw(vin,"Erro ao abrir ficheiro\n");
    else
        wprintw(vin,"Ficheiro %s aberto com sucesso!\n",filename);
    while(read(file,&c,1)>0)
    {
        if(c!='\n'){          
            nome[i] = c;
            i++;
        }
        else
        {
            nome[i]='\0';
            i=0;
            wprintw(vin,"Utilizador num%d: %s\n",j,nome);
            j++;
        }
    }
}
void edit(Settings settings, char tabela[settings.lines][settings.columns]){
    int mode=1,d;
    char ch,c='*',posx=0,posy=0,tmpchar;
    wclear(vin);
    //noecho();
    for(int i = 0;i<settings.lines;i++)
    {
        for(int j=0;j<settings.columns;j++)
        {
            if(j==0)
            {
                wprintw(vin,"%d",i+1);
            }
            wprintw(vin,"%c",tabela[i][j]);
            wrefresh(vin);
            if(j==settings.columns-1)
                wprintw(vin,"\n");
        }
    }
    //mvcur(0,0,0,0);
    //mvwaddch(vin,0,0,' ');
    //fscanf(stdout,"%d",&d);    
    //wprintw(vin,"%d\n",d);
    //mvwaddch(vin,0,0,c);
    //mvwaddch(vin,0,1,d + '0');
    wrefresh(vin);
    while(true)
    {
        if(mode==1)
        {
            ch = wgetch(vin);
            if(ch==27)           //27 = ESC
            {
                break;
            }
            if(ch==KEY_UP)
            {
                if(posy>0)
                {
                    wprintw(vin,"\b \b");
                    posy++;
                    mvwaddch(vin,posx,posy,c);
                    
                }
                break;
            }
        }
    }
}
void fillmatrix(Settings settings,char tabela[settings.lines][settings.columns])
{
    for(int i = 0;i<settings.lines;i++)
    {
        for(int j=0;j<settings.columns;j++)
        {
            tabela[i][j]= ' ';
        }
    }
}
void printmatrix(Settings settings, char tabela[settings.lines][settings.columns])
{
    wclear(vin);
    for(int i = 0;i<settings.lines;i++)
    {
        for(int j=0;j<settings.columns;j++)
        {
            if(j==0)
            {
                wprintw(vin,"%d",i+1);
            }
            wprintw(vin,"%c",tabela[i][j]);
            wrefresh(vin);
            if(j==settings.columns-1)
                wprintw(vin,"\n");
        }
    }
    wprintw(vin,"Clique num botao\n");
    wgetch(vin);
}