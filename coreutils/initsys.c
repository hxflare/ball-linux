#include "../btools.h"
#include <bits/pthreadtypes.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef enum {
  shell = 0,
  execute = 1,
  print = 3,
  wait = 4,
} ServiceType;
typedef struct {
  char name[64];
  char after[16][64];
  int after_count;
  char before[16][64];
  int before_count;
  char functional[256];
  char type[32];
} Service;
Service parse_service(char *service_name) {
  Service service;
  strncpy(service.name, service_name, 64);
  char *path = concat("/etc/initsys.d/", service_name);
  FILE *rc = fopen(path, "r");
  char fline[1024];
  char lines[1024][1024];
  int amm = 0;
  while (fgets(fline, sizeof(fline), rc) != NULL) {
    strncpy(lines[amm], fline, 1024);
    lines[amm][strcspn(lines[amm], "\n")] = 0;
    amm++;
  }
  int cur_line = 0;
  while (cur_line < amm) {
    if (lines[cur_line][0] == '!') {
      if (strcmp(lines[cur_line] + 1, "after") == 0) {
        int count = 0;
        while (lines[cur_line][0] != '!') {
          if (lines[cur_line][0] == '@') {
            strncpy(service.after[count], lines[cur_line] + 1, 64);
            count++;
          }
          cur_line++;
        }
        service.after_count = count;
      } else if (strcmp(lines[cur_line] + 1, "before") == 0) {
        int count = 0;
        while (lines[cur_line][0] != '!') {
          if (lines[cur_line][0] == '@') {
            strncpy(service.before[count], lines[cur_line] + 1, 64);
          }
          count++;
          cur_line++;
        }
        service.before_count = count;
      } else if (strcmp(lines[cur_line] + 1, "type") == 0) {
        while (lines[cur_line][0] != '!') {
          if (lines[cur_line][0] == '@') {
            strncpy(service.type, lines[cur_line] + 1, 32);

            break;
          }
          cur_line++;
        }
      } else if (strcmp(lines[cur_line] + 1, "functional") == 0) {
        while (cur_line < amm) {
          cur_line++;
        }
      }
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
  cprint(service.functional);
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
    if (i < service.before_count - 1)
      cprint(", ");
  }
  cprint("\n");
}
void execute_service(Service service) {}
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
  int ammount = 0;
  service_root = malloc(sizeof(Service) * (ammount + 1));
  while ((entry = readdir(initsys_dir)) != NULL) {
    char *name = entry->d_name;
    unsigned char type = entry->d_type;
    if (type == DT_REG) {
      service_root[ammount] = parse_service(name);
      print_service(service_root[ammount]);
    } else {
      cprint("Only regular files are parsed as services. ");
      cprint(name);
      cprint(" is not a service.\n");
    }
    ammount++;
    service_root = realloc(service_root, sizeof(Service) * (ammount + 1));
  }
}