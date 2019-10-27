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
#include <time.h>

/* arbitrario, pero el cliente y el servidor deben coincidir */
#define SERVER_PORT 21

/* tamaño de bloque para transferencia */ 
#define BUF_SIZE 4096 

void fatal(char *string);

/*Se definen los comandos en conexión pasiva*/
void pasvCommand(char* string);

/*Definición de la función de conexión pasiva*/
void passiveConnection(char* host, char* port);

int main(int argc, char **argv){

    srand(time(NULL));

    int c, s, bytes;

    char command[BUF_SIZE];

    /* búfer para el archivo entrante */
    char buf[BUF_SIZE]; 
    char copy_buf[BUF_SIZE];
    char commandBuf[BUF_SIZE];

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
        fatal("Creación del socket falló");
    } 

    
    memset(&channel, 0, sizeof(channel));
    channel.sin_family= AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port= htons(SERVER_PORT);
    c = connect(s, (struct sockaddr *) &channel, sizeof(channel));
    
    if (c < 0){
        fatal("Connect falló");
    } 
    read(s, buf, BUF_SIZE); 
    fprintf(stdout,"%s",buf);
    
    while (1){
        printf("\nIngrese comando: ");
        /*Lee desde el teclado*/
        fgets(command, sizeof(command), stdin);

        /*Escribe en el socket*/
        write(s,command,strlen(command)-1);

        /* lee del socket */
        bytes = read(s, buf, BUF_SIZE); 
        strcpy(copy_buf,buf);
        strcpy(commandBuf, buf);

        // keep reading
        if (bytes <= 0){
            /* verifica el final del archivo */
             exit(0); 
        }
         
        
        if(strcmp(strtok(copy_buf, " "),"227") == 0){
            //puts("Entra a la comparación con 227\n");
            //fprintf(stdout, "%s", buf);
            pasvCommand(commandBuf);
            
        } else if (strcmp(buf, "close(s);") == 0){
            puts("221 Hasta luego.");
            close(s);
            break;
        /* escribe en la salida estándar */
        }
        write(1, buf, bytes); 
    }
}

void fatal(char *string){
    
    printf("%s\n", string);
    exit(1);

}

void pasvCommand(char* string){
    //puts("Entra a la funcion pasvCommand\n");
    /* obtiene la primera parte del comando*/
    char *option = strtok(string,"(");
    char *ip = strtok(NULL, " ");
    char *puerto = strtok(NULL, ")");
    if(strcmp(option,"227 Iniciando modo pasivo ")==0){

        //Puerto del host
        //write(1,ip,strlen(ip));
        //write(1,puerto,strlen(puerto));
        passiveConnection(puerto, ip);
    }
}

void passiveConnection(char* port, char* host){
    //puts("\nEntra a la funcion passiveConnection\n");
    int c, sdata, bytes;

    /*Imprime la dirección de la conexión pasiva*/
    /*Imprime el puerto de la conexión pasiva*/

    char command[BUF_SIZE];

    /*Bufer para archivo entrante*/
    char buf [BUF_SIZE];

    /*Información sobre el servidor*/
    struct hostent *h;

    /*Contiene la dirección IP del hwrite(1, buf, bytes);ost*/
    struct sockaddr_in channel;

    /*Obtiene direccion IP del host*/
    h = gethostbyname(host);
    if(!h){
        fatal ("gethostbyname fallo");
    }

    sdata = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sdata < 0){
        fatal ((char*)"socket");
    } 
    memset(&channel, 0, sizeof(channel));
    channel.sin_family= AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port= htons(atoi(port));
    //puts("boom BITCH");

    c = connect(sdata, (struct sockaddr *) &channel, sizeof(channel));
    if (c < 0){
        fatal ("falló la conexión");
    }
    //read(sdata, buf, BUF_SIZE);
}