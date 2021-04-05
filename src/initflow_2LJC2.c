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

#include "include/prototypes_LJC2.h"

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
/* ActiveList *CAList, 
VentArr *vent, */
Vent *vent,
int num_vents,
unsigned int *CAListSize,
/* unsigned int *aCT, */
double *gridInfo) 
{

	unsigned int maxCellsPossible = 0;
	ActiveList *local_CAList;
	int i;
	
	/*Create the active lava list array . 
	if (CAList == NULL) {*/
		maxCellsPossible = gridInfo[4] * gridInfo[2];
		if (maxCellsPossible > 10e6) maxCellsPossible = 10e6;
		*CAListSize = maxCellsPossible;
		local_CAList = ACTIVELIST_INIT(*CAListSize); 
		if (local_CAList == NULL) return NULL;
/*		CAList = local_CAList; */
		fprintf(stderr, "Allocating Memory for Active Cell List, size = %u ", *CAListSize);
	/*}*/
	
	/* Do not put vent(s) on active list 
	Initialize vents, assign vent its grid location*/
	for (i = 0; i < num_vents; i++) { 
	  (vent+i)->row = (int) (((vent+i)->northing - gridInfo[3]) / gridInfo[5]); /* Row (Y) of vent cell*/
	  (vent+i)->col = (int) (((vent+i)->easting - gridInfo[0]) / gridInfo[1]); /* Col (X) of vent cell*/
	  /* grid[(vent+i)->row][(vent+i)->col].active = 0; */
	}
	//CAList->row = (int) ( (vent->northing - gridInfo[3]) / gridInfo[5]); /* Row (Y) of new cell*/
	//CAList->col = (int) ((vent->easting - gridInfo[0]) / gridInfo[1]); /* Col (X) of new cell*/
  //grid[CAList->row][CAList->col].active = 0;

	return local_CAList;
}
