#include <unistd.h>
size_t strlen(const char* start)
{
    const char* end = start;
    while (*end != '\0')
        ++end;
    return end - start;
}
void cprint(char *string)
{
    write(1, string, strlen(string));
}
int main(int argc, char **argv){
    cprint("\e[1;1H\e[2J");
}