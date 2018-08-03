/*main.c*/

// Chicago Crime Lookup via hashing and AVL trees.
// Bushra Baqui
// Sublime text using bert to test
// U. of Illinois, Chicago
// CS251, Fall 2016
// HW #9

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "avl.h"
#include "mymem.h"


// Global variables:
int g_collisions = 0;       // instrument # of collisions for reporting:


//freeAVLTree: frees AVLTree by PostOrder
void freeAVLTree(AVLNode *root)
{
	if (root == NULL)         // base case: empty tree
    	return;
  	else                      // recursive case: non-empty tree
  	{
	    freeAVLTree(root->left);
	    freeAVLTree(root->right);
	    myfree(root);
  	}
}


// Hash function:
int hash(char *CaseNum, int N)
{
	// case numbers are 7 or 8 chars long, with the last 6 as digits.  some examples:
	// HZ256372, HY266148, HY299741, HH254987, G219399.  Since there could be millions 
	// of cases, 6 digits are not enough.  so we need to take advantage of the letters 
	// too...
	int len = strlen(CaseNum);

	if (len < 8 || len > 9)       // invalid, e.g. perhaps user enters an invalid case #:
		return -1;

	int advance = len - 6;        // come from the end in case there are more than 2 digits to start:
	char *p = CaseNum + advance;  // ptr to first digit:

	int i = atoi(p);

	assert(i > 0 && i < 1000000);  // 6 meaningful digits:

	if (len > 7)                  // use the 2nd letter:
	{
		char c = CaseNum[1];

		int diff = abs('Z' - c);
		i = (diff * 1000000) + i;
	}

	// return hash value:
	return i % N;                 // whatever we have, make sure it falls within 0..N-1:
}


// parseCrime:
// parses given line and stores the fields in the given Crime struct:
void parseCrime(char *line, Crime *c)
{
	// format:
	// Case Number, Date, IUCR, Arrest, Domestic, Beat, District, Ward, Community Area, Year
	// HZ256372, 01/01/2015 12:00:00 AM, 0281, false, false, 0334, 003, 7, 43, 2015
	// HZ257172, 11/24/2015 05:30:00 PM, 0820, false, false, 1124, 011, 28, 27, 2015
	// HY266148, 05/19/2015 01:12:00 AM, 0560, true, false, 1933, 019, 44, 6, 2015
	char *token;

	// case number:
	token = strtok(line, ",");
	assert(token != NULL);
	strcpy(c->caseNum, token);

	// datetime:
	token = strtok(NULL, ",");
	assert(token != NULL);
	strcpy(c->dateTime, token);

	// IUCR:
	token = strtok(NULL, ",");
	assert(token != NULL);
	strcpy(c->IUCR, token);

	// arrested:
	token = strtok(NULL, ",");
	assert(token != NULL);
	if (strcmp(token, "true") == 0)
		c->arrested = TRUE;
	else
		c->arrested = FALSE;

	// domestic:
	token = strtok(NULL, ",");
	assert(token != NULL);
	if (strcmp(token, "true") == 0)
		c->domestic = TRUE;
	else
		c->domestic = FALSE;

	// beat: --- skip
	token = strtok(NULL, ",");
	assert(token != NULL);
	// district: --- skip
	token = strtok(NULL, ",");
	assert(token != NULL);
	// ward: --- skip
	token = strtok(NULL, ",");
	assert(token != NULL);

	// area:
	token = strtok(NULL, ",");
	assert(token != NULL);
	c->area = atoi(token);

	assert(c->area >= 0 && c->area <= 77);

	// year:
	token = strtok(NULL, ",");
	assert(token != NULL);
	c->year = atoi(token);

	token = strtok(NULL, ",");
	assert(token == NULL);    // no more tokens on this line:
}


