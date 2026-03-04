#include "../btools.h"
int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    cprint(argv[i]);
  }
  cprint("\n");
}