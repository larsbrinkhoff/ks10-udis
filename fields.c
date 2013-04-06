#include "udis.h"

static char *ADnames[] =
{
  "A+Q",	  "A+B",	  "0+Q",    "0+B",    "0+A",	"D+A",		"D+Q",		"0+D",
  "Q-A-.25",	  "B-A-.25",	  "Q-.25",  "B-.25",  "A-.25",	"A-D-.25",	"Q-D-.25",	"-D-.25",
  "A-Q-.25",	  "A-B-.25",	  "-Q.25",  "-B.25",  "-A.25",	"D-A-.25",	"D-Q-.25",	"D-.25",
  "A.OR.Q",	  "A.OR.B",	  "Q",	    "B",      "A",	"D.OR.A",	"D.OR.Q",	"D",
  "A.AND.Q",	  "A.AND.B",	  "ZERO",   "ZERO",   "ZERO",	"D.AND.A",	"D.AND.Q",	"ZERO",
  ".NOT.A.AND.Q", ".NOT.A.AND.B", "Q",	    "B",      "A",	".NOT.D.AND.A",	".NOT.D.AND.Q",	"ZERO",
  "A.XOR.Q",	  "A.XOR.B",	  "Q",	    "B",      "A",	"D.XOR.A",	"D.XOR.Q",	"D",
  "A.EQV.Q",	  "A.EQV.B",	  ".NOT.Q", ".NOT.B", ".NOT.A",	"D.EQV.A",	"D.EQV.Q",	".NOT.D",
};

static char *SRCnames[] =
{
  "AQ", "AB", "0Q", "0B", "0A", "DA", "DQ", "D0"
};

static char *DESTnames[] =
{
  "A", "AD", "Q_AD", "PASS", "Q_Q*2", "AD*2", "Q_Q*.5", "AD*.5",
};

static char *ABnames[] =
{
  "MAG", "PC", "HR", "AR", "ARX", "BR", "BRX", "ONE",
  "EBR", "UBR", "MASK", "FLG", "PI", "XWD1", "T0", "T1"
};

static char *RAMADRnames[] =
{
  "AC#", "AC*#", "XR#", "3", "VMA", "5", "RAM", "#"
};

static char *DBUSnames[] =
{
  "PC FLAGS", /* "PI NEW", "VMA", */ "DP", "RAM", "DBM"
};

static char *DBMnames[] =
{
  "SCAD DIAG", /* "PF DISP", "APR FLAGS", */ "BYTES", "EXP",
  "DP", "DP SWAP", "VMA", "MEM", "#"
};

static char *SPECnames[] =
{
  "0", "1", "2", "3", "4", "5", "6", "7"
  "#", "CLRCLK", "CLR IO LATCH", "CLR IO BUSY", "LDPAGE", "NICOND", "LDPXCT",
    "WAIT",
  "PREV", "LOADCR", "22", "APR FLAGS", "CLRCSH", "APR EN", "26", "MEMCLR",
  "30", "31", "32", "33", "SWEEP", "35", "PXCT OFF", "37",
  "INCRY18", "LOADIR", "42", "LDPI", "ASHOV", "EXPTST", "FLAGS", "LDACBLK",
  "50", "51", "52", "53", "54", "55", "56", "57",
  "60", "LDINST", "62", "63", "64", "65", "66", "67",
  "70", "71", "72", "73", "74", "75", "76", "77",
};

static char *DISPnames[] =
{
  "CONSOLE", "1", "2", "3", "4", "5", "6", "7"
  "10", "11", "DROM", "AREAD", "14", "15", "16", "17",
  "20", "21", "22", "23", "24", "25", "26", "27"
  "30", "DP LEFT", "32", "33", "NORM", "DP", "ADISP", "BDISP",
  "40", "RETURN", "42", "43", "44", "45", "46", "47",
  "50", "51", "52", "53", "54", "55", "56", "57",
  "60", "61", "MUL", "PAGE FAIL", "NICOND", "BYTE", "EAMODE", "SCAD0",
  "70", "71", "72", "73", "74", "75", "76", "77",
};

static char *SKIPnames[] =
{
  "0", "1", "2", "3", "IOLGL", "5", "6", "7",
  "10", "11", "LLE", "13", "14", "15", "16", "17",
  "20", "21", "22", "23", "24", "25", "26", "27"
  "30", "CRY0", "ADLEQ0", "ADREQ0", "KERNEL", "FPD", "AC0", "INT",
  "40", "41", "LE", "43", "44", "45", "46", "47",
  "50", "CRY2", "DP0", "DP18", "IOT", "JFCL", "CRY1", "TXXX",
  "60", "TRAP CYCLE", "ADEQ0", "SC", "EXECUTE", "-IO BUSY", "-CONTINE","-1 MS",
  "70", "71", "72", "73", "74", "75", "76", "77",
};

static char *Tnames[] =
{
  "2T", "3T", "4T", "5T"
};

static int SPECbits[] = { 110, 111, 116, 46, 47, 48, -1 };
static int DISPbits[] = { 117, 118, 119, 43, 44, 45, -1 };
static int SKIPbits[] = { 107, 108, 109, 49, 50, 51, -1 };
static int NUMBERbits[] = { 70, 71, 76, 77, 78, 79, 52, 53, 54,
			    55, 56, 57, 58, 59, 60, 61, 62, 63, -1 };

#define RANGE(first, last) { { first, last } }
#define LIST(x) { list: { -1, x } }

struct field fields[] =
{
  { "J",		RANGE (92, 103),	-1,	0 },
  { "AD",		RANGE (0, 5),		044,	ADnames },
  { " LSRC",		RANGE (3, 5),		-1,	SRCnames },
  { "RSRC",		RANGE (6, 8),		-2,	SRCnames },
  { "DEST",		RANGE (27, 29),		3,	DESTnames },
  { "A",		RANGE (14, 17),		-1,	ABnames },
  { "B",		RANGE (20, 23),		0,	ABnames },
  { "RAMADR",		RANGE (24, 26),		4,	RAMADRnames },
  { "DBUS",		RANGE (12, 13),		1,	DBUSnames },
  { "DBM",		RANGE (9, 11),		7,	DBMnames },
  { "CLKL",		RANGE (18, 18),		1,	0 },
  { "GENL",		RANGE (66, 66),		0,	0 },
  { "CHKL",		RANGE (36, 36),		0,	0 },
  { "CLKR",		RANGE (19, 19),		1,	0 },
  { "GENR",		RANGE (67, 67),		0,	0 },
  { "CHKR",		RANGE (37, 37),		0,	0 },
  { "SPEC",		LIST (SPECbits),	0,	SPECnames },
  { "DISP",		LIST (DISPbits),	070,	DISPnames },
  { "SKIP",		LIST (SKIPbits),	070,	SKIPnames },
  { "T",		RANGE (104, 105),	-1,	Tnames },
  { "CRY38",		RANGE (41, 41),		0,	0 },
  { "LOADSC",		RANGE (30, 30),		0,	0 },
  { "LOADFE",		RANGE (31, 31),		0,	0 },
  { "FMWRITE",		RANGE (64, 64),		0,	0 },
  { "MEM",		RANGE (42, 24),		0,	0 },
  { "DIVIDE",		RANGE (68, 68),		0,	0 },
  { "MULTI PREC",	RANGE (69, 69),		0,	0 },
  { "MULTI SHIFT",	RANGE (65, 65),		0,	0 },
  { "CALL",		RANGE (106, 106),	0,	0 },
  { "#",		LIST (NUMBERbits),	0,	0 },
};

int field_num = sizeof fields / sizeof fields[0];
