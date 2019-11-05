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
#include <time.h>

/* arbitrario, pero el cliente y el servidor deben coincidir */
#define SERVER_PORT 21

/* tamaño de bloque para la transferencia */
#define BUF_SIZE 4096 
#define QUEUE_SIZE 10
#define STATIC_PASSWORD "Jipixz"
#define true 1
#define false 0
#define DATA_PORT 20

int userAuth = false;
int userPass = false;
int sData = false;
char currentDirectory [BUF_SIZE] = "/";

void fatal(char *string);

/*Definicion del menú de comandos*/
int menu(int sa, char* option, int* userAccount);

/*Definicion de la funcion que valida el usuario*/
char* userCommand(char* option, int* userAccount);

/*Definicion de la funcion de validación de la contraseña*/
char* passwordCommand(char* password, int* userAccount);

/*Definicion de la conexión pasiva*/
char* passiveSocket(int sa);

/*Definicion del menu de la conexion pasiva*/
void menuPassive(int sa, char* string, int* userAccount);

void authCommand(int sa);

void cwdCommand(int sa,char* path);

void typeCommand (int sa);

void pwdCommand(int sa);

/*Comando LIST*/
char *listCommand(int sa);

int tuberia[2];

int main(int argc, char *argv[]){

    pipe(tuberia);

    struct sockaddr_in server , client;

    int cliente = sizeof(struct sockaddr_in);

    int s, b, l, fd, sa, bytes, on = 1, pid, no_cliente = 0;

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
    if (s < 0) fatal("Creación del socket falló");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
    if (b < 0) fatal("Bind falló");

    /* especifica el tamaño de la cola */
    l = listen(s, QUEUE_SIZE); 
    if (l < 0) fatal("Listen falló");
    /* El socket ahora está configurado y enlazado. Espera una conexión y la procesa. */
    while(2){
        puts("Servidor en espera de una conexión...\n\n");
        //fprintf(stdout, "WHILE 1");
        /* se bloquea para la solicitud de conexión */
        sa = accept(s, (struct sockaddr *)&client, (socklen_t*)&cliente); 
        no_cliente ++;
        pid = fork();
        char *bienvenida = "220 Servicio preparado para nuevo usuario.\n";
        if (sa < 0) fatal("Accept falló");
        if(pid == 0){
            //system((char*)"cd /");
            fprintf(stdout, "Conexión entrante con número de ID %d.\n\n",no_cliente);
            write(sa, bienvenida,strlen(bienvenida));
            char* aviso = "Conexión establecida.\n\n";
            //write(sa, aviso, strlen(aviso));
            // end
            while (1){

                if(read(sa, buf, BUF_SIZE)==0){
                    fprintf(stdout,"El cliente %d salió.\n\n",no_cliente);
                    return 1;
                } 
                if(menu(sa,buf,&userAccount) == 1){

                    write (sa, "close(s);",BUF_SIZE);
                    return 1;

                }
                memset(buf, 0, BUF_SIZE); // clear buffer;

            }
        }
    }
}

int menu(int sa, char* string, int* userAccount){

    char response[BUF_SIZE]="";
    char param[BUF_SIZE];

    /*Divición del string entrante en tokens con delimitadores*/
    char *option = strtok(string," ");

    if (strncmp(option,"USER",4) == 0){

        sscanf(string,"USER %s",param);
        strcpy(response,userCommand(strtok(NULL," "), userAccount));

    } else if (strncmp(option,"PASS",4) == 0) {

        sscanf(string,"PASS %s",param);
        strcpy(response,passwordCommand(strtok(NULL," "), userAccount));

    } else if (strncmp(option,"PORT",4) == 0) {

        strcpy(response,"\n200 Orden correcta.\n");

    } else if (strncmp(option,"LIST",4) == 0) {
        if(sData == 1){
            
            strcpy(response,"\n125 La conexion de datos ya esta abierta; comenzando transferencia.\n");
            listCommand(sa);
        
        }else{

            strcpy(response,"\n425 No se puede abrir la conexión de datos.\n");
            
        }

    } else if (strncmp(option,"PASV",4) == 0) {
        //if (userAuth == true && userPass == true){

            passiveSocket(sa);

        //}
        //strcpy(response,"530 No está conectado.\n");

    } else if (strncmp(option,"SYST",4) == 0) {

        strcpy(response,"\n215 NOMBRE system type. Donde NOMBRE es un nombre de sistema oficial de la lista que hay en el documento Números Asignados.\n");

    } else if (strncmp(option,"QUIT",4) == 0){
        
        return 1;
        
    }else if(strncmp(option, "FEAT",4) == 0){

        strcpy(response,"\nComandos disponibles:\n- USER\n- PASS\n- PORT\n- LIST\n- PASV\n- SYST\n");

    }else if(strncmp(option, "CWD",3) == 0){

        sscanf(string, "CWD %s", param);
        cwdCommand(sa,param);

    }else if(strncmp(option, "PWD", 3) == 0){

        pwdCommand(sa);

    }else if(strncmp(option, "AUTH",4) == 0){

        authCommand(sa);

    }else if (strncmp(option,"TYPE",4) == 0) {

        strcpy(response,"200 TYPE command success\n");
        //write(sa, response, strlen(response));

    }else if(strncmp(option, "RETR",4) == 0){

        

    }else{

        strcpy(response, "\n\nComando no reconocido.\n");

    }
    
    write(sa, response, strlen(response));
}

