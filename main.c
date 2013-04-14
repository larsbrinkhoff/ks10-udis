#include <stdio.h>
#include <stdlib.h>
#include "udis.h"

static unsigned char word[RAM_SIZE][WORD_SIZE];

int
main (int argc, char **argv)
{
  int length;

  length = 0;
  while (!feof (stdin))
    {
      if (length == sizeof word / 15)
	abort ();
      if (fread (word[length], sizeof word[0], 1, stdin) != 1)
	break;
      length++;
    }

  udis (length, word);

  return 0;
}