// inputCrimes
// Inputs the crimes from the given file, building and storing these crimes 
// in a hash table.  Chaining is used to deal with collisions.  A pointer
// to the hash table is returned; the size of the hash table is also returned
// via the 2nd function argument.
Crime **inputCrimes(char *filename, int *ht_size, int *crimeCount)
{
	FILE *input;
	char  line[256];
	int   linesize = sizeof(line) / sizeof(line[0]);
	int   startYear, endYear;
	int crimeCount2 = 0;

	// open file, make sure it worked:
	input = fopen(filename, "r");
	if (input == NULL)
	{
		printf("**Error: unable to open '%s'.\n\n", filename);
		exit(-1);
	}

	// read the range of years from 1st line:
	printf(">> reading:   '%s'\n", filename);
	fscanf(input, "%d %d", &startYear, &endYear);
	fgets(line, linesize, input);  // discard rest of line

	printf(">> years:      %d..%d\n", startYear, endYear);

	// allocate space for hash table: assume 300,000 crimes/year, so compute size
	// we need with a 20% load factor (i.e. 80% unused):
	int years = endYear - startYear + 1;
	int N = years * 300000 * 5 /*load factor, 5 => 20%*/;

	Crime **hashtable = (Crime **)mymalloc(N * sizeof(Crime));
	if (hashtable == NULL)  // alloc failed :-(
	{
		printf("**Error: unable allocate memory for hash table (%d).\n\n", N);
		exit(-1);
	}

	// initialize hash table entries to NULL, i.e. empty chains:
	for (int i = 0; i < N; ++i)
		hashtable[i] = NULL;

	// now start reading the crime data:
	fgets(line, linesize, input);            // discard 2nd line --- column headers

	while (fgets(line, linesize, input))    // start reading data:
	{
		line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

		// allocate memory for this crime:
		Crime *c = (Crime *)mymalloc(sizeof(Crime));
    	if (c == NULL)  // alloc failed :-(
    	{
    		printf("**Error: unable allocate memory for Crime struct.\n\n");
    		exit(-1);
		}

		// fill crime struct with data from file:
		parseCrime(line, c);

		// link into hashtable:
		int index = hash(c->caseNum, N);

		if (hashtable[index] != NULL)
			g_collisions++;

	    c->next = hashtable[index];       // existing chain follows us:
	    hashtable[index] = c;             // insert @ head:

    	crimeCount2++;
	}

	fclose(input);

	// stats:
	printf(">> # crimes:   %d\n", crimeCount2);
	printf(">> ht size:    %d\n", N);
	printf(">> collisions: %d\n", g_collisions);

	// return hash table pointer and size:
	*ht_size = N;
	*crimeCount=crimeCount2;

	return hashtable;
}


// inputAreas
// Reads the file of chicago community areas, of which there should be 78.
// Returns pointer to array of community names.
char **inputAreas(char *filename)
{
	FILE *input;
	char  line[256];
	int   linesize = sizeof(line) / sizeof(line[0]);

	// open file, check to make sure it worked:
	input = fopen(filename, "r");
	if (input == NULL)
	{
		printf("**Error: unable to open '%s'.\n\n", filename);
		exit(-1);
	}

	printf(">> reading:   '%s'\n", filename);

	// allocate array for community names:
	int N = 78;
	char **areas = (char **)mymalloc(N * sizeof(char *));

	// now read the names and fill the array:
	fgets(line, linesize, input);         // discard 1st line --- column headers

	int count = 0;

	while (fgets(line, linesize, input))  // start reading data:
	{
		char *token;

    	line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

		// format:  area #, area name
		token = strtok(line, ",");
		assert(token != NULL);

		int index = atoi(line);

		token = strtok(NULL, ",");
		assert(token != NULL);

		int len = strlen(token) + 1;
		char *p = (char *)mymalloc(len * sizeof(char));
		strcpy(p, token);

		areas[index] = p;

		token = strtok(NULL, ",");
		assert(token == NULL);            // no more tokens on this line:

		count++;
	}

	assert(count == N);

	fclose(input);

	// return areas array pointer and size:
	return areas;
}


// inputCodes: Reads the file codes.csv and stores the data.
AVLNode *inputCodes(char *filename, AVLElementType new)
{
	FILE *input;
	char  line[256];
	int   linesize = sizeof(line) / sizeof(line[0]);

	// open file, make sure it worked:
	input = fopen(filename, "r");
	if (input == NULL)
	{
	    printf("**Error: unable to open '%s'.\n\n", filename);
	    exit(-1);
	}

	fgets(line, linesize, input);         // discard rest of line

	AVLNode *root = NULL;

	while (fgets(line, linesize, input))  // start reading data:
	{
		line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

		char *token;

		// IUCR:
		token = strtok(line, ",");
		assert(token != NULL);
		strcpy(new.IUCR, token);

		// Primary description:
		token = strtok(NULL, ",");
		assert(token != NULL);
		strcpy(new.Prim_Desc, token);

		// Secondary description:
		token = strtok(NULL, ",");
		assert(token != NULL);
		strcpy(new.Second_Desc, token);

		root = InsertByIUCR(root, new);
	}

	fclose(input);

	return root;
}


// parses, stores and counts the area and iucr
void parseAndStore(char *filename, int areaNum, char *IUCR, int *tempareaNum, int *tempIUCR, int *tempIUCR_Area)
{
	FILE *input;
	char  line[256];
	int   linesize = sizeof(line) / sizeof(line[0]);
	int   totalArea = 0;
	int   totalIUCR = 0;
	int   totalIUCR_Area = 0;
  
	// open file, make sure it worked:
	input = fopen(filename, "r");
  
	if (input == NULL)
	{
    	printf("**Error: unable to open '%s'.\n\n", filename);
    	exit(-1);
	}

	fgets(line, linesize, input);  // discard rest of line

	// now start reading the crime data:
	fgets(line, linesize, input);         // discard 2nd line --- column headers

	while (fgets(line, linesize, input))  // start reading data:
	{
		line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

		// format:
		// Case Number, Date, IUCR, Arrest, Domestic, Beat, District, Ward, Community Area, Year
		// HZ256372, 01/01/2015 12:00:00 AM, 0281, false, false, 0334, 003, 7, 43, 2015
		// HZ257172, 11/24/2015 05:30:00 PM, 0820, false, false, 1124, 011, 28, 27, 2015
		// HY266148, 05/19/2015 01:12:00 AM, 0560, true, false, 1933, 019, 44, 6, 2015
		char *token;

		// case number:--- skip
		strtok(line, ",");

		// datetime:--- skip
		strtok(NULL, ",");

		// IUCR:
		token = strtok(NULL, ",");
		assert(token != NULL);
		char *IUCR2 = token;

		if(strcmp(IUCR2, IUCR) == 0)
			totalIUCR++;

		// arrested:--- skip
		strtok(NULL, ",");

		// domestic:--- skip
		strtok(NULL, ",");

		// beat: --- skip
		strtok(NULL, ",");

		// district: --- skip
		strtok(NULL, ",");

		// ward: --- skip
		strtok(NULL, ",");

		// area:
		token = strtok(NULL, ",");
		assert(token != NULL);
		int areaNum2 = atoi(token);

		if (areaNum2 == areaNum)
			totalArea++;

		assert(areaNum >= 0 && areaNum <= 77);

		if((strcmp(IUCR2, IUCR) == 0) && (areaNum2 == areaNum))
			totalIUCR_Area++;

		// year:--- skip
		strtok(NULL, ",");

		token = strtok(NULL, ",");
		assert(token == NULL);  // no more tokens on this line:
	}

	fclose(input);

	*tempareaNum = totalArea;
	*tempIUCR = totalIUCR;
	*tempIUCR_Area = totalIUCR_Area;

	return;
}


