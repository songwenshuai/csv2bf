/*********************************************************************
----------------------------------------------------------------------
File        : csv2bf.c
Purpose     : This program will transform a csv file into a C header
              file suitable for compiling.
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

//
// Define _MEM_DEBUG: Debug level for SSL product
//                  0: No checks                      (Smallest and fastest code)
//                  1: Warnings & Panic checks
//                  2: Warnings, logs, & panic checks (Bigger code)
//
#ifndef   DEBUG
  #define DEBUG     0
#endif

#define CSV_DATA_START        1
#define BF_SIZE               32

#define STR_IP_NAME           "IP_Name"
#define STR_BLOCK             "Block"
#define STR_ADDRESS           "Address"
#define STR_BITS              "Bits"
#define STR_REG_NAME          "Register Name"
#define STR_R_W               "R/W"
#define STR_WIDTH             "Width"

#define VERSION_DATE          __DATE__
#define VERSION_TIME          __TIME__

#define GET_IP(csv,r)         _csvGet(csv, r, _csvIp)
#define GET_BLOCK(csv,r)      _csvGet(csv, r, _csvBlock)
#define GET_ADDRESS(csv,r)    _csvGet(csv, r, _cavAddress)
#define GET_BITS(csv,r)       _csvGet(csv, r, _csvBits)
#define GET_REG_NAME(csv,r)   _csvGet(csv, r, _csvRegname)
#define GET_R_W(csv,r)        _csvGet(csv, r, _csvRw)
#define GET_WIDTH(csv,r)      _csvGet(csv, r, _csvWidth)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef int   BOOLEAN;

#define TRUE  1
#define FALSE 0

#define U8    unsigned char
#define I8      signed char
#define U16   unsigned short
#define I16     signed short
#define U32   unsigned long
#define I32     signed long

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

int _csvIp      = 0;
int _csvBlock   = 0;
int _cavAddress = 0;
int _csvBits    = 0;
int _csvRegname = 0;
int _csvRw      = 0;
int _csvWidth   = 0;


char outhname[256];
char outfname[256];
char outpname[256];

typedef struct {
  char    _rw[256];
  char  _name[256];
  char _width[256];
  char  _bits[256];
} _BF_LIST_T;

_BF_LIST_T _bfList[BF_SIZE];

//
// csv readonly parse
// 1. Adjacent fields must be separated by a single comma, CRLF wraps
// 2. Each embedded double quote character must be represented as two double quote characters
// 3. Fields can be wrapped in double quotes, double quotes or commas with carriage return and line feed, it must be wrapped
//
typedef struct {   // struct in heap malloc
    int    _rlen;   // Number of data rows, index [0, _rlen)
    int    _clen;   // Number of data columns, index [0, _clen)
    char * _data[]; // Save data, _rlen * _clen two-dimensional array
} * _CSV_T;

typedef struct {
    int    _row;   // Number of data rows
    int    _col;   // Number of data columns
} _CSV_XY_T;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _Strlwr
*/
static char* _Strlwr(char *s)
{
  unsigned char *ucs = (unsigned char *) s;
  for ( ; *ucs != '\0'; ucs++)
    {
      *ucs = tolower(*ucs);
    }
  return s;
}

/*********************************************************************
*
*       _Strupr
*/
static char* _Strupr(char *s)
{
  unsigned char *ucs = (unsigned char *) s;
  for ( ; *ucs != '\0'; ucs++) {
    *ucs = toupper(*ucs);
  }
  return s;
}

