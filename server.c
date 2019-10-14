/* Éste es el código del servidor */
#include <sys/types.h> 
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/socket.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* arbitrario, pero el cliente y el servidor deben coincidir */
#define SERVER_PORT 12345 

/* tamaño de bloque para la transferencia */
#define BUF_SIZE 4096 
#define QUEUE_SIZE 10
#define STATIC_PASSWORD "Jipixz"
#define true 1
#define false 0
#define DATA_PORT 54321

void fatal(char *string);

/*Definicion del menú de comandos*/
void menu(int sa, char* option, int* userAccount);

/*Definicion de la funcion que valida el usuario*/
char* userCommand(char* option, int* userAccount);

/*Definicion de la funcion de validación de la contraseña*/
char* passwordCommand(char* password, int* userAccount);

/*Definicion de la conexión pasiva*/
void passiveCommand(int sa);

/*Definicion del menu de la conexion pasiva*/
void menuPassive(int sa, char* string, int* userAccount);

/*Comando LIST*/
char *listCommand(int sa);

int main(int argc, char *argv[]){

    int s, b, l, fd, sa, bytes, on = 1;

    /*Inicia sin que alguien haya iniciado sesión*/
    int userAccount = false;

    /* búfer para el archivo saliente */
    char buf[BUF_SIZE]; 

    /* contiene la dirección IP */
    struct sockaddr_in channel; 

    /* Construye la estructura de la dirección para enlazar el socket. */
    memset(&channel, 0, sizeof(channel)); /* canal cero */
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(SERVER_PORT);

    /* Apertura pasiva. Espera una conexión. */
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); /* crea el socket */
    if (s < 0) fatal("socket failed");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
    if (b < 0) fatal("bind failed");

    /* especifica el tamaño de la cola */
    l = listen(s, QUEUE_SIZE); 
    if (l < 0) fatal("listen failed");
    /* El socket ahora está configurado y enlazado. Espera una conexión y la procesa. */

    /* se bloquea para la solicitud de conexión */
    sa = accept(s, 0, 0); 
    if (sa < 0) fatal("accept failed");
    fprintf(stdout, "Conexión entrante\n\n");
    write (sa, "Conexión establecida\n\n",25);
    // end
    while (1){
        /* lee el comando desde el socket [buf]*/ 
        read(sa, buf, BUF_SIZE); 
        menu(sa,buf,&userAccount);
        memset(buf, 0, BUF_SIZE); // clear buffer;
        //fprintf(stdout, "WHILE 2");
    }
    close(sa);
    /* 
    inside while cierra la conexión
    */
}

void menu(int sa, char* string, int* userAccount){

    char response[BUF_SIZE]="";

    /*Divición del string entrante en tokens con delimitadores*/
    char *option = strtok(string," ");

    if (strcmp(option,"USER") == 0){

        strcpy(response,userCommand(strtok(NULL," "), userAccount));

    } else if (strcmp(option,"PASS") == 0) {

        strcpy(response,passwordCommand(strtok(NULL," "), userAccount));

    } else if (strcmp(option,"PORT") == 0) {

        strcpy(response,"200 Orden correcta.\n");

    } else if (strcmp(option,"LIST") == 0) {

        strcpy(response,"150 Estado del fichero correcto; va a abrirse la conexión de datos.\n");

    } else if (strcmp(option,"PASV") == 0) {
        
        passiveCommand(sa);

    } else if (strcmp(option,"SYST") == 0) {

        strcpy(response,"215 NOMBRE system type. Donde NOMBRE es un nombre de sistema oficial de la lista que hay en el documento Números Asignados.\n");

    } else if (strcmp(option,"QUIT") == 0){
        
        
        
    }else {

        strcpy(response,"\nComandos disponibles:\n- USER\n- PASS\n- PORT\n- LIST\n- PASV\n- SYST\n");

    }
    
    write(sa, response, strlen(response));
}

void fatal(char *string)
{
    printf("%s\n", string);
    exit(1);
}

char* userCommand(char* user,int* userAccount){
    if (user==NULL){
        return (char*)"332 Need account for login.\n";
    }
    *(userAccount)=true;
    return (char*)"331 User name okay, need password.\n";
}

char* passwordCommand(char* password, int* userAccount){
    if(!*(userAccount)){
        return (char*)"332 Need account for login.\n";
    }
    if(password == NULL || strcmp(password,STATIC_PASSWORD) != 0){
        return (char*)"530 Not logged in.\n";
    }
    return (char*)"230 User logged in, proceed.\n";
}
void passiveCommand(int sa){
    char *response = (char*)"passiveON 127.0.0.1 54321\n";
    write(sa, response, strlen(response));
    int s, b, l, fd, bytes, on = 1;
    int userAccount=false;
    char buf[BUF_SIZE]; /* búfer para el archivo saliente */
    struct sockaddr_in channel; /* contiene la dirección IP */
    /* Construye la estructura de la dirección para enlazar el socket. */
    memset(&channel, 0, sizeof(channel)); /* canal cero */
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(DATA_PORT);
    /* Apertura pasiva. Espera una conexión. */
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); /* crea el socket */
    if (s < 0) fatal((char*)"socket failed");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
    if (b < 0) fatal((char*)"bind failed");
    l = listen(s, QUEUE_SIZE); /* especifica el tamaño de la cola */
    if (l < 0) fatal((char*)"listen failed");
    /* El socket ahora está configurado y enlazado. Espera una conexión y la
    /* procesa. */
    // inside while
    sa = accept(s, 0, 0); /* se bloquea para la solicitud de conexión */
    if (sa < 0) fatal((char*)"accept failed");
    while (1) {
        read(sa, buf, BUF_SIZE); /* lee el comando desde el socket [buf]*/ 
        menuPassive(sa,buf, &userAccount);
        memset(buf, 0, BUF_SIZE); // clear buffer;
    }
    close(sa); /* cierra la conexión*/
}


void menuPassive(int sa, char* string, int*  userAccount){
    char response[BUF_SIZE];
    char *option = strtok(string," "); // obtain first part of command
    
    if (strcmp(option,"LIST") == 0) {
        //strcpy(response,"LIST: \n");
        listCommand(sa);
    } else {
        strcpy(response,"\nComandos disponibles:\n- LIST\n");
    }
    
    write(sa, response, strlen(response));
}

char *listCommand(int sa){
    int bytes,fd;
    char* buf[BUF_SIZE];
    system((char*)"ls -l > lsCommand.txt");

    /* abre el archivo para regresarlo */
    fd = open((char*)"lsCommand.txt", O_RDONLY); 
    if (fd < 0) fatal((char*)"open failed");
    while (1) {
        /* lee del archivo */
        bytes = read(fd, buf, BUF_SIZE); 
        /* verifica el final del archivo */
        if (bytes <= 0) break; 
        /* describe bytes en el socket */
        write(sa, buf, bytes); 
    }
}