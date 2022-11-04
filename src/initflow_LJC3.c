/*############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.
#
#    Copyright (C) 2015-2021  
#    Laura Connor (lconnor@usf.edu)
#    Jacob Richardson 
#    Charles Connor
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################*/ 

#include "include/prototypes_LJC3.h"
#include <limits.h>
/**************************************************
MODULE: INIT_FLOW
Create Cellular Automata Lists (ACTIVELIST_INIT)
Add vent as first cell on active List (AL)
	
INPUTS:
DataCell **grid
Automata *CAList 
VentArr *vent
int *CAListSize
int *aCT
double *gridInfo  - GDAL array:
	[0] lower left x
	[1] w-e pixel resolution, column size
	[2] number of columns
	[3] lower left y
	[4] number of lines/rows
	[5] n-s pixel resolution, row size

RETURN:
ActiveList * or NULL on error
****************************************/
ActiveList *INIT_FLOW (
DataCell **grid,
//Vent *vent,
//int num_vents,
int *CAListSize,
double *gridInfo) 
{
	int maxCellsPossible = 0;
	ActiveList *local_CAList = NULL;
	
	/* Create the active lava list array 
     Reserves memory for active list  
  */
	maxCellsPossible = (int)(gridInfo[4] * gridInfo[2]);
	if (maxCellsPossible > 1e7 ) maxCellsPossible = 1e7;
		
	//local_CAList = ACTIVELIST_INIT(*CAListSize); 
	local_CAList = (ActiveList*) GC_MALLOC( maxCellsPossible * sizeof(ActiveList) );
	if (local_CAList == NULL) 
	{
		fprintf(stderr, "[ACTIVELIST_INIT]\n");
		fprintf(stderr, "   NO MORE MEMORY: Tried to allocate memory Active Lists!! Program stopped!\n");
		return NULL;
	}
  *CAListSize = maxCellsPossible;
		
	fprintf(stderr, "Allocating Memory for Active Cell List, Active list size: %d, uint max: %u, int max: %d\n", *CAListSize, (unsigned int)UINT_MAX, (int)INT_MAX);

	
	/* Do not put vent(s) on active list 
	Initialize vents, assign vent its grid location
	for (i = 0; i < num_vents; i++) { 
	  (vent+i)->row = (int) (((vent+i)->northing - gridInfo[3]) / gridInfo[5]); // Row (Y) of vent cell
	  (vent+i)->col = (int) (((vent+i)->easting - gridInfo[0]) / gridInfo[1]); // Col (X) of vent cell
	  grid[(vent+i)->row][(vent+i)->col].active = 0; 
	} */
	//CAList->row = (int) ( (vent->northing - gridInfo[3]) / gridInfo[5]); /* Row (Y) of new cell*/
	//CAList->col = (int) ((vent->easting - gridInfo[0]) / gridInfo[1]); /* Col (X) of new cell*/
  //grid[CAList->row][CAList->col].active = 0;

	return local_CAList;
}