/*********************************************************************
*
*       _ParsePath
*/
static void _ParsePath(char * topath, char * filename, char * path) {
  char * pfile;
  char * pto;
  char * pfr;
  char * pslash;
  BOOLEAN colon;

  pto    = topath;
  pfr    = path;
  pslash = NULL;
  if (path[0] && path[1] == ':') {
    colon = TRUE;
  } else {
    colon = FALSE;
  }
  //
  // Copy the whole path to topath remembering where the last backslash is
  //
  *pto = 0;
  while (*pfr) {
    *pto = *pfr++;
    if (*pto == '\\') {
      pslash = pto;
    }
    pto++;
  }
  *pto = '\0';
  //
  // Figure out where the file name starts
  //
  if (pslash) {
      pfile = pslash+1;
  } else {
    if (colon) {
      pfile = topath + 2;
    } else {
      pfile = topath;
    }
  }
  //
  // Copy and filter file name
  //
  while (*pfile) {
    if ((*pfile > 0x40 && *pfile < 0x5B) ||
      (*pfile > 0x60 && *pfile <= 0x7B) ||
      (*pfile > 0x2F && *pfile <= 0x3A)) {
      *(filename++) = *(pfile++);
    } else {
      *(filename++) = '_';
      pfile++;
    }
  }
  *filename = *pfile;
  if (!pslash) {
    if (colon) {
      *(topath+2) = '\0';
    } else {
      *topath = '\0';
    }
  } else {
    if ( (colon && (pslash == topath + 2)) || (!colon && (pslash == topath)) ) {
      *(pslash+1) = '\0';
    } else {
      *pslash = '\0';
    }
  }
}

//
// _csvGet - Get the string at the position of csv[r][c]
// csv     : _CSV_T object
// r       : Row index [0, csv->_rlen)
// c       : Column index [0, csv->_clen)
// return  : Return csv[r][c], follow-up can be atoi, atof, strdup...
//
static char * _csvGet(_CSV_T csv, int r, int c) {
  if (!csv || r < 0 || r >= csv->_rlen || c < 0 || c >= csv->_clen) {
    printf("params is error csv:%p, r:%d, c:%d.", csv, r, c);
    return NULL;
  }

  // return csv[r][c] index position string
  return csv->_data[r * csv->_clen + c];
}

//
// _xyGet - Get the string at the position of csv[r][c]
// csv     : _CSV_T object
// str     : Row index [0, csv->_rlen)
// return  : Return csv[r][c], follow-up can be atoi, atof, strdup...
//
_CSV_XY_T _xyGet(_CSV_T csv, const char *str) {
  char temp_csv[1024];
  char temp_str[1024];
  _CSV_XY_T _csv_xy = { -1, -1 };

  strcpy(temp_str, str);
  for (U32 i = 0; i < csv->_rlen; i++) {
    for (U32 j = i + 1; j < csv->_clen; j++) {

      memset(temp_csv, 0, sizeof(temp_csv));
      strcpy(temp_csv, _csvGet(csv, i, j));
      _Strlwr(temp_csv);
      _Strlwr(temp_str);

      if (!strcmp(temp_str, temp_csv)) {
#if (DEBUG == 1)
        printf("temp_csv = %s\r\n", temp_csv);
        printf("temp_str = %s\r\n", temp_str);
        printf("\r\n");
#endif
        _csv_xy._row = i;
        _csv_xy._col = j;
        return _csv_xy;
      }
    }
  }
  return _csv_xy;
}

//
// _csvDelete - Release the _CSV_T object
// csv     : _CSV_T object
// return  : void
//
void _csvDelete(_CSV_T csv) {
    free(csv);
}

//
// _csvCheck - Analyze and check the content of the csv file, and return the constructed legal string length
//
static int _csvCheck(char * str, int * pr, int * pc) {
    int c, rnt = 0, cnt = 0;
    char * tar = str, * s = str;
    while ((c = *tar++) != '\0') {
        // csv Content analysis, state machine switching
        switch (c) {
        case '"' : // Special character handling in double quotes
            while ((c = *tar++) != '\0') {
                if ('"' == c) {
                    // Valid characters are pushed onto the stack again, and extra "characters are removed by the way
                    if (*tar != '"')
                        break;
                    ++tar;
                }
                // add the resulting character
                *s++ = c;
            }
            // Continue to judge, only if c =='"' will continue, otherwise it is abnormal
            if (c != '"')
                goto err_faid;
            break;
        case ',' : *s++ = '\0'; ++cnt; break;
        case '\r': break;
        case '\n': *s++ = '\0'; ++cnt; ++rnt; break;
        // In all other cases, just add data
        default  : *s++ = c;
        }
    }
    // CRLF processing
    if (str != s && (c = tar[-2]) && c != '\n') {
        *s++ = '\0'; ++cnt; ++rnt;
    }

    // Check whether the number of rows and columns is normal
    if (rnt == 0 || cnt % rnt) {
err_faid:
        printf("csv parse error %d, %d, %d.", c, rnt, cnt);
        return -1;
    }

    // return the final content
    *pr = rnt; *pc = cnt;
    return (int)(s - str);
}

