#include "../btools.h"
#include <bits/pthreadtypes.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
typedef struct {
  char name[64];
  char after[16][64];
  int after_count;
  char before[16][64];
  int before_count;
  char functional[256][256];
  char type[32];
} Service;
Service parse_service(char *service_name) {
  Service service;
  memset(&service, 0, sizeof(Service));
  strncpy(service.name, service_name, 64);
  char *path = concat("/etc/initsys.d/", service_name);
  FILE *rc = fopen(path, "r");
  if (rc == NULL) {
    cprint("Failed to open service file\n");
    return service;
  }
  char fline[1024];
  char lines[1024][1024];
  int amm = 0;
  while (fgets(fline, sizeof(fline), rc) != NULL) {
    strncpy(lines[amm], fline, 1024);
    lines[amm][strcspn(lines[amm], "\n")] = 0;
    amm++;
  }
  fclose(rc);
  int cur_line = 0;
  while (cur_line < amm) {
    if (lines[cur_line][0] == '!') {
      if (strcmp(lines[cur_line] + 1, "after") == 0) {
        int count = 0;
        cur_line++;
        while (cur_line < amm && lines[cur_line][0] != '!') {
          if (lines[cur_line][0] == '@') {
            strncpy(service.after[count], lines[cur_line] + 1, 64);
            count++;
          }
          cur_line++;
        }
        service.after_count = count;
      } else if (strcmp(lines[cur_line] + 1, "before") == 0) {
        int count = 0;
        cur_line++;
        while (cur_line < amm && lines[cur_line][0] != '!') {
          if (lines[cur_line][0] == '@') {
            strncpy(service.before[count], lines[cur_line] + 1, 64);
            count++;
          }
          cur_line++;
        }
        service.before_count = count;
      } else if (strcmp(lines[cur_line] + 1, "type") == 0) {
        cur_line++;
        while (cur_line < amm && lines[cur_line][0] != '!') {
          if (lines[cur_line][0] == '@') {
            strncpy(service.type, lines[cur_line] + 1, 32);
            cur_line++;
            break;
          }
          cur_line++;
        }
      } else if (strcmp(lines[cur_line] + 1, "functional") == 0) {
        cur_line++;
        int count = 0;
        while (cur_line < amm && lines[cur_line][0] != '!') {
          strncpy(service.functional[count], lines[cur_line], 256);
          count++;
          cur_line++;
        }

      } else {
        cur_line++;
      }
    } else {
      cur_line++;
    }
  }
  return service;
}
void print_service(Service service) {
  cprint("name: ");
  cprint(service.name);
  cprint("\n");
  cprint("type: ");
  cprint(service.type);
  cprint("\n");
  cprint("functional: ");
  int i = 0;
  while (service.functional[i][0] != '\0') {
    cprint(service.functional[i]);
    cprint("\n");
    i++;
  }
  cprint("\n");
  cprint("after: ");
  for (int i = 0; i < service.after_count; i++) {
    cprint(service.after[i]);
    if (i < service.after_count - 1)
      cprint(", ");
  }
  cprint("\n");
  cprint("before: ");
  for (int i = 0; i < service.before_count; i++) {
    cprint(service.before[i]);
    if (i < service.before_count - 1) {
      cprint(", ");
    }
  }

  cprint("\n");
}
void execute_service(Service *service) {
  if (strcmp(service->type, "execute") == 0) {
    pid_t fork_pid;
    extern char **environ;
    int i = 0;
    while (service->functional[i][0] != '\0') {
      fork_pid = fork();

      if (fork_pid == 0) {
        char *args[] = {service->functional[i], NULL};
        execve(service->functional[i], args, environ);
        perror("execve failed");
        exit(EXIT_FAILURE);
      } else if (fork_pid == -1) {
        cprint("fork failed\n");
      } else {
        int status;
        waitpid(fork_pid, &status, 0);
      }
      i++;
    }
  } else if (strcmp(service->type, "write")==0) {
    int i = 0;
    while (service->functional[i][0] != '\0') {
      cprint(service->functional[i]);
      i++;
    }
  }else if(strcmp(service->type, "command")==0){
    pid_t fork_pid;
    extern char **environ;
    int i = 0;
    while (service->functional[i][0] != '\0') {
      fork_pid = fork();

      if (fork_pid == 0) {
        char *func=concat(concat("\"", service->functional[i]),"\"");
        char *args[] = {"ball","-c",func, NULL};
        execve(service->functional[i], args, environ);
        perror("execve failed");
        exit(EXIT_FAILURE);
      } else if (fork_pid == -1) {
        cprint("fork failed\n");
      } else {
        int status;
        waitpid(fork_pid, &status, 0);
      }
      i++;
    }
  }
}
int main(int argc, char **argv) {
  cprint("initsys starting. getting services\n");
  Service *service_root;
  DIR *initsys_dir;
  struct dirent *entry;
  char *path = "/etc/initsys.d";
  initsys_dir = opendir(path);
  if (initsys_dir == NULL) {
    cprint("Unable to open /etc/initsys.d\n");
    exit(EXIT_FAILURE);
  }
  int amount = 0;
  service_root = malloc(sizeof(Service) * (amount + 1));
  while ((entry = readdir(initsys_dir)) != NULL) {
    char *name = entry->d_name;
    unsigned char type = entry->d_type;
    if (type == DT_REG) {
      cprint("service found: ");
      cprint(name);
      cprint("\n");
      service_root[amount] = parse_service(name);
      execute_service(&service_root[amount]);
      amount++;
      service_root = realloc(service_root, sizeof(Service) * (amount + 1));
    } else if (name[0] != '.') {
      cprint("Only regular files are parsed as services. ");
      cprint(name);
      cprint(" is not a service.\n");
    }
  }
}