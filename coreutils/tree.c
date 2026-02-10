#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MARGIN 40
void cprint(char *string)
{
    write(1, string, strlen(string));
}
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
void scan_dir(char *path, int show_colors, int show_filetypes, int show_hidden, int help,int stage){
    DIR *dir;
    struct dirent *entry;
    dir=opendir(path);
    while ((entry = readdir(dir)) != NULL) {
        char *name=entry->d_name;
        unsigned char type=entry->d_type;
        if (name[0]!='.'||show_hidden==1){
            if (show_colors==1){
                switch (type) {
                    case DT_DIR:
                        cprint("\033[33m");
                        break;
                    case DT_REG:
                        cprint("\033[34m");
                        break;
                    case DT_BLK:
                        cprint("\033[32m");
                        break;
                    case DT_LNK:
                        cprint("\033[36m");
                        break;
                    case DT_UNKNOWN:
                        cprint("\033[0m");
                        break;
                    case DT_FIFO:
                        cprint("\033[35m");
                        break;
                    case DT_SOCK:
                        cprint("\033[33m");
                        break;
                    case DT_CHR:
                        cprint("\033[33m");
                        break;
                    default:
                        cprint("\033[0m");
                        break;
                }
            }
            for (int i=0; i<stage; i++){
                cprint("-");
            }
            cprint(name);
            if(show_filetypes){
                if (strlen(name)<MARGIN)
                {
                    for (int i=0;i<(MARGIN-strlen(name));i++){
                        cprint(" ");
                    }
                }
                if(help){
                    cprint("  ");
                    cprint("Type:");
                    cprint("    ");
                }
                
            }
            
            if (show_filetypes==1){
                switch (type) {
                    case DT_DIR:
                        cprint("Directory");
                        break;
                    case DT_REG:
                        cprint("File");
                        break;
                    case DT_BLK:
                        cprint("Block Device");
                        break;
                    case DT_LNK:
                        cprint("Symlink");
                        break;
                    case DT_UNKNOWN:
                        cprint("Unknown");
                        break;
                    case DT_FIFO:
                        cprint("FIFO");
                        break;
                    case DT_SOCK:
                        cprint("Unix Domain Socket");
                        break;
                    case DT_CHR:
                        cprint("Character Device");
                        break;
                    default:
                        cprint("Unknown");
                        break;
                }
            }
            cprint("\n");
            if (type==DT_DIR){
                scan_dir(concat(concat(path,"/"), name), show_colors, show_filetypes, show_hidden, help, stage+1);
            }
        }
    }
    closedir(dir);
}
int main(int argc, char **argv)
{
    int show_hidden=0;
    int show_filetypes=0;
    int show_colors=0;
    int help=0;
    DIR *dir;
    struct dirent *entry;
    char *path=".";
    for (int i =1; i<argc;i++){
        if (argv[i][0]!='-'){
            path = argv[i];
        }else{
            for(int k=1;k<(strlen(argv[i]));k++){
                switch (argv[i][k]){
                    case 'h':
                        show_hidden=1;
                        break;
                    case 't':
                        show_filetypes=1;
                        break;
                    case 'c':
                        show_colors=1;
                        break;
                    case 'i':
                        help=1;
                        break;
                    default:
                        cprint("invalid argument "); cprint(argv[i]); cprint("\n");
                        break;
                }
            }
        }
    }
    dir=opendir(path);
    if (dir == NULL){
        cprint("shi... unable to open that directory");
        cprint("\n");
        exit(EXIT_FAILURE);
    }    
    scan_dir(path, show_colors, show_filetypes, show_hidden, help, 0);
    return EXIT_SUCCESS;
}