//
// _csvParse - Parse the byte stream and return a csv object
//
_CSV_T _csvParse(char * str) {
    int n, rnt, cnt;
    n = _csvCheck(str, &rnt, &cnt);
    if (n < 0)
        return NULL;

    // Allocate final memory
    _CSV_T csv = malloc(n + sizeof *csv + sizeof(char *) * cnt);
    char * s = (char *)csv + sizeof *csv + sizeof(char *) * cnt;
    memcpy(s, str, n);

    // Start memory sorting, csv field filling
    n = 0;
    csv->_rlen = rnt;
    csv->_clen = cnt / rnt;
    do {
        csv->_data[n] = s;
        while (*s++)
            ;
    } while (++n < cnt);

    return csv;
}

/*********************************************************************
*
*       _Isunderscore()
*
*/
static int _Isunderscore(int ch) {
    if (ch == '_')
        return TRUE;
    else
        return FALSE;
}

/*********************************************************************
*
*       _Illegal
*/
static char* _Illegal(char *s)
{
  unsigned char *ucs = (unsigned char *) s;
  for ( ; *ucs != '\0'; ucs++) {
    if(!isalnum(*ucs) || _Isunderscore(*ucs)) {
        *ucs = '_';
    }
  }
  return s;
}

/*********************************************************************
*
*       _Addrtoui
*/
static unsigned int _Addrtoui(const char *string) {
  const char *p;
  unsigned int digit = 0;
  unsigned int result = 0;

/*
 * The table below is used to convert from ASCII digits to a
 * numerical equivalent.  It maps from '0' through 'z' to integers
 * (100 for non-digit characters).
 */
static char cvtIn[] = {
    0,   1,   2,   3,   4,   5,   6,  7,  8,  9,  /* '0' - '9' */
  100, 100, 100, 100, 100, 100, 100,		          /* punctuation */
   10,  11,  12,  13,  14,  15,  16, 17, 18, 19,	/* 'A' - 'Z' */
   20,  21,  22,  23,  24,  25,  26, 27, 28, 29,
   30,  31,  32,  33,  34,  35,
  100, 100, 100, 100, 100, 100,		                /* punctuation */
   10,  11,  12,  13,  14,  15,  16, 17, 18, 19,	/* 'a' - 'z' */
   20,  21,  22,  23,  24,  25,  26, 27, 28, 29,
   30,  31,  32,  33,  34,  35
};

  p = string;
  if (*p == '0') {
    p += 1;
    if ((*p == 'x') || (*p == 'X')) {
      p += 1;
    }
  }
  for (;; p += 1) {
    if (*p == '_') {
      p += 1;
    }
    digit = *p - '0';
    if (digit > ('z' - '0')) {
      break;
    }
    digit = cvtIn[digit];
    if (digit > 15) {
      break;
    }
    result = (result << 4);
    result += digit;
  }

  return result;
}

//
// Intercept size characters from string pos position
//
char *_strMid(char *src, char *dst, U32 pos, U32 size) {
	char *p = src;
	char *q = dst;

	p += pos;

	while (size--)
		*(q++) = *(p++);

	*(q++) = '\0';

	return dst;
}

