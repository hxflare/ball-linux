#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;
void enable_term_rawmode(){
  tcgetattr(STDIN_FILENO, &orig_termios);
  struct termios raw=orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  raw.c_cc[VMIN]=1;
  raw.c_cc[VTIME]=0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
typedef enum exec_types {
  normal = 0,
  piped_out_of = 1,
  overwrite_file = 2,
  append_to_file = 3,
  or = 4,
  background = 5,
  and = 6
} exec_types;
typedef struct exec_batch {
  char *command;
  exec_types type;
} exec_batch;
char *str_replace(char *orig, char *rep, char *with) {
  char *result;
  char *ins;
  char *tmp;
  int len_rep;
  int len_with;
  int len_front;
  int count;
  if (!orig || !rep)
    return NULL;
  len_rep = strlen(rep);
  if (len_rep == 0)
    return NULL;
  if (!with)
    with = "";
  len_with = strlen(with);
  ins = orig;
  for (count = 0; (tmp = strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;
  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep;
  }
  strcpy(tmp, orig);
  return result;
}
void cprint(char *string) { write(1, string, strlen(string)); }
typedef struct shellConf {
  char aliases[255][255];
  char meanings[255][255];
  char PISS[255];
  char paths[255][255];
} shellConf;
void print_strlist(char **array) {
  for (int i = 0; array[i] != NULL; i++) {
    printf("%s\n", array[i]);
  }
}
char **extract_args(char *command, shellConf config) {
  int raw_c = 1;
  int command_len = strlen(command);
  for (int i = 1; i < command_len; i++) {
    if (command[i] == ' ' && command[i - 1] != ' ') {
      raw_c++;
    }
  }
  char **args = malloc((raw_c + 1) * sizeof(char *));
  {
    int arg_index = 0;
    int i = 0;
    while (i < command_len) {
      while (i < command_len && command[i] == ' ')
        i++;
      if (i >= command_len)
        break;
      char cur_arg[256];
      int cur_len = 0;
      if (command[i] == '"') {
        i++;
        while (i < command_len && command[i] != '"') {
          cur_arg[cur_len++] = command[i++];
        }
        if (i < command_len)
          i++;
      } else {
        while (i < command_len && command[i] != ' ') {
          if (command[i] == '~') {
            i++;
            char *username = getlogin();
            char homepath[256];
            snprintf(homepath, sizeof(homepath), "/home/%s", username);
            for (int p = 0; p < (int)strlen(homepath); p++) {
              cur_arg[cur_len++] = homepath[p];
            }
          }
          cur_arg[cur_len++] = command[i++];
        }
      }
      cur_arg[cur_len] = '\0';
      args[arg_index++] = strdup(cur_arg);
    }
    args[arg_index] = NULL;
  }
  char **aliased = malloc((raw_c * 16 + 1) * sizeof(char *));
  {
    int arg_index = 0;
    int i = 0;
    while (args[i] != NULL) {
      int j = 0;
      while (config.aliases[j][0] != '\0') {
        char *replaced =
            str_replace(args[i], config.aliases[j], config.meanings[j]);
        if (replaced != NULL) {
          free(args[i]);
          args[i] = replaced;
        }
        j++;
      }
      char *cur = args[i];
      int cur_len_total = strlen(cur);
      i++;
      int k = 0;
      while (k < cur_len_total) {
        char cur_arg[256];
        int cur_len = 0;
        while (k < cur_len_total && cur[k] == ' ') {
          k++;
        }
        if (k >= cur_len_total)
          break;
        if (cur[k] == '"') {
          k++;
          while (k < cur_len_total && cur[k] != '"') {
            cur_arg[cur_len++] = cur[k++];
          }
          if (k < cur_len_total)
            k++;
        } else {
          while (k < cur_len_total && cur[k] != ' ') {
            if (cur[k] == '~') {
              k++;
              char *username = getlogin();
              char homepath[256];
              snprintf(homepath, sizeof(homepath), "/home/%s", username);
              for (int p = 0; p < (int)strlen(homepath); p++) {
                cur_arg[cur_len++] = homepath[p];
              }
            }
            cur_arg[cur_len++] = cur[k++];
          }
        }
        cur_arg[cur_len] = '\0';
        aliased[arg_index++] = strdup(cur_arg);
      }
    }
    aliased[arg_index] = NULL;
    free(args);
    return aliased;
  }
}
exec_batch *get_exec_order(char *raw) {
  int len = strlen(raw);
  char *cur_command = malloc(256);
  int cur_len = 0;
  int del_amount = 0;
  for (int i = 0; i < len; i++) {
    if (raw[i] == ';' || raw[i] == '|' || raw[i] == '&' || raw[i] == '>' ||
        raw[i] == '^' || raw[i] == '@' || raw[i] == ':') {
      del_amount++;
    }
  }
  exec_batch *order = malloc(sizeof(exec_batch) * (del_amount + 1));
  int i = 0;
  int quoted = 0;
  int index = 0;
  while (raw[i] != '\0') {
    if (raw[i] == '"') {
      if (quoted==1)
        quoted = 0;
      if (quoted==0)
        quoted = 1;
    }
    if ((raw[i] == ';' || raw[i] == '|' || raw[i] == '&' || raw[i] == '>' ||
        raw[i] == '^' || raw[i] == '@' || raw[i] == ':')&&quoted==0) {
      
      if (cur_len > 0) {
        cur_command[cur_len] = '\0';
        cur_len = 0;
        order[index].command = strdup(cur_command);
        switch (raw[i]) {
        case ';':
          order[index].type = normal;
          break;
        case '|':
          order[index].type = piped_out_of;
          break;
        case '^':
          order[index].type = overwrite_file;
          break;
        case '>':
          order[index].type = append_to_file;
          break;
        case '&':
          order[index].type = background;
          break;
        case '@':
          order[index].type = and;
          break;
        case ':':
          order[index].type = or;
          break;
        default:
          order[index].type = normal;
          break;
        }
        i++;
        index++;
      }

    } else {
      cur_command[cur_len] = raw[i];
      cur_len++;
    }
    i++;
  }
  if (cur_len > 0) {
    cur_command[cur_len] = '\0';
    order[index].command = strdup(cur_command);
    order[index].type = normal;
  }
  free(cur_command);
  return order;
}
char *formatPISS(shellConf config) {
  int normal_len = strlen(config.PISS);
  char *prompt = malloc(normal_len * 2);
  char *PISS = config.PISS;
  int cur_char_i = 0;
  for (int i = 0; i < normal_len; i++) {
    if (PISS[i] != '%') {
      prompt[cur_char_i] = PISS[i];
      cur_char_i++;
    } else {
      char escape_ident = PISS[i + 1];
      i += 1;
      char *insert_str;
      switch (escape_ident) {
      case 'n':
        insert_str = "\n";
        break;
      case 'u':
        insert_str = getlogin();
        break;
      case 'p':
        char *cur_dir = getcwd(NULL, 0);
        char *usern = getlogin();
        char homedir[256];
        snprintf(homedir, 256, "/home/%s", usern);
        insert_str = str_replace(cur_dir, homedir, "~");
        break;
      case 'P':
        insert_str = getcwd(NULL, 0);
        break;
      case 'h':
        insert_str = malloc(256);
        gethostname(insert_str, 256);
        break;
      default:
        insert_str = "";
        break;
      }
      int insert_len = strlen(insert_str);
      for (int k = 0; k < insert_len; k++) {
        prompt[cur_char_i] = insert_str[k];
        cur_char_i++;
      }
    }
  }
  prompt[cur_char_i] = '\0';
  return prompt;
}
int execute(char mode, char *execd, shellConf config) {
  switch (mode) {
  case 'c':
    break;
  case 'f':
    break;
  default:
    cprint("invalid mode");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
shellConf getConf(FILE *rc) {
  char line[1024];
  char loaded[90][1024];
  int ammount = 0;
  shellConf config = {0};
  while (fgets(line, sizeof(line), rc) != NULL) {
    strcpy(loaded[ammount], line);
    ammount++;
  }
  for (int i = 0; i < ammount; i++) {
    if (loaded[i][0] == '!') {
      char keyword[1024];
      char values[50][255];
      int valueCount = 0;
      loaded[i][strcspn(loaded[i], "\n")] = 0;
      strcpy(keyword, loaded[i]);
      for (int j = i + 1; j < ammount; j++) {
        if (loaded[j][0] == '@') {
          strcpy(values[valueCount], loaded[j]);
          valueCount++;
        } else if (loaded[j][0] == '!') {
          break;
        }
      }
      if (strcmp(keyword, "!PISS") == 0) {
        char *format = values[0] + 1;
        format[strcspn(format, "\n")] = 0;
        strcpy(config.PISS, format);
      } else if (strcmp(keyword, "!ALIAS") == 0) {
        for (int i = 0; i < valueCount; i++) {
          char *format = values[i] + 1;
          format[strcspn(format, "\n")] = 0;
          char *eq = strchr(format, '=');
          if (!eq)
            continue;
          *eq = '\0';
          strcpy(config.aliases[i], format);
          strcpy(config.meanings[i], eq + 1);
        }
      } else if (strcmp(keyword, "!PATH") == 0) {
        for (int i = 0; i < valueCount; i++) {
          char *format = values[i] + 1;
          format[strcspn(format, "\n")] = 0;
          strcpy(config.paths[i], format);
        }
      }
    }
  }
  return config;
}
void loop() {}
int main(int argc, char **argv) {
  FILE *rcfile = fopen(".ballrc", "r");
  shellConf conf;
  if (rcfile == NULL) {
    rcfile = fopen(".ballrc", "w");
    fprintf(rcfile,
            "The ball shell configuration file.\n  You should put a newline "
            "without any spaces after if you want to declare a keyword or a "
            "meaning or a startup commands(must be put last).\n  Keyword start "
            "with !, values start with @, startup commands start with $.\n  "
            "string modifiers for PISS: %%n - newline %%u - user %%h - "
            "hostname %%p - current path %%P - current full path\n  Colors - "
            "use ascii color codes "
            "\n\n \n!PISS\n@%%u@%%h  %%p %%n#   "
            "\n!ALIAS\n\n!PATH\n@/bin\n@/usr/bin\n@/usr/local/bin\n@/sbin\n@/"
            "usr/sbin\n");
    fclose(rcfile);
    rcfile = fopen(".ballrc", "r");
    conf = getConf(rcfile);
    fclose(rcfile);
  } else {
    conf = getConf(rcfile);
    fclose(rcfile);
  }
  exec_batch *order = get_exec_order("f; cprint ass| gay \"homori^ ass\"");
  int i = 0;
  while (1) {
    cprint(order[i].command);
    cprint("\n");
    i++;
  }
}