void fatal(char *string){
    printf("%s\n", string);
    exit(1);
}

char* userCommand(char* user,int* userAccount){
     if (user==NULL){
         return (char*)"\n332 Necesita una cuenta para entrar en el sistema.\n";
     }
     *(userAccount)=true;
     userAuth = true;
    return (char*)"\n331 Usuario OK, necesita contraseña.\n";
}

char* passwordCommand(char* password, int* userAccount){
    if(!*(userAccount)){
        return (char*)"\n332 Necesita una cuenta para entrar en el sistema.\n";
    }
    if(password == NULL || strcmp(password,STATIC_PASSWORD) != 0){
        return (char*)"\n530 No está conectado.\n";
    }
    userPass = true;
    return (char*)"\n230 Usuario conectado, continúe.\n";
}

char* passiveSocket(int sa){
    srand(time(NULL));
    char pasvResponse[BUF_SIZE];
    int passivePort = 1024 + rand() % (65535 - 1024);
    int firstPassive = passivePort / 256;
    int secondPassive = passivePort % firstPassive;
    int data_port = passivePort;
    sprintf(pasvResponse, "227 Iniciando modo pasivo (127,0,0,1,%d,%d).\n", firstPassive, secondPassive);
    puts(pasvResponse);
    char *response = (char*)pasvResponse;
    write(sa, response, strlen(response));
    int sad, b, l, fd, bytes, on = 1,sdata;
    int userAccount=false;
    char buf[BUF_SIZE]; /* búfer para el archivo saliente */
    struct sockaddr_in channel; /* contiene la dirección IP */
    /* Construye la estructura de la dirección para enlazar el socket. */
    memset(&channel, 0, sizeof(channel)); /* canal cero */
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(data_port);
    /* Apertura pasiva. Espera una conexión. */
    sad = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); /* crea el socket */
    if (sad < 0) fatal((char*)"socket failed");
    setsockopt(sad, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    b = bind(sad, (struct sockaddr *) &channel, sizeof(channel));
    if (b < 0) fatal((char*)"bind failed");
    l = listen(sad, QUEUE_SIZE); /* especifica el tamaño de la cola */
    if (l < 0) fatal((char*)"listen failed");
    int pid;
    char respuesta[BUF_SIZE];
    pid = fork();
    if (pid == 0){

        close(tuberia[1]);
        sdata = accept(sad, 0, 0); /* se bloquea para la solicitud de conexión */
        if (sdata < 0) fatal((char*)"accept failed");
        while (read (tuberia[0], respuesta, BUF_SIZE)){

            write(sdata, respuesta, strlen(respuesta));
            
        }
        

    }else{

        sData = true;
        close(tuberia[0]);

    }

    //char* respuestaFinal = ("Confirmación pasivo");
    //write(sdata, respuestaFinal, strlen(respuestaFinal));
}

char *listCommand(int sa){
    int bytes,fd;
    char* buf[BUF_SIZE];
    system((char*)"ls -l > lsCommand.txt");
    char *respuesta = "150 Estado del fichero correcto; va a abrirse la conexión de datos.";
    write(sa, respuesta, strlen(respuesta));

    /* abre el archivo para regresarlo */
    fd = open((char*)"lsCommand.txt", O_RDONLY); 
    if (fd < 0) fatal((char*)"open failed");
    while (1) {
        /* lee del archivo */
        bytes = read(fd, buf, BUF_SIZE);
        /* verifica el final del archivo */
        if (bytes <= 0) break;
        /* describe bytes en el socket */
        write(tuberia[1], buf, bytes); 
    }

    char *response = "226 Cerrando la conexión de datos.";
    write(sa, response, strlen(response));

}

void authCommand(int sa){
    char respuesta[BUF_SIZE] = "Este servidor no soporta crifrado TLS";
    write(sa, respuesta, strlen(respuesta));
}

void pwdCommand(int sa){
    int bytes,fd;
    char buf[BUF_SIZE];
    char output[BUF_SIZE]="257 ";
    system((char*)"pwd > pwdCommand.txt");
    fd = open((char*)"pwdCommand.txt", O_RDONLY); /* abre el archivo para regresarlo */
    if (fd < 0) fatal((char*)"open failed");
    while (1) {
        bytes = read(fd, buf, BUF_SIZE); /* lee del archivo */
        if (bytes <= 0) break; /* verifica el final del archivo */
        strcat(output,buf);
        write(sa, output, strlen(output)); /* cdescribe bytes en el socket */
        memset(buf,0,BUF_SIZE);
    }
}

void typeCommand(int sa){

    char respuesta[BUF_SIZE] = "200 Orden correcta.";
    write(sa, respuesta, strlen(respuesta));

}

void cwdCommand(int sa,char* path){
    int bytes,fd;
    char CurrentDirectory[BUF_SIZE];
    char root[34] = "/home/jipixz/Documentos/FTP-Server";
    int handler = chdir(path);
    if (handler != 0){
        write(sa, "501 Error de sintaxis en parámetros o argumentos.\r\n", 53); /* cdescribe bytes en el socket */
    } else {
        if (getcwd(CurrentDirectory, sizeof(CurrentDirectory)) != NULL) {
            if(strncmp(root,CurrentDirectory,28)==0){
                write(sa, "200 Orden correcta.\r\n", 22); /* cdescribe bytes en el socket */
            } else {
                chdir(root);
                write(sa, "500 Permiso denegado\r\n", 23); /* cdescribe bytes en el socket */
            }
        }
    }
}