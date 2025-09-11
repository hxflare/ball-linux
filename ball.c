#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define RED 31
#define WHITE 37
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define DEFAULT 0


// set the color of the folowing output
void setColor(int color)
{
    char buffer[30];
    int len = snprintf(buffer, sizeof(buffer), "\033[%dm", color);
    write(1, buffer, len);
}
// write but colored
void cprint(char string[255], int colorCode)
{
    write(1, string, strlen(string));
}
// shell config.
typedef struct shellConf
{
    char aliases[255][255];
    char meanings[255][255];
    char PISS[255];
    char paths[255][255];
} shellConf;
// find a string in an array
int findStrInArr(char string[255], char array[255][255])
{
    for (int i = 0; i < 255; i++)
    {
        if (strcmp(string, array[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}
// Transform the escape characters into actual strings
void formatEscape(char *string)
{
    char result[255];
    char *src = string;
    char *dst = result;
    while (*src)
    {
        // Detect the escape character
        if (*src == '%' && *(src + 1))
        {
            switch (*(src + 1))
            {
            // Case for the path
            case 'p':
            {
                char cwd[256];
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                    strcpy(dst, cwd);
                    dst += strlen(cwd);
                }
                src += 2;
                break;
            }
            // Case for the user
            case 'u':
            {
                char *user = getenv("USER");
                if (user)
                {
                    strcpy(dst, user);
                    dst += strlen(user);
                }
                src += 2;
                break;
            }
            // Case for the hostname
            case 'h':
            {
                char hostname[255];
                int result = gethostname(hostname, 255);
                if (result == 0)
                {
                    strcpy(dst, hostname);
                    dst += strlen(hostname);
                }
                src += 2;
                break;
            }
            // New line
            case 'n':
            {

                strcpy(dst, "\n");
                dst += 1;

                src += 2;
                break;
            }
            // Colors
            case 'R': { strcpy(dst, "\033[31m"); dst += strlen("\033[31m"); src += 2; break; }
            case 'G': { strcpy(dst, "\033[32m"); dst += strlen("\033[32m"); src += 2; break; }
            case 'W': { strcpy(dst, "\033[37m"); dst += strlen("\033[37m"); src += 2; break; }
            case 'Y': { strcpy(dst, "\033[33m"); dst += strlen("\033[33m"); src += 2; break; }
            case 'B': { strcpy(dst, "\033[34m"); dst += strlen("\033[34m"); src += 2; break; }
            case 'M': { strcpy(dst, "\033[35m"); dst += strlen("\033[35m"); src += 2; break; }
            case 'C': { strcpy(dst, "\033[36m"); dst += strlen("\033[36m"); src += 2; break; }
            case 'D': { strcpy(dst, "\033[0m");  dst += strlen("\033[0m");  src += 2; break; }

            default:
                *dst++ = *src++;
                break;
            }
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    strcpy(string, result);
}
// Split the command by spaces
void splitCommand(char *command, char **argv, int *argc)
{
    *argc = 0;
    char *token = strtok(command, " ");
    while (token)
    {
        argv[*argc] = token;
        (*argc)++;
        token = strtok(NULL, " ");
    }
    argv[*argc] = NULL;
}
// Execute a single command, no forks and pipes etc
void executeSingle(char command[255], shellConf config)
{
    extern char **environ;
    char *argv[255];
    int argc = 0;
    splitCommand(command, (char **)argv, &argc);
    if (argc == 0)
    {
        return;
    }
    // Detect aliases
    for (int i = 0; i < argc; i++)
    {
        int aliasid = findStrInArr(argv[i], config.aliases);
        if (aliasid != -1)
        {
            printf("alias detected");
            strcpy(argv[i], config.meanings[aliasid]);
        }
    }
    // Add null to the end of the args
    char *args[argc + 1];
    for (int i = 0; i < argc; i++)
    {
        args[i] = argv[i];
    }
    args[argc] = NULL;
    if (strchr(argv[0], '/'))
    {
        execve(argv[0], args, environ);
        perror("execve");
        exit(1);
    }
    // For paths defined in the config
    for (int i = 0; i < 255 && config.paths[i][0] != '\0'; i++)
    {
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", config.paths[i], argv[0]);
        if (execve(fullpath, args, environ) != -1)
            return;
    }
    fprintf(stderr, "Command not found: %s\n", argv[0]);
    exit(1);
}
// Wrapper for executing commands. Builtins too.
void executeCommand(char command[255], shellConf config)
{
    
    char *argv[255];
    int argc = 0;
    splitCommand(command, (char **)argv, &argc);
    if (argc == 0)
    {
        return;
    }
    // Detect aliases
    for (int i = 0; i < argc; i++)
    {
        int aliasid = findStrInArr(argv[i], config.aliases);
        if (aliasid != -1)
        {
            printf("alias detected");
            strcpy(argv[i], config.meanings[aliasid]);
        }
    }
    // Add null to the end of the args
    char *args[argc + 1];
    for (int i = 0; i < argc; i++)
    {
        args[i] = argv[i];
    }
    args[argc] = NULL;
    // BUILTINS
    // cd command
    if (strcmp(argv[0], "cd") == 0)
    {
        const char *dir = (argc > 1) ? argv[1] : getenv("HOME");
        if (chdir(dir) != 0)
            perror("not a directory");
        return;
    }
    // exit command
    if (strcmp(argv[0], "exit") == 0)
    {
        exit(0);
    }
    // export command
    if (strcmp(argv[0], "export") == 0 && argc > 1)
    {
        char *eq = strchr(argv[1], '=');
        if (eq)
        {
            *eq = '\0';
            setenv(argv[1], eq + 1, 1);
        }
        return;
    }
    // Normal command, piping(actually works, but not if space separated)
    
}
// Get the shell config
shellConf getConf(FILE *rc)
{
    char line[1024];
    char loaded[90][1024];
    int ammount = 0;
    shellConf config = {0};
    while (fgets(line, sizeof(line), rc) != NULL)
    {
        strcpy(loaded[ammount], line);
        ammount++;
    }
    for (int i = 0; i < ammount; i++)
    {
        if (loaded[i][0] == '!')
        {
            char keyword[10];
            char values[50][255];
            int valueCount = 0;
            loaded[i][strcspn(loaded[i], "\n")] = 0;
            strcpy(keyword, loaded[i]);
            // Assign the values which it finds to the keyword
            for (int j = i + 1; j < ammount; j++)
            {
                if (loaded[j][0] == '@')
                {
                    strcpy(values[valueCount], loaded[j]);
                    valueCount++;
                }
                else if (loaded[j][0] == '!')
                {
                    break;
                }
            }
            // Piss formating
            if (strcmp(keyword, "!PISS") == 0)
            {
                char *format = values[0] + 1;
                format[strcspn(format, "\n")] = 0;
                strcpy(config.PISS, format);
            }
            // Alias formating
            else if (strcmp(keyword, "!ALIAS") == 0)
            {
                for (int i = 0; i < valueCount; i++)
                {
                    char *format = values[i] + 1;
                    format[strcspn(format, "\n")] = 0;
                    char *eq = strchr(format, '=');
                    if (!eq)
                        continue;
                    *eq = '\0';
                    strcpy(config.aliases[i], format);
                    strcpy(config.meanings[i], eq + 1);
                }
            }
            // Path formating
            else if (strcmp(keyword, "!PATH") == 0)
            {
                for (int i = 0; i < valueCount; i++)
                {
                    char *format = values[i] + 1;
                    format[strcspn(format, "\n")] = 0;
                    strcpy(config.paths[i], format);
                }
            }
        }
    }
    for (int i = 0; i < ammount; i++)
    {
        if (loaded[i][0] == '$')
        {
            char *format = loaded[i] + 1;
            executeCommand(format, config);
        }
    }
    return config;
}
// Main shell loop
void loop()
{
    FILE *rcfile = fopen(".ballrc", "r");
    shellConf conf;
    // Config file creating and parsing
    if (rcfile == NULL)
    {
        rcfile = fopen(".ballrc", "w");
        fprintf(rcfile, "The ball shell configuration file.\n  You should put a newline without any spaces after if you want to declare a keyword or a meaning or a startup commands(must be put last).\n  Keyword start with !, values start with @, startup commands start with $.\n  string modifiers for PISS: %%n - newline %%u - user %% h - hostname %%p - current path\n  Colors(start with %%) are just the capital first letter of color. For example: %%C - cyan \n!PISS\n@%%u@%%h  %%p %%n#   \n!ALIAS\n\n!PATH\n@/bin\n@/usr/bin\n@/usr/local/bin\n@/sbin\n@/usr/sbin\n");
        fclose(rcfile);
        rcfile = fopen(".ballrc", "r");
        conf = getConf(rcfile);
        fclose(rcfile);
    }
    else
    {
        conf = getConf(rcfile);
        fclose(rcfile);
    }
    char command[255];
    int commandLength;
    char prompt[255];

    while (1)
    {
        strcpy(prompt, conf.PISS);
        formatEscape(prompt);
        cprint(prompt, 0);
        commandLength = read(0, command, 255);
        if (commandLength <= 0)
            continue;
        command[commandLength - 1] = 0;
        executeCommand(command, conf);
    }
}
// Start of the program
int main()
{

    loop();
    return 0;
}
