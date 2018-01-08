#include "include/prototypes_LJC.h"

/*Reserves memory for active list with size [CAListSize] With the typedef of Automata. */
Automata *ACTIVELIST_INIT(
unsigned int CAListSize)
{
	Automata *m = NULL;
	
	/*Allocate active list*/
	m = (Automata*) GC_MALLOC( (size_t)(CAListSize) * sizeof(Automata) );
	if (m == NULL) 
	{
		fprintf(stderr, "[ACTIVELIST_INIT]\n");
		fprintf(stderr, "   NO MORE MEMORY: Tried to allocate memory Active Lists!! Program stopped!\n");
		return NULL;
	}
	return (m);
}

/*Reserves memory for matrix of size [rows]x[cols] With the typedef of DataCell.*/
DataCell **GLOBALDATA_INIT(
int rows, 
int cols)
{
	int i;
	DataCell **m = NULL;
	
	/*Allocate row pointers*/
	if((m = (DataCell**) GC_MALLOC((size_t)(rows) * sizeof(DataCell*) )) == NULL)
	{
		fprintf(stderr, "[GLOBALDATA_INIT]\n");
		fprintf(stderr, "   NO MORE MEMORY: Tried to allocate memory for %d Rows!! Program stopped!\n", rows);
		return NULL;
	}
	/*allocate cols & set previously allocated row pointers to point to these*/
	for (i = 0; i < rows; i++) 
	{
		if((m[i] = (DataCell*) GC_MALLOC((size_t)(cols) * sizeof(DataCell) )) == NULL)
		{
			fprintf(stderr, "[GLOBALDATA_INIT]\n");
			fprintf(stderr, "   NO MORE MEMORY: Tried to allocate memory for %d cols in %d rows!! Program stopped!", cols,rows);
			return NULL;
		}
	}
	return m; /*return array */
}
