#ifndef BTOOLS_H
#define BTOOLS_H
void cprint(char *string);
char *str_replace(char *orig, char *rep, char *with);
void print_strlist(char **array);
char* concat(const char *s1, const char *s2);
int str_isdigit(char *str);
#endif