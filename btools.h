#ifndef BTOOLS_H
#define BTOOLS_H
void cprint(const char *string);
char *str_replace(char *orig, char *rep, char *with);
void print_strlist(const char **array);
char* concat(const char *s1, const char *s2);
int str_isdigit(const char *str);
#endif