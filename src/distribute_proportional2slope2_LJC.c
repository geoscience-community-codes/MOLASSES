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

/*DISTRIBUTE_PROPORTIONAL2SLOPE_LJC
		Determine how much lava to share between neighbors PROPORTIONAL to elevation difference
		between active cell and neghboring cells.
		The active Cell will SHARE ALL VOLUME of lava it contains ABOVE its RESIDUAL VOLUME.
*/
/*Module: DISTRIBUTE_proportional2slope_LJC
INPUTS:
dataCell grid - a 2D Global Data Grid
Automata activeList - a cellular automata list of active cells
unsigned int activeCount  - the number of elements within activeList
double gridMetadata - geometry of the Global Data Grid
int parents - 1(yes) or 0(no) to indicate if active cell is giving lava back to parent cell
double residual
          
Algorithm:
	Do While there are more cells in ActiveList:
		Calculate excess lava in active cell
		IF active cell has lava to give:
			Identify neighboring cells (N S E W)
				For each neighbor (i.e., getting new lava):
					Update parent-code
					Add neighbor to update list
					Calculate amount of receiving lava 
					(lava received is proportional to elevation difference between active cell and neighbor)
					Add lava to eff_elev of neighbor cell
			Remove lava given from active cell
	For each active cell on UPDATE list:
		set previous elevation to current elevation
RETURN:
0 = SUCCESS
<0 = some ERROR

NEIGHBOR_ID:
Identifies lower-elevation, valid-to-spread-to cells around
the active cell.

Parent bit-code is 4-bits, denotes which cell(s) is/are the "parent"
If bit-0 = 1 then a parent cell is SOUTH
If bit-1 = 1 then a parent cell is EAST
If bit-2 = 1 then parent cell is NORTH
If bit-3 = 1 then parent cell is WEST

ACTIVATE:
Appends a cell to the Update List with Global Data Grid info
************************************************************/
int DISTRIBUTE( 
DataCell **grid,
Automata *activeList,
unsigned int *CAListSize,
unsigned int *activeCount,
Automata *activeNeighbor,
double *gridinfo,
Inputs *in,
VentArr *vent)
{

	int c = 0;
	int neighborCount = 0;              /* Number of lava-accepting cells in neighborhood */
	double total_elev_diff;
	double myResidual, thickness;
	int n = 0;  					/* neighbor counter*/
	double lavaOut, lavaIn;      /* lava volume to advect away from center cell    */
	unsigned char parentCode = 0;            /* bitwise parent-child relationship code         */
	Automata *more = NULL;
	int excess = 0; /* Initialize this DISTRIBUTE with no excess lava*/
	int active;
	int count = 0;
	int recount = 0;
 double threshold;
	

	
	do { /* for all active cells */
		myResidual = grid[(activeList+c)->row][(activeList+c)->col].residual;
		threshold = myResidual * 0.1;
		thickness = grid[(activeList+c)->row][(activeList+c)->col].eff_elev 
		          - grid[(activeList+c)->row][(activeList+c)->col].dem_elev;
		lavaOut = thickness - myResidual;			
		
		if (lavaOut > 0.0) { /* if this active cell has measurable lava to give */
			
			/* Find neighbor cells which are not parents and have lower elevation than active cell */		
			neighborCount = NEIGHBOR_ID(
									(activeList+c),		/*Automata Center Cell (parent))*/
									grid,							/*DataCell Global Data Grid */
									gridinfo,					/*double   grid data */
									activeNeighbor,		/* list for active neighbors */
									vent,						   /* VentArr* Pointer to the vent data structure */	
									&total_elev_diff); /* sum of elevation difference between neighbors and parents */
			
			/* If neighbors are found */
			if (neighborCount > 0) { 
				(activeList+c)->excess = 1;
								
				/* For each neighbor */				
				for (n = 0; n < neighborCount; n++) { 
						
					/* Get previous parentCode of neighbor from data grid */
					parentCode = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].parentcode;
			
						/* Find parent or additional parent of each neighbor cell */ 
					if ((activeNeighbor+n)->row > (activeList+c)->row) parentCode |= 1; /* this neighbor's parent is SOUTH */
					
					else if ((activeNeighbor+n)->col < (activeList+c)->col) parentCode |= 2; /*  this neighbor's parent is EAST */
						
					else if ((activeNeighbor+n)->row < (activeList+c)->row) parentCode |= 4; /* this neighbor's parent is NORTH */
						
					else if ((activeNeighbor+n)->col > (activeList+c)->col) parentCode |= 8; /* this neighbor's parent is WEST */
					
					/* Assign new parentCode to neighbor grid cell */
					grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].parentcode = (parentCode == 15) ? 0: parentCode;
					
					/* Check if this neighbor cell is active*/
					active = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active;
					
					if (active < 0) { /* If neighbor cell is not on active list */
					
						/* Add neighbor to active list */
						(activeList + *activeCount)->row = (activeNeighbor+n)->row;
						(activeList + *activeCount)->col = (activeNeighbor+n)->col;
						(activeList + *activeCount)->excess = 0; /* Initialize new AL member with no lava to give*/
						grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active = (int) *activeCount;
						*activeCount += 1;
						
						if (*activeCount == *CAListSize) { /* resize active list if more space is needed */
							fprintf (stderr, 
							"Number of cells = %u; active list out of memory (%u) reallocation happening .....\n", 
							(unsigned int) *activeCount, (unsigned int)*CAListSize);
							*CAListSize *= 2;
							more = (Automata *) GC_REALLOC( activeList, (size_t)(*CAListSize) * sizeof(Automata) );
							if (more != NULL) activeList = more;
								else {
								fprintf(stderr, "[DISTRIBUTE]\n");
								fprintf(stderr, 
								"   NO MORE MEMORY: Tried to re-allocate memory for activelist (%u)\n", 
								(unsigned int) *CAListSize);
								fflush(stderr);
								return 1;
							}
						}
					} /* END if (active < 0) Neighbor not on active list */ 
					
					/* Now Calculate the amount of lava this neighbor gets */				
					/* This neighbor gets lava proportional to the elevation difference with its parent */	
					
					if (total_elev_diff  > 0.0) {
						lavaIn = lavaOut * ((activeNeighbor+n)->elev_diff / total_elev_diff);
					}
					else {
						fprintf (stderr, 
						"PROBLEM: Cannot divide by zero or difference is less than 0: total_elev_diff= %f", 
						total_elev_diff);
						fflush(stderr);
						exit(1);
					}
					/* Distribute lava to neighbor */
					active = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active;
					grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev += lavaIn;
					myResidual = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].residual;
					thickness = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev
					          - grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].dem_elev;
					

					/* NOW, IF neighbor has excess lava and has not yet been flagged */ 
					if (thickness > (myResidual + threshold))
							/*Flag neighbor to distribute its excess lava */
							(activeList+active)->excess = 1;	
							
					} /* END for each neighbor */
				
				/*REMOVE ALREADY SPREAD LAVA FROM CENTER CELL**************************/
				/* Subtract lavaOut  from activeCell's  effective elevation*/
				grid[(activeList+c)->row][(activeList+c)->col].eff_elev -= lavaOut;
				(activeList+c)->excess = 0;
			} /*End (neighborCount > 0)*/
			
			else if (!neighborCount) { /* If a cell has lava to give but no neighbors */
				(activeList + c)->excess = 0;
			}
			
			else if (neighborCount < 0) { /* might be off the grid */
				fprintf(stdout, 
				"ERROR [DISTRIBUTE]:  neighbor count=%d\n", 
				neighborCount);
				return neighborCount;
			}
		} /*End IF lava is thick enough to advect */
		
		/* If active cell does not have any lava to give */
		else (activeList + c)->excess = 0;
		if ((activeList + c)->excess) excess++;
		c++; /*Move to next active cell */
		
		/* Pulse has ended, now finish spreading out lava */
		if (c == *activeCount) { 
		  if (count > 1000000) {
    	    break;
      } 
         /* Check and see if any cells still have excess lava */
			do {
           grid[(activeList+recount)->row][(activeList+recount)->col].prev_elev = grid[(activeList+recount)->row][(activeList+recount)->col].eff_elev;
    	     if ((activeList+recount)->excess) excess++;   	     
    	     recount++;
    	     count++;  		
			} while (recount < *activeCount); /*Keep looping until all active cells have been tested*/
			recount = 0;
			if (excess > 0) c = 0;	
      excess = 0;	
		} 
	} while (c < *activeCount); /*Keep looping until all active cells have been tested*/

	/*return 0 for a successful round of distribution.*/
/*	fflush(stderr);*/
	return 0;
}
