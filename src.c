#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{   

    char usr[100];
    strcpy(usr,argv[1]);
    
    FILE *fp = fopen("config.txt", "r");
    if (fp == NULL)
    {
        printf("Error while opening file: %d \n", errno);
    }
    char u[100];
    char q[100];
    char key[100]; 

    while(fscanf(fp, "%s %s", u, q) != EOF){
        if(strcmp(u, usr) == 0){
            strcpy(key,q);
            break;
        }
    }
    printf("my key: %s \n", key); //get key from config file
    close(fp);
    if (mkfifo(key, 0666) == -1) //create main program fifo
    {   if (errno != EEXIST){
            perror("Error while opening main program fifo  \n");
            return 1;
        }
    }
 
    pid_t pid = fork(); 
    if (pid == 0)
    {
        char str[100];
        char second_usr[100];
        char command[100];
        char second_usr_fifo[100];
        printf("Enter second user name, command and fifo name: \n");
        scanf("%[^\n]s", str);
        char *token = strtok(str, " \"");
        char array[3][100];
        
        int i = 0;
        while (token != NULL)  //parse input
        {
            strcpy(array[i], token);
            token = strtok(NULL, " \"");
            i++;
        }
        char keyy[100];
        char uu[100];
        char qq[100];
        FILE *fpp = fopen("config.txt", "r");
        if (fpp == NULL)
        {
            printf("Error while opening file: %d \n", errno);
        }
        while (fscanf(fpp, "%s %s", uu, qq) != EOF)
        {  
            if (strcmp(uu, array[0]) == 0)
            {
                strcpy(keyy,qq);
                break;
            }
        }
        close(fpp);
        printf("key to send: %s \n", keyy);
        int fifo_to_second_user = open(keyy, O_WRONLY); //open fifo to send
        
        if (fifo_to_second_user == -1)
        {
            printf("Error with fifo to second usr: %d\n", errno);
        }
        strcat(array[1], " ");
        strcat(array[1], array[2]);
        if (mkfifo(array[2], 0666)==-1){
            if (errno != EEXIST){
                perror("Error while opening fifo to recive \n");
                return 1;
            }
        }
        if(write(fifo_to_second_user,array[1], sizeof(array[1]) ) == -1){
            printf("Error while sending to doe: %d\n", errno);
        }

        int returnFifo = open(array[2], O_RDONLY);
        char buf[100];
        printf("opened return fifo here is result\n");
        while (read(returnFifo, buf, 100) != 0)
        {
            printf("%s", buf);
        }
        close(returnFifo);
    }
    else
    { //doer
        char buff[100];
        int myfifo = open(key, O_RDONLY);
        if(read(myfifo, buff, 100) == -1){
            printf("Error while reading : %d\n", errno);
        }
        char command[100];
        char send_key[100];
        char* token = strtok(buff, " ");
        strcpy(command, token);
        token = strtok(NULL, " ");
        strcpy(send_key, token);
        printf("command: %s \n", command);
        printf("fifo to send: %s \n", send_key);
        int fifo_to_send = open(send_key, O_WRONLY);
        if(fifo_to_send == -1){
            printf("Error: %d \n", errno);
        }
        FILE *fp = popen(command, "r");
        char buf[100];
        while (fgets(buf, 100, fp) != NULL)
        {
            if(write(fifo_to_send, buf, sizeof(buf)) == -1){
                printf("Error: %d \n", errno);
            }
        }
        close(fifo_to_send);
    }
    wait(NULL);
}
