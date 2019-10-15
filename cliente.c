/* 
Esta página contiene un programa cliente que puede solicitar un archivo
desde el programa servidor de la siguiente página. El servidor responde
enviando el archivo completo.
*/ 

#include <sys/types.h>
#include <linux/socket.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* arbitrario, pero el cliente y el servidor deben coincidir */
#define SERVER_PORT 12345

/* tamaño de bloque para transferencia */ 
#define BUF_SIZE 4096 

void fatal(char *string);

/*Se definen los comandos en conexión pasiva*/
void clientActions(char* string);

/*Definición de la función de conexión pasiva*/
void passiveConnection(char* host, char* port);

int main(int argc, char **argv){

    int c, s, bytes;
    char command[BUF_SIZE];

    /* búfer para el archivo entrante */
    char buf[BUF_SIZE]; 

    /* información sobre el servidor */
    struct hostent *h; 

    /* contiene la dirección IP */
    struct sockaddr_in channel; 
    if (argc != 2){
        fatal("Uso: cliente nombre-de-servidor");
    }

    /* busca la dirección IP del host */
    h = gethostbyname(argv[1]); 
    if (!h){
        fatal("gethostbyname falló");
    }

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s < 0){
        fatal("socket");
    } 

    memset(&channel, 0, sizeof(channel));
    channel.sin_family= AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port= htons(SERVER_PORT);
    c = connect(s, (struct sockaddr *) &channel, sizeof(channel));
    
    if (c < 0){
        fatal("connect failed");
    } 
    read(s, buf, BUF_SIZE); 
    fprintf(stdout,"%s",buf);
    /* 
    Se ha establecido la conexión. Se envía el nombre del archivo incluyendo el byte 0 al final.
    write(s, argv[2], strlen(argv[2])+1);
    Obtiene el archivo y lo escribe en la salida estándar. 
    */
    
    while (1){
        /*Lee desde el teclado*/
        fgets(command, sizeof(command), stdin);

        /*Escribe en el socket*/
        write(s,command,strlen(command)-1);

        /* lee del socket */
        bytes = read(s, buf, BUF_SIZE); 
        // keep reading
        if (bytes <= 0){
            /* verifica el final del archivo */
             exit(0); 
        } else if (strcmp(buf, "close(s);")==0){
            close(s);
            break;
        }
        /* escribe en la salida estándar */
        write(1, buf, bytes); 
        clientActions(buf);
    }
}

void fatal(char *string){
    
    printf("%s\n", string);
    exit(1);

}

void clientActions(char* string){
    /* obtiene la primera parte del comando*/
    char *option = strtok(string," ");

    if(strcmp(option,"passiveON")==0){
        //Puerto del host
        passiveConnection(strtok(NULL," "), strtok(NULL," "));
    }
}

void passiveConnection(char* port, char* host){
    int c, s, bytes;

    /*Imprime la dirección de la conexión pasiva*/
    printf("HOST %s",host);

    /*Imprime el puerto de la conexión pasiva*/
    printf("PORT %s", port);

    char command[BUF_SIZE];

    /*Bufer para archivo entrante*/
    char buf [BUF_SIZE];

    /*Información sobre el servidor*/
    struct hostent *h;

    /*Contiene la dirección IP del host*/
    struct sockaddr_in channel;

    /*Obtiene direccion IP del host*/
    h = gethostbyname(host);
    if(!h){
        fatal ("gethostbyname fallo");
    }

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0){
        fatal ((char*)"socket");
    } 
    memset(&channel, 0, sizeof(channel));
    channel.sin_family= AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port= htons(atoi(port));
    c = connect(s, (struct sockaddr *) &channel, sizeof(channel));
    if (c < 0){
        fatal ("falló la conexión");
    } 

    /* 
    Se ha establecido la conexión. Se envía el nombre del archivo incluyendo
    el byte 0 al final. 
    write(s, argv[2], strlen(argv[2])+1);
    Obtiene el archivo y lo escribe en la salida estándar. 
    TuT
    */


   while (1) {
        
        fgets(command, sizeof(command), stdin);
        write(s,command,strlen(command)-1);

        /* lee del socket */
        bytes = read(s, buf, BUF_SIZE); 
        
        // keep reading
        if (bytes <= 0){
            /* verifica el final del archivo */
            exit(0); 
        } 
        /* escribe en la salida estándar */
        write(1, buf, bytes); 
        //close(s);
    }
}