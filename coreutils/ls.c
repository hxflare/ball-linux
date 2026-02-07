#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void cprint(char string[255])
{
    write(1, string, strlen(string));
}
int main(int argc, char **argv)
{
    DIR *dir;
    struct dirent *entry;
    char *path=".";
    dir=opendir(path);
    if (dir == NULL){
        cprint("shi... unable to open that directory");
        cprint("\n");
        exit(EXIT_FAILURE);
    }    
    while ((entry = readdir(dir)) != NULL) {
        cprint(entry->d_name);
        cprint("\n");
    }
    closedir(dir);
    return EXIT_SUCCESS;
}