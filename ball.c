#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// write but colored
void cprint(char *string)
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
char **extract_args(char *command){
    int raw_c=1;
    for (int i=0;command[i];i++){
        if (command[i]==' '){
            raw_c++;
        }
    }
    char **raw_args=malloc((raw_c+1)*sizeof(char *));
    free(raw_args);
}
int execute(char mode, char *execd,shellConf config){
    switch (mode){
        case 'c':
            //execute a normal command
            //check if file is a path

            break;
        case 'f':
            //execute a file
            break;
        default:
            cprint("invalid mode");
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
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
    return config;
}
// Main shell loop
void loop()
{
    
}
int main(int argc, char **argv){
    FILE *rcfile = fopen(".ballrc", "r");
    shellConf conf;
    // Config file creating and parsing
    if (rcfile == NULL)
    {
        rcfile = fopen(".ballrc", "w");
        fprintf(rcfile, "The ball shell configuration file.\n  You should put a newline without any spaces after if you want to declare a keyword or a meaning or a startup commands(must be put last).\n  Keyword start with !, values start with @, startup commands start with $.\n  string modifiers for PISS: %%n - newline %%u - user %%h - hostname %%p - current path\n  Colors - use ascii color codes \n\n \n!PISS\n@%%u@%%h  %%p %%n#   \n!ALIAS\n\n!PATH\n@/bin\n@/usr/bin\n@/usr/local/bin\n@/sbin\n@/usr/sbin\n");
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
    if(argc>1){
        // execute a shell script
        for (int i=1;i<argc;i++){
            execute('f', argv[i],conf);
        }
    }else{
        //start the shell
        loop();
    }
}