//
// Remove underscore from both ends
//
void _strTrimLeftRight(char *src, U32 ilen)
{
	char *s = src;
	U32 start = 0, end = ilen;
  char tmp[256];

	BOOLEAN bChanged = TRUE;

	while (start < end && bChanged) {
		bChanged = FALSE;

		if (*(s + start) == '_') {
			++start;

			bChanged = TRUE;
		} else {
			if (*(s + end - 1) == '_') {
				--end;

				bChanged = TRUE;
			}
		}
	}

	_strMid(src, tmp, start, end - start);
  strcpy(s, tmp);
}

//
// Do a character swap from begin to end
//
void _reverseStr(char *begin, char *end) {
    char temp;
    while (begin < end) {
      temp   = *begin;
      *begin = *end;
      *end   = temp;
      begin++;
      end--;
    }
}

//
// Cycle right string
//
void _rightRotationStr(char *str, int n, int nLen) {
    n %= nLen;
    _reverseStr(str    , str + nLen - 1);
    _reverseStr(str    , str + n - 1);
    _reverseStr(str + n, str + nLen - 1);
}

/*********************************************************************
*
*       _RWHandle
*/
void _RWHandle(void *_bfList, U32 _bCount)
{
  U8* _pChar             = NULL;
  U32 _Cnt               = 0;
  _BF_LIST_T *_pbfList   = NULL;

  _pbfList = _bfList;
  _Cnt     = _bCount;

  while (_Cnt--) {
    //
    // point to rw
    //
    _pChar = _pbfList[_Cnt]._rw;
    //
    // Replace RW to IO
    //
    if (!strcmp(_pChar,"R/W") || !strcmp(_pChar,"RW"))
      strcpy(_pChar,"__IO");
    else if (!strcmp(_pChar,"R"))
      strcpy(_pChar,"__I");
    else if (!strcmp(_pChar,"W"))
      strcpy(_pChar,"__O");

#if (DEBUG == 1)
    printf("\n");
    printf(">\n");
    printf("_pChar        = %s\n", _pChar);
    printf("_Cnt          = %ld\n", _Cnt);
    printf("_width        = %s\n", _pbfList[_Cnt]._width);
    printf("_rw           = %s\n", _pbfList[_Cnt]._rw);
    printf("_name         = %s\n", _pbfList[_Cnt]._name);
    printf("_bits         = %s\n", _pbfList[_Cnt]._bits);
    printf("\n");
    printf("<\n");
#endif
  }
}

/*********************************************************************
*
*       _NameHandle
*/
void _NameHandle(void *_bfList, U32 _bCount)
{
  U8* _pChar            = NULL;
  U32 _strLen           = 0;
  U32 _Cnt              = 0;
  _BF_LIST_T *_pbfList  = NULL;

  _pbfList = _bfList;
  _Cnt     = _bCount;

  while (_Cnt--) {
    //
    // point to name
    //
    _pChar = _pbfList[_Cnt]._name;
    //
    // calc str len
    //
    _strLen = (U32)strlen(_pChar);
    //
    // name to lower
    //
    _Strlwr(_pChar);
    //
    // Non-alphanumeric or underscores are replaced with underscores
    //
    _Illegal(_pChar);
    //
    // Remove the underscore from both ends.
    //
    _strTrimLeftRight(_pChar, _strLen);
    //
    // Replace starting position 0
    //
    if(isdigit(_pChar[0]))
    {
      //
      // Cycle right, intentionally move one more place, move the last 0 to the first place, and replace it with an underscore.
      //
      _rightRotationStr(_pChar, 1, _strLen + 1);
      _pChar[0] = '_';
    }

#if (DEBUG == 1)
    printf("\n");
    printf(">\n");
    printf("_Cnt        = %ld\n", _Cnt);
    printf("_width      = %s\n", _pbfList[_Cnt]._width);
    printf("_rw         = %s\n", _pbfList[_Cnt]._rw);
    printf("_name       = %s\n", _pbfList[_Cnt]._name);
    printf("_bits       = %s\n", _pbfList[_Cnt]._bits);
    printf("\n");
    printf("<\n");
#endif
  }
  //
  // Modify duplicate variable names
  //
  char   _count[3];
  for (U32 i = 0; i < BF_SIZE; i++) {
    for (U32 j = i + 1; j < BF_SIZE; j++) {
      if (!strcmp(_pbfList[i]._name, _pbfList[j]._name)) {
        strcat(_pbfList[i]._name, "_");
        sprintf(_count, "%ld", BF_SIZE - i - 1);
        strcat(_pbfList[i]._name, _count);
      }
    }
  }
}

