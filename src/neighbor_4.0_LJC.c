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
#define ONE_DIV_SQRT2 (1.0/sqrt(2.0))
#define DIAGONAL_DISTANCE(x,y,z) ((sqrt(((x)*(x))+((y)*(y))))/(z))
#define NORMALIZE_DISTANCE(x,y) ((x)/(y))

/**********************************
Module: NEIGHBOR_ID (4 Directions)
Identify 4-neighbors, List of Cells (4 long)

INPUTS:
Automata *active
DataCell **grid
double *gridMetadata
Automata *ActiveList
Automata *neighborList
VentArr *vent
	
RETURN:
(int) number of neighbors
	
Accepts a Cell structure (active cell location)
Allocate memory for the neighbor list array
Define column and row numbers for current cell and neighbors
If location is at global grid boundary:
return "off the grid" error, -1 to -4
	Off grid handling
	gridMetadata format:
		[0] lower left x
		[1] w-e pixel resolution
		[2] number of cols, assigned manually
		[3] lower left y
		[4] number of lines, assigned manually
		[5] n-s pixel resolution (negative value)
	
Else:
Find lower neighbors in cardinal directions:
	that aren't parent cells (using bitswitching: 0th bit = right;1=d;2=l;3=u
	if parentCode bit 3 is 0, WEST cell is not parent
	if parentCode bit 2 is 0, NORTH cell is not parent
	if parentCode bit 1 is 0, EAST cell is not parent
	if parentCode bit 0 is 0, SOUTH cell is not parent
	Some neighbors may have 2 parent cells. A neighbor with 2 parents has 2 bits set to 1
	i.e. 1100 = NORTH + WEST parent
	     1001 = SOUTH + WEST parent
	     0110 = NORTH + EAST parent
	     0011 = SOUTH + EAST parent
	
Update neighbor count
pass back to [DISTRIBUTE]
return neighbor count or < 0 on error

*******************************************/
int NEIGHBOR_ID(
Automata *active, 
DataCell **grid, 
double *gridMetadata,
Automata *neighborList,
VentArr *vent,
double *total_elev_diff) 
{
	unsigned char code;																/*Parent bitcode*/
	int Nrow, Srow, Wcol, Ecol, aRow, aCol;	/* neighbor Row and Col  relative to active (active cell) */
	int neighborCount = 0;									/* Initialize neighbor counter */
	*total_elev_diff = 0.0; 								/* initialize total elevation difference */
	
	/* Calculate row and column locations for active cell and its neighbor cells */
	aRow = active->row;
	aCol = active->col;
	Nrow = aRow + 1;
	Srow = aRow - 1;
	Ecol = aCol + 1;
	Wcol = aCol - 1;

	if (Srow < 0) {
		printf("\nFLOW IS OFF THE MAP! (South) [NEIGHBOR_ID]\n");
		/* *neighborCount = -1;
		return neighborList;*/
		return -1;
	} else if (Nrow >= gridMetadata[4]) {
		printf("\nFLOW IS OFF THE MAP! (North) [NEIGHBOR_ID]\n");
		/* *neighborCount = -2;
		return neighborList;*/
		return -2;
	} else if (Wcol < 0) {
		printf("\nFLOW IS OFF THE MAP! (West) [NEIGHBOR_ID]\n");
		/* *neighborCount = -3;
		return neighborList; */
		return -3;
	} else if (Ecol >= gridMetadata[2]) {
		printf("\nFLOW IS OFF THE MAP! (East) [NEIGHBOR_ID]\n");
		return -4;
	}
	
	/*NORTH neighbor*/
	code = grid[aRow][aCol].parentcode & 4;
	if (!code) { /* NORTH cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\t NORTH-Cell * ");
#endif
			if (grid[aRow][aCol].eff_elev > grid[Nrow][aCol].prev_elev) { /* active cell is higher than North neighbor */
				/* Calculate elevation difference between active cell and its North neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[Nrow][aCol].prev_elev;
				(neighborList+neighborCount)->row  = Nrow;
				(neighborList+neighborCount)->col  = aCol;
				*total_elev_diff += (neighborList+neighborCount)->elev_diff;
				(neighborList+neighborCount)->rise = NORMALIZE_DISTANCE((neighborList+neighborCount)->elev_diff, gridMetadata[1]); 
				neighborCount +=1;
			}
#ifdef PRINT4  
			else fprintf(stderr, "NORTH neighbor too high [%0.4f]\n", grid[Nrow][aCol].prev_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=NORTH[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
	/*SOUTH*/
	code = grid[aRow][aCol].parentcode & 1;
	if (!code) { /* SOUTH cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\t SOUTH-Cell * ");
#endif			
			if(grid[aRow][aCol].eff_elev > grid[Srow][aCol].prev_elev) { /* active cell is higher than SOUTH neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[Srow][aCol].prev_elev;
				(neighborList+neighborCount)->row  = Srow;
				(neighborList+neighborCount)->col  = aCol;
				*total_elev_diff += (neighborList+neighborCount)->elev_diff;
				(neighborList+neighborCount)->rise = NORMALIZE_DISTANCE((neighborList+neighborCount)->elev_diff, gridMetadata[1]);
				neighborCount +=1;
			}			
#ifdef PRINT4  
			else fprintf(stderr, "SOUTH neighbor too high [%0.4f]\n", grid[Srow][aCol].prev_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=SOUTH[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
	/*EAST*/
	code = grid[aRow][aCol].parentcode & 2;
	if (!code) { /* EAST cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\t EAST-Cell * ");
#endif
			if (grid[aRow][aCol].eff_elev > grid[aRow][Ecol].prev_elev) { /* active cell is higher than EAST neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[aRow][Ecol].prev_elev;
				(neighborList+neighborCount)->row  = aRow;
				(neighborList+neighborCount)->col  = Ecol;
				*total_elev_diff += (neighborList+neighborCount)->elev_diff;
				(neighborList+neighborCount)->rise = NORMALIZE_DISTANCE((neighborList+neighborCount)->elev_diff, gridMetadata[1]);
				neighborCount +=1;
			}			
#ifdef PRINT4  
			else fprintf(stderr, "EAST neighbor too high [%0.4f]\n", grid[aRow][Ecol].prev_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=EAST[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
	
	/*WEST*/
	code = grid[aRow][aCol].parentcode & 8;
	if (!code) { /* WEST cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\tWEST-Cell * ");
#endif			
			if(grid[aRow][aCol].eff_elev > grid[aRow][Wcol].prev_elev) {/* active cell is higher than WEST neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[aRow][Wcol].prev_elev;
				(neighborList+neighborCount)->row  = aRow;
				(neighborList+neighborCount)->col  = Wcol;
				*total_elev_diff += (neighborList+neighborCount)->elev_diff;
				(neighborList+neighborCount)->rise = NORMALIZE_DISTANCE((neighborList+neighborCount)->elev_diff, gridMetadata[1]);
				neighborCount +=1;
			}				
#ifdef PRINT4  
			else fprintf(stderr, "WEST neighbor too high [%0.4f]\n", grid[aRow][Wcol].prev_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=WEST[%d]\n", grid[aRow][aCol].parentcode);
#endif
  return neighborCount;
}
