#define RAM_SIZE 4096
#define WORD_SIZE 15

union bits
{
  struct { int first; int last; } range;
  struct { int dummy; int *list; } list;
};

struct ks10_uinsn
{
  int field[100];
  int done;
};

struct field
{
  char *name;
  int (*print_field) (int, struct ks10_uinsn *);
  union bits bits;
  int def_ault;
  char **names;
};

extern struct field fields[];
extern int field_num;

extern void udis (int, unsigned char [RAM_SIZE][WORD_SIZE]);
extern int get_field (char *name, struct ks10_uinsn *insn);