/*********************************************************************
*
*       _WriteStructFile
*/
void _WriteStructFile (FILE* const _Stream, _CSV_T _Csv)
{
  //
  // Points to the name of the current register page
  //
  const U8* Block = NULL;
  //
  // Register internal bit field count
  //
  U32 bCount = 0;
  //
  // Calculate base address, current address, and address offset
  //
  U32 BaseAddr = 0, OffSet = 0;
  //
  // Bit field width
  //
  U32 Width = 0;
  //
  // Row count
  //
  I32 rCount = CSV_DATA_START;

  BaseAddr      = _Addrtoui(GET_ADDRESS(_Csv, CSV_DATA_START));
#if (DEBUG == 1)
  printf("BaseAddr    = 0x%08lX\n", BaseAddr);
  printf("_rlen        = 0x%d\n", _Csv->_rlen);
  printf("\n");
#endif
  while (rCount < _Csv->_rlen) {
    //
    // Check that the table content is empty
    //
    if(!*GET_IP(_Csv, rCount)) {
      rCount++;
      continue;
    }

    OffSet      = _Addrtoui(GET_ADDRESS(_Csv, rCount)) - BaseAddr;
    Block       = GET_BLOCK(_Csv, rCount);
    //
    // Block to uper
    //
    _Strupr((char *)Block);
    //
    // Non-alphanumeric or underscores are replaced with underscores
    //
    _Illegal((char *)Block);
    //
    // Because it is deliberately shifted by one bit when shifting to the left, it needs to be cleared here.
    //
    memset(_bfList, 0, sizeof(_bfList));
#if (DEBUG == 1)
    printf("rCount      = %ld\n", rCount);
    printf("CurrentAddr = 0x%08X\n", _Addrtoui(GET_ADDRESS(_Csv, rCount)));
    printf("OffSet      = 0x%04lX\n", OffSet);
    printf("Block       = %s\n", GET_BLOCK(_Csv, rCount));
    printf("\n");
#endif
    fprintf(_Stream, "//\n");
    fprintf(_Stream, "// %s_%04lX\n", Block, OffSet);
    fprintf(_Stream, "//\n");
    fprintf(_Stream, "#define  _TV303_%s_%04lX\t\t(_TV303_%s_BASE + 0x%04lXU)\n", Block, OffSet, GET_IP(_Csv, CSV_DATA_START), OffSet);
    fprintf(_Stream, "\n");
    fprintf(_Stream, "typedef struct\n");
    fprintf(_Stream, "{\n");
#if (DEBUG == 1)
    printf("=====================================\n");
#endif
    for (Width = 0, bCount = 0; Width < 32; Width = Width + atoi(GET_WIDTH(_Csv, rCount)), rCount++, bCount++) {

      strcpy(_bfList[bCount]._width, GET_WIDTH(_Csv, rCount));
      strcpy(_bfList[bCount]._rw   , GET_R_W(_Csv, rCount));
      strcpy(_bfList[bCount]._name , GET_REG_NAME(_Csv, rCount));
      strcpy(_bfList[bCount]._bits , GET_BITS(_Csv, rCount));

#if (DEBUG == 1)
      printf("\n");
      printf(">\n");
      printf("rCount      = %ld\n", rCount);
      printf("bCount      = %ld\n", bCount);
      printf("width       = %ld\n", Width);
      printf("Bit         = %d\n", atoi(GET_WIDTH(_Csv, rCount)));
      printf("CurrentAddr = 0x%08X\n", _Addrtoui(GET_ADDRESS(_Csv, rCount)));
      printf("OffSet      = 0x%04lX\n", OffSet);
      printf("Block       = %s\n", GET_BLOCK(_Csv, rCount));
      printf("_width      = %s\n", _bfList[bCount]._width);
      printf("_rw         = %s\n", _bfList[bCount]._rw);
      printf("_name       = %s\n", _bfList[bCount]._name);
      printf("_bits       = %s\n", _bfList[bCount]._bits);
      printf("<\n");
      printf("\n");
#endif
    }
    _NameHandle(&_bfList, bCount);
    _RWHandle(&_bfList, bCount);
    //
    // Inverted output
    //
    while (bCount--)
      fprintf(_Stream, "    %s uint32_t %s\t: %s;\t// %s\n", _bfList[bCount]._rw, _bfList[bCount]._name, _bfList[bCount]._width, _bfList[bCount]._bits);

    fprintf(_Stream, "} %s_%04lX_TypeDef;\n", Block, OffSet);
    fprintf(_Stream, "\n");
    fprintf(_Stream, "typedef union {\n");
    fprintf(_Stream, "    uint32_t\tall;\n");
    fprintf(_Stream, "    %s_%04lX_TypeDef\tbit;\n", Block, OffSet);
    fprintf(_Stream, "} %s_%04lX_REG;\n", Block, OffSet);
    fprintf(_Stream, "\n");
    fprintf(_Stream, "#define %s_%04lX ((%s_%04lX_REG *)((uint32_t)SVP_REG_ADDR_ARM_2_MIPS(_TV303_%s_%04lX)))\n", Block, OffSet, Block, OffSet, Block, OffSet);
    fprintf(_Stream, "\n");
#if (DEBUG == 1)
    printf("bCount      = %ld\n", bCount);
    printf("=====================================\n");
#endif
  }
}

