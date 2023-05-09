#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define PERMISSION 0666
#define MAX_TEXT_SIZE 200

struct messageBuffer {
   long mtype;
   char mtext[MAX_TEXT_SIZE];
};

int main(void) {
   struct messageBuffer buf;
   int msqid;
   key_t key;
   
   if ((key = ftok("song_playlist.txt", 'B')) == -1) {
      perror("ftok");
      exit(1);
   }
   
   if ((msqid = msgget(key, PERMISSION)) == -1) {
      perror("Message queue tidak ditemukan");
      exit(1);
   }
   printf("Message queue: Siap menerima message.\n");
   
   while (msgrcv(msqid, &buf, MAX_TEXT_SIZE, 0, 0) != -1) {
      if(strcmp(buf.mtext,"DECRYPT")==0){
         printf("recvd: \"%s\"\n", buf.mtext);
         if (strcmp(buf.mtext,"end") == 0)
            break;
      }
   }
   printf("Message queue: Selesai menerima message.\n");
   system("rm song_playlist.txt");
   return 0;
}
