/*avl.h*/

// Bushra Baqui
// Sublime text using bert to test
// U. of Illinois, Chicago
// CS251, Fall 2016
// HW #9

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TRUE  1
#define FALSE 0

// Defines information about one crime:
typedef struct Crime
{
	char caseNum[10];
	char  dateTime[24];
	char  IUCR[5];
	int   arrested;
	int   domestic;
	int   area;
	int   year;
	struct Crime *next;
}Crime;

typedef struct AVLElementType
{
	char IUCR[5];
	char Prim_Desc[128];
	char Second_Desc[128];
}AVLElementType;

typedef struct AVLNode
{
	AVLElementType  value;
	int             height;
	struct AVLNode  *left;
	struct AVLNode  *right;
}AVLNode;

AVLNode *Contains(AVLNode *root, AVLElementType value);
AVLNode *InsertByIUCR(AVLNode *root, AVLElementType value);

int Count(AVLNode *root);
int Height(AVLNode *root);

void PrintInorder(AVLNode *root);

int hash(char *CaseNum, int N);
void parseCrime(char *line, Crime *c);
Crime **inputCrimes(char *filename, int *ht_size, int *crimtCount);
char **inputAreas(char *filename);
AVLNode *inputCodes(char *filename, AVLElementType new);
void parseAndStore(char *filename, int areaNum, char *IUCR, int *tempareaNum, int *tempIUCR, int *tempIUCR_Area);
void outputCrimeInfo(Crime *c, char *areas[], AVLNode *root, int crimeCount);