/*********************************************************************
*
*       _CompareFilenameExt
*
*  Function description
*    Checks if the given filename has the given extension
*    The test is case-sensitive, meaning:
*    _CompareFilenameExt("Index.csv", ".csv")           ---> Match
*    _CompareFilenameExt("Index.CSV", ".csv")           ---> Mismatch
*    _CompareFilenameExt("Index.csv", ".CSV")           ---> Mismatch
*
*  Parameters
*    sFilename     Null-terminated filename, such as "Index.csv"
*    sExtension    Null-terminated filename extension with dot, such as ".csv"
*
*  Return value
*     0   match
*  != 0   mismatch
*/
static char _CompareFilenameExt(const char * sFilename, const char * sExt) {
  int LenFilename;
  int LenExt;
  int i;
  char c0;
  char c1;

  LenFilename = strlen(sFilename);
  LenExt = strlen(sExt);
  if (LenFilename < LenExt) {
    return 0;                     // mismatch
  }
  for (i = 0; i < LenExt; i++) {
    c0 = *(sFilename + LenFilename -i -1);
    c1 = *(sExt + LenExt -i -1);
    if (c0 != c1) {
      return 0;                   // mismatch
    }
  }
  return 1;
}

/*********************************************************************
*
*       _IsCSV
*/
static int _IsCSV(const char * Filename) {
  int r;
  r = _CompareFilenameExt(Filename, ".csv");
  return r;
}

/*********************************************************************
*
*       _PrintUsage()
*
*  Function description
*    Prints the utility's usage information.
*/
static void _PrintUsage(void) {
  printf("\n\nBin2C.exe (c) 2002 - 2015 V-SILICON Co.,Ltd. --- www.v-silicon.com\n");
  printf("Usage:\n");
  printf("csv2bf <infile> <outfile>\n");
  printf("where <infile>  is the input binary (or text) file (with extension) and\n");
  printf("      <outfile> is the name (without extension) of the .c and .h files to create.\n");
  printf("Example:\n");
  printf("csv2bf index.csv index\n");
}