// outputCrimeInfo:
void outputCrimeInfo(Crime *c, char *areas[], AVLNode *root, int crimeCount)
{
	printf("%s:\n", c->caseNum);
	printf("  date/time: %s\n", c->dateTime);
	printf("  city area: %d => %s\n", c->area, areas[c->area]);

	//traversing to find the associated iucr to casenum
	AVLNode *cur = root;
	AVLNode *prev = NULL;

	while(cur != NULL)
	{
		if(strcmp(c->IUCR, cur->value.IUCR) == 0)
		{
			prev = cur;
			break;
		}
		else if (strcmp(c->IUCR, cur->value.IUCR) < 0)  //smaller, go left
    		cur = cur->left;
		else //larger, go right
    		cur = cur->right;
	}
  
	//checking for UNKNOWN condition
	if(cur == NULL)
	{
		printf("  IUCR code: %s => UNKNOWN\n", c->IUCR);
	}
	else
	{
		printf("  IUCR code: %s => %s: %s\n", c->IUCR, prev->value.Prim_Desc, prev->value.Second_Desc); 
	}
	
	printf("  arrested:  %s\n", ((c->arrested) ? "true" : "false"));
	printf("  domestic violence:  %s\n\n", ((c->domestic) ? "true" : "false"));

	printf("AREA: %d => %s\n", c->area, areas[c->area]);

	int tempareaNum = 0;
	int tempIUCR = 0;
	int tempIUCR_Area = 0;
	int tempAreaNum =0;
	parseAndStore("Crimes.csv", c->area, c->IUCR, &tempareaNum, &tempIUCR, &tempIUCR_Area);
  
	printf("  # of crimes in area: %d\n", tempareaNum);
	double perAreaCrime = (double) (tempareaNum * 100)/crimeCount;
	printf("  %% of chicago crime: %f%%\n\n", perAreaCrime);

	if(cur != NULL)
	{
    	printf("CRIME: %s => %s: %s\n", prev->value.IUCR, prev->value.Prim_Desc, prev->value.Second_Desc);  
	}
	else
	{
    	printf("CRIME: %s => UNKNOWN\n", c->IUCR);
	}
 
	printf("  # of THIS crime in area: %d\n", tempIUCR_Area);

	double perIucrCrime = (double) (tempIUCR_Area * 100)/tempareaNum;
	printf("  %% of THIS crime in area: %f%%\n", perIucrCrime);
  
	double perIucrCrimeChicago = (double) (tempIUCR * 100)/crimeCount;
	printf("  %% of THIS crime in Chicago: %f%%\n\n", perIucrCrimeChicago);
}

/*****************************************************************************************************************************************/

// Main:
int main()
{
	printf("\n** Chicago Crime Lookup Program **\n\n");

	Crime **hashtable;  // array of Crime pointers:
	int     N;          // size of hash table
	char  **areas;      // array of community names (strings):
	AVLNode *root;
	AVLElementType new;
	int crimeCount=0;
  
	areas = inputAreas("Areas.csv");
	hashtable = inputCrimes("Crimes.csv", &N, &crimeCount);
  
	root = inputCodes("Codes.csv", new);

	printf("\n");

	// crime lookup loop:
	char line[256];
	int  linesize = sizeof(line) / sizeof(line[0]);

	printf("Enter a case number> ");
	fgets(line, linesize, stdin);
	line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:

	while (strlen(line) > 0)
	{
    	int index = hash(line, N);

	    if (index < 0)
	    {
	    	printf("** invalid case #, try again...\n");
	    }
	    else
	    {
	    	printf(">> hash index: %d <<\n", index);

    		// walk along the chain and see if we can find this case #:
    		Crime *c = hashtable[index];

    		while (c != NULL)
    		{
        		if (strcmp(line, c->caseNum) == 0)  // found it!
          		break;
        
		        // otherwise keep looking:
		        c = c->next;
    		}

    		// if get here, see if we found it:
    		if (c == NULL)
        		printf("** Case not found...\n");
      		else
        		outputCrimeInfo(c, areas, root, crimeCount);
    	}

		printf("\n");
		printf("Enter a case number> ");
		fgets(line, linesize, stdin);
		line[strcspn(line, "\r\n")] = '\0';  // strip EOL(s) char at the end:
	}

	// done:
	printf("\n** Done **\n\n");

	mymem_stats();

	printf("** Freeing memory...\n");

	//free areas
	int i;
	for (i = 0; i < 78; ++i)
	{
    	myfree(areas[i]);
  	}
	myfree(areas);

	//free hashtable
	Crime *curr_temp = NULL;
	Crime *prev_temp = NULL;

	for(i = 0; i < N; ++i)
	{
    	curr_temp = hashtable[i];
      
    	while(curr_temp != NULL)
    	{
			prev_temp = curr_temp;
			curr_temp = curr_temp->next;
			myfree(prev_temp);
		}
	} 
	myfree(hashtable);
 
	//free AVLtree
	freeAVLTree(root);

	mymem_stats();

	printf("\n\n");

	return 0;
}
