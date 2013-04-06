#define RAM_SIZE 4096
#define WORD_SIZE 15

union bits
{
  struct { int first; int last; } range;
  struct { int dummy; int *list; } list;
};

struct field
{
  char *name;
  union bits bits;
  int def_ault;
  char **names;
};

struct ks10_uinsn
{
  int field[29];
  int done;
};

extern struct field fields[];
extern int field_num;

extern void udis (int, unsigned char [RAM_SIZE][WORD_SIZE], int *);
