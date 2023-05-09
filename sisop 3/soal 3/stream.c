#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <json-c/json.h>

#define PERMISSION 0666
#define MESSAGE_SIZE 100

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

char* rot13(char* str) {
    char* p = str;
    while (*p != '\0') {
        if ((*p >= 'A' && *p <= 'M') || (*p >= 'a' && *p <= 'm')) {
            *p += 13;
        } else if ((*p >= 'N' && *p <= 'Z') || (*p >= 'n' && *p <= 'z')) {
            *p -= 13;
        }
        ++p;
    }
    return str;
}

char* base64(char* str) {
    char* p = str;
    while (*p != '\0') {
        if (*p == ' ') {
            *p = '+';
        } else if (*p == '-') {
            *p = '/';
        } else if (*p == '_') {
            *p = '=';
        }
        ++p;
    }
    return str;
}

char* hex(char* str) {
    char* p = str;
    while (*p != '\0') {
        if (*p == ' ') {
            *p = '+';
        } else if (*p == '-') {
            *p = '/';
        } else if (*p == '_') {
            *p = '=';
        }
        ++p;
    }
    return str;
}

void decryptLoadJSON() {
  FILE *fp = fopen("song_playlist.json", "r");
  char buffer[1024];
  size_t len = sizeof(buffer);

  if(fp == NULL) {
    printf("Error opening file\n");
    exit(1);
  }

  struct json_object *parsed_json;
  struct json_object *method;
  struct json_object *title;

  while(fgets(buffer, len, fp) != NULL) {
    parsed_json = json_tokener_parse(buffer);

    json_object_object_get_ex(parsed_json, "method", &method);
    json_object_object_get_ex(parsed_json, "title", &title);

    const char *method_str = json_object_get_string(method);
    char *title_str = json_object_get_string(title);

    if(strcmp(method_str, "rot13") == 0) {
      rot13(title_str);
    } else if(strcmp(method_str, "base64") == 0) {
      base64(title_str);
    } else if(strcmp(method_str, "hex") == 0) {
      hex(title_str);
    }

    FILE *playlist = fopen("song_playlist.txt", "a");
    fprintf(playlist, "%s\n", title_str);
    fclose(playlist);
  }

  fclose(fp);
}

void showPlaylist() {
  FILE *fp = fopen("song_playlist.txt", "r");
  char buffer[1024];
  int count = 0;

  if(fp == NULL) {
    printf("Error opening file\n");
    exit(1);
  }

  while(fgets(buffer, sizeof(buffer), fp) != NULL) {
    char song[MESSAGE_SIZE];
    if(sscanf(buffer, "%s", song) != 1) {
      continue;
    }
    printf("%d - %s\n", ++count, song);
  }

  fclose(fp);
}

void handlePlaySong(char *command) {
  FILE *fp;
  char buffer[1024];
  int matchCount = 0;

  fp = fopen("song_playlist.txt", "r");
  if(fp == NULL) {
    printf("Error opening file\n");
    exit(1);
  }

  // Search for matching song
  while(fgets(buffer, sizeof(buffer), fp) != NULL) {
    char song[MESSAGE_SIZE];
    if(sscanf(buffer, "%s", song) != 1) {
      continue;
    }
    if(strstr(song, command) != NULL) {
      matchCount++;
      printf("%d. USER PLAYING \"%s\"", matchCount, song);
    }
  }

  if(matchCount == 0) {
    printf("THERE IS NO SONG CONTAINING \"%s\"\n", command);
  } else {
    printf("THERE ARE \"%d\" SONG CONTAINING \"%s\"\n", matchCount, command);
  }

  fclose(fp);
}

int main(void) {
  struct my_msgbuf buf;
  int msqid, len;
  key_t key;

  if ((key = ftok("song_playlist.txt", 'B')) == -1) {
    perror("ftok");
    exit(1);
  }
  if ((msqid = msgget(key, PERMISSION | IPC_CREAT)) == -1) {
    perror("msgget");
    exit(1);
  }
  printf("message queue: ready to send messages.\n");
  printf("Enter lines of text, ^D to quit:\n");
  buf.mtype = 1;

  while(fgets(buf.mtext, sizeof buf.mtext, stdin)) {
    len = strlen(buf.mtext) - 1;
    if (buf.mtext[len] == '\n')
      buf.mtext[len] = '\0';
    if (msgsnd(msqid, &buf, len+1, 0) == -1)
      perror("msgsnd");

    if (!strcmp(buf.mtext, "DECRYPT"))
      decryptLoadJSON();
    else if (!strcmp(buf.mtext, "PLAY")) {
      showPlaylist();
      char command[100];
      printf("Pilih lagu yang ingin diputar: ");
      scanf("%s", command);
      handlePlay(command);
    }
  }
  strcpy(buf.mtext, "end");
  len = strlen(buf.mtext);
  if (msgsnd(msqid, &buf, len+1, 0) == -1)
    perror("msgsnd");

  if (msgctl(msqid, IPC_RMID, NULL) == -1) {
    perror("msgctl");
    exit(1);
  }
  printf("Message queue: Pesan telah berhasil disampaikan.\n");
  return 0;
}
