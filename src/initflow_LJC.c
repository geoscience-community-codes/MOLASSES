/*############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.MOLASSES 
#
#    Copyright (C) 2015-2020  
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
Automata * or NULL on error
*/
Automata *INIT_FLOW (
DataCell **grid,
Automata *CAList, 
VentArr *vent,
unsigned int *CAListSize,
unsigned int *aCT,
double *gridInfo) 
{

	unsigned int maxCellsPossible = 0;
	Automata *local_CAList;
  
	
	
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
	CAList->row = (int) ( ( vent->northing - gridInfo[3]) / gridInfo[5]);/* Row (Y) of new cell*/
	CAList->col = (int) ((vent->easting - gridInfo[0]) / gridInfo[1]);/* Col (X) of new cell*/
	CAList->vent = 1; /* This is a vent cell */
	CAList->elev_diff = 0;
	grid[CAList->row][CAList->col].active = 0;

#ifdef PRINT2  
	fprintf(stderr, "\n\nVent cell initialized:\n");
	fprintf(stderr, "  [AL=%u] (%d,%d) %15.3f m^3, elev=%.2f\n",
	(unsigned int) *aCT, CAList->row, CAList->col, vent->currentvolume, grid[CAList->row][CAList->col].eff_elev);
#endif	

/* Increment active list counter */
	*aCT = 1;
	return CAList;
}
