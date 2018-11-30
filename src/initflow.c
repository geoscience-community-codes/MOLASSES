#include "include/prototypes_LJC.h"

/**************************************************
MODULE: INIT_FLOW
Create Cellular Automata Lists (ACTIVELIST_INIT)
Add vent as first cell on active List (AL)
	
INPUTS:
DataCell **grid
Automata *CAList 
VentArr *vent
unsigned int *CAListSize
unsigned int *aCT
double *gridInfo  - GDAL array:
	[0] lower left x
	[1] w-e pixel resolution, column size
	[2] number of columns
	[3] lower left y
	[4] number of lines/rows
	[5] n-s pixel resolution, row size

RETURN:
ActiveList * or NULL on error
*/
ActiveList *INIT_FLOW (
DataCell **grid,
ActiveList *CAList, 
VentArr *vent,
unsigned int *CAListSize,
unsigned int *aCT,
double *gridInfo) 
{

	unsigned int maxCellsPossible = 0;
	ActiveList *local_CAList;
	
	/*Create the active lava list array . */
	if (CAList == NULL) {
		maxCellsPossible = gridInfo[4] * gridInfo[2];
		if (maxCellsPossible > 10e6) maxCellsPossible = 10e6;
		*CAListSize = maxCellsPossible;
		local_CAList = ACTIVELIST_INIT(*CAListSize); 
		if (local_CAList == NULL) return NULL;
		CAList = local_CAList;
		fprintf(stderr, "Allocating Memory for Active Cell List, size = %u ", *CAListSize);
	}
	
	/* put vent as first cell on active lava list */
	CAList->row = (int) ( (vent->northing - gridInfo[3]) / gridInfo[5]); /* Row (Y) of new cell*/
	CAList->col = (int) ((vent->easting - gridInfo[0]) / gridInfo[1]); /* Col (X) of new cell*/
  grid[CAList->row][CAList->col].active = 0;

#ifdef PRINT2  
	fprintf(stderr, "\n\nVent cell initialized:\n");
	fprintf(stderr, "  [AL=%u] (%d,%d) \n",
	(unsigned int) *aCT, CAList->row, CAList->col);
#endif	

/* Increment active list counter */
	*aCT = 1; 

	return CAList;
}