/*********************************************************************
*
*       main
*/
void main(int argc, char *argv[]) {
  FILE *infile;
  FILE *outh;
  _CSV_T csv;
  struct stat st;
  char *infileBuffer;

  //
  // Print Usage
  //
  if (argc < 3) {
    _PrintUsage();
    exit(0);
  }
  //
  // Check file ext
  //
  if (!_IsCSV(argv[1])) {
    perror(argv[1]);
    exit(-1);
  }
  //
  // Get file size
  //
  if (stat(argv[1], &st)) {
    perror(argv[1]);
    exit(-1);
  }
  //
  // Open input file
  //
  infile = fopen(argv[1], "rb");
  if (infile == NULL) {
    perror(argv[1]);
    exit(-1);
  }
  //
  // Create a buffer for reading files
  //
  infileBuffer = malloc(st.st_size + 1);
  if (infileBuffer == NULL) {
    perror(argv[1]);
    exit(-1);
  }
  infileBuffer[st.st_size] = '\0';
  //
  // Read file to buffer
  //
  fread(infileBuffer, sizeof(char), st.st_size, infile);
  if (ferror(infile)) {
    free(infileBuffer);
    fclose(infile);
    exit(-1);
  }
  //
  // Parse CSV file
  //
  csv = _csvParse(infileBuffer);
#if (DEBUG == 1)
      printf("\n");
      printf(">\n");
      printf("Clen      = %d\n", csv->_clen);
      printf("Rlen      = %d\n", csv->_rlen);
      printf("<\n");
      printf("\n");
#endif

  _CSV_XY_T temp;
  temp = _xyGet(csv,STR_IP_NAME);
  if (temp._col != -1)
    _csvIp = temp._col;
  
  temp = _xyGet(csv,STR_BLOCK);
  if (temp._col != -1)
    _csvBlock = temp._col;

  temp = _xyGet(csv,STR_ADDRESS);
  if (temp._col != -1)
    _cavAddress = temp._col;

  temp = _xyGet(csv,STR_BITS);
  if (temp._col != -1)
    _csvBits = temp._col;

  temp = _xyGet(csv,STR_REG_NAME);
  if (temp._col != -1)
    _csvRegname = temp._col;

  temp = _xyGet(csv,STR_R_W);
  if (temp._col != -1)
    _csvRw = temp._col;

  temp = _xyGet(csv,STR_WIDTH);
  if (temp._col != -1)
    _csvWidth = temp._col;

#if (DEBUG == 1)
  printf("_csvIp      = %d\n", _csvIp);
  printf("_csvBlock   = %d\n", _csvBlock);
  printf("_cavAddress = %d\n", _cavAddress);
  printf("_csvBits    = %d\n", _csvBits);
  printf("_csvRegname = %d\n", _csvRegname);
  printf("_csvRw      = %d\n", _csvRw);
  printf("_csvWidth   = %d\n", _csvWidth);
#endif

  //
  // Isolate the file name for use as a macro name in the .h file
  //
  _ParsePath(outpname, outfname, argv[2]);
  strcpy(outhname, argv[2]);
  strcat(outhname, ".h");
  outh = fopen(outhname, "wt");
  if (outh == NULL) {
    perror(outhname);
    fclose(infile);
    exit(-1);
  }

  //
  // Get year
  //
  time_t tt     = time(NULL);
	struct tm *t  = localtime(&tt);

  //
  // Write header file
  //
  fprintf(outh, "/*********************************************************************\n");
  fprintf(outh, "----------------------------------------------------------------------\n");
  fprintf(outh, "File    : %s\n", outhname);
  fprintf(outh, "Purpose : Automatically created from %s using csv2bf\n", argv[1]);
  fprintf(outh, "Date    : %s\n", VERSION_DATE);
  fprintf(outh, "Time    : %s\n", VERSION_TIME);
  fprintf(outh, "---------------------------END-OF-HEADER------------------------------\n");
  fprintf(outh, "*/\n");
  fprintf(outh, "\n");
  fprintf(outh, "#ifndef __%s_H__\n", _Strupr(outfname));
  fprintf(outh, "#define __%s_H__\n", _Strupr(outfname));
  fprintf(outh, "\n");
  fprintf(outh, "#ifdef __cplusplus\n");
  fprintf(outh, "  extern \"C\" {\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "\n");
  fprintf(outh, "/*********************************************************************\n");
  fprintf(outh, "*\n");
  fprintf(outh, "*       #include Section\n");
  fprintf(outh, "*\n");
  fprintf(outh, "**********************************************************************\n");
  fprintf(outh, "*/\n");
  fprintf(outh, "#include <stdint.h>\n");
  fprintf(outh, "#include <mmap.h>\n");
  fprintf(outh, "\n");
  fprintf(outh, "/*********************************************************************\n");
  fprintf(outh, "*\n");
  fprintf(outh, "*       Defines, fixed\n");
  fprintf(outh, "*\n");
  fprintf(outh, "**********************************************************************\n");
  fprintf(outh, "*/\n");
  fprintf(outh, "\n");
  fprintf(outh, "//\n");
  fprintf(outh, "// addr offset\n");
  fprintf(outh, "//\n");
  fprintf(outh, "#ifndef  SVP_REG_BASE_MIPS\n");
  fprintf(outh, "#define  SVP_REG_BASE_MIPS     (0xB5000000)\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "\n");
  fprintf(outh, "//\n");
  fprintf(outh, "// reg addr uncache remap\n");
  fprintf(outh, "//\n");
  fprintf(outh, "#ifndef  SVP_REG_UNCACHE\n");
  fprintf(outh, "#define  SVP_REG_UNCACHE(addr) ((addr) | 0xA0000000U)\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "\n");
  fprintf(outh, "#ifndef  SVP_REG_ADDR_ARM_2_MIPS\n");
  fprintf(outh, "#define  SVP_REG_ADDR_ARM_2_MIPS(addr)    (SVP_REG_UNCACHE(SVP_REG_BASE_MIPS + addr))\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "\n");
  fprintf(outh, "//\n");
  fprintf(outh, "// Defines 'read only' permissions\n");
  fprintf(outh, "//\n");
  fprintf(outh, "#ifndef  __I\n");
  fprintf(outh, "#define  __I     volatile const\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "//\n");
  fprintf(outh, "// Defines 'write only' permissions\n");
  fprintf(outh, "//\n");
  fprintf(outh, "#ifndef  __O\n");
  fprintf(outh, "#define  __O     volatile\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "//\n");
  fprintf(outh, "// Defines 'read / write' permissions\n");
  fprintf(outh, "//\n");
  fprintf(outh, "#ifndef  __IO\n");
  fprintf(outh, "#define  __IO    volatile\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "\n");
  fprintf(outh, "//\n");
  fprintf(outh, "// %s base addr\n", GET_IP(csv, CSV_DATA_START));
  fprintf(outh, "//\n");
  fprintf(outh, "#define  _TV303_%s_BASE\t0x%08XU", GET_IP(csv, CSV_DATA_START), _Addrtoui(GET_ADDRESS(csv, CSV_DATA_START)));
  fprintf(outh, "\n");
  fprintf(outh, "\n");
  fprintf(outh, "/*********************************************************************\n");
  fprintf(outh, "*\n");
  fprintf(outh, "*       Bit Field Structure\n");
  fprintf(outh, "*\n");
  fprintf(outh, "**********************************************************************\n");
  fprintf(outh, "*/\n");
  fprintf(outh, "\n");
  fprintf(outh, "#pragma pack( push, 4 )\n");
  fprintf(outh, "\n");
  _WriteStructFile(outh, csv);
  fprintf(outh, "#pragma pack( pop )\n");
  fprintf(outh, "\n");
  fprintf(outh, "\n");
  fprintf(outh, "#ifdef __cplusplus\n");
  fprintf(outh, "  };\n");
  fprintf(outh, "#endif\n");
  fprintf(outh, "#endif  //__%s_H__\n", _Strupr(outfname));
  //
  // Write c header file, footer
  //
  fprintf(outh, "\n");
  fprintf(outh, "/****** End Of File *************************************************/\n");
  //
  // Cleanup: close files
  //
  _csvDelete(csv);
  free(infileBuffer);
  fclose(infile);
  fclose(outh);
}


