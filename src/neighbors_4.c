#include "include/prototypes_LJC.h"
#define SQRT2 (sqrt(2))
#define DIAGONAL_DISTANCE(x,y,z) ((sqrt(((x)*(x))+((y)*(y))))/(z))
#define NORMALIZE_DISTANCE(x,y) ((x)/(y))
#define ONE 1.0

/**********************************
Module: NEIGHBOR_ID (4 Directions)
Identify 4-neighbors, List of Cells (4 long)

INPUTS:
ActiveList *active
DataCell **grid
double *gridMetadata
ActiveList *ActiveList
Neighbor *neighborList
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
ActiveList *active, 
DataCell **grid, 
double *gridMetadata,
Neighbor *neighborList,
VentArr *vent) 
{
	unsigned char code;																/*Parent bitcode*/
	int Nrow, Srow, Wcol, Ecol, aRow, aCol;	/* neighbor Row and Col  relative to active (active cell) */
	int neighborCount = 0;									/* Initialize neighbor counter */ 								/* initialize total elevation difference */
	
	/* Calculate row and column locations for active cell and its neighbor cells */
	aRow = active->row;
	aCol = active->col;
	Nrow = aRow + 1;
	Srow = aRow - 1;
	Ecol = aCol + 1;
	Wcol = aCol - 1;

	if (Srow < 0) {
		fprintf(stderr,"\nFLOW IS OFF THE MAP[%d]! (South) [NEIGHBOR_ID]\n", Srow);
		return -1;
	} else if (Nrow >= gridMetadata[4]) {
		fprintf(stderr,"\nFLOW IS OFF THE MAP[%d]! (North) [NEIGHBOR_ID]\n", Nrow);
		return -2;
	} else if (Wcol < 0) {
		fprintf(stderr,"\nFLOW IS OFF THE MAP[%d]! (West) [NEIGHBOR_ID]\n",Wcol);
		return -3;
	} else if (Ecol >= gridMetadata[2]) {
		fprintf(stderr, "\nFLOW IS OFF THE MAP[%d]! (East) [NEIGHBOR_ID]\n", Ecol);
		return -4;
	}
	
	/*NORTH neighbor*/
	code = grid[aRow][aCol].parentcode & 4;
	if (!code) { /* NORTH cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\t NORTH-Cell * ");
#endif
			if (grid[aRow][aCol].eff_elev > grid[Nrow][aCol].eff_elev) { /* active cell is higher than North neighbor */
				/* Calculate elevation difference between active cell and its North neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[Nrow][aCol].eff_elev;
				(neighborList+neighborCount)->row  = Nrow;
				(neighborList+neighborCount)->col  = aCol;
				neighborCount +=1;
			}
#ifdef PRINT4  
			else fprintf(stderr, "NORTH neighbor too high [%0.4f]\n", grid[Nrow][aCol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=NORTH[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
	/*EAST*/
	code = grid[aRow][aCol].parentcode & 2;
	if (!code) { /* EAST cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\t EAST-Cell * ");
#endif
			if (grid[aRow][aCol].eff_elev > grid[aRow][Ecol].eff_elev) { /* active cell is higher than EAST neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[aRow][Ecol].eff_elev;
				(neighborList+neighborCount)->row  = aRow;
				(neighborList+neighborCount)->col  = Ecol;
				neighborCount +=1;
			}			
#ifdef PRINT4  
			else fprintf(stderr, "EAST neighbor too high [%0.4f]\n", grid[aRow][Ecol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=EAST[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
/*SOUTH*/
	code = grid[aRow][aCol].parentcode & 1;
	if (!code) { /* SOUTH cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\t SOUTH-Cell * ");
#endif			
			if(grid[aRow][aCol].eff_elev > grid[Srow][aCol].eff_elev) { /* active cell is higher than SOUTH neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[Srow][aCol].eff_elev;
				(neighborList+neighborCount)->row  = Srow;
				(neighborList+neighborCount)->col  = aCol;
				neighborCount +=1;
			}			
#ifdef PRINT4  
			else fprintf(stderr, "SOUTH neighbor too high [%0.4f]\n", grid[Srow][aCol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=SOUTH[%d]\n", grid[aRow][aCol].parentcode);
#endif

	/*WEST*/
	code = grid[aRow][aCol].parentcode & 8;
	if (!code) { /* WEST cell is not the parent cell*/
#ifdef PRINT  
			fprintf(stderr,"\tWEST-Cell * ");
#endif			
			if(grid[aRow][aCol].eff_elev > grid[aRow][Wcol].eff_elev) {/* active cell is higher than WEST neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[aRow][Wcol].eff_elev;
				(neighborList+neighborCount)->row  = aRow;
				(neighborList+neighborCount)->col  = Wcol;
				neighborCount +=1;
			}				
#ifdef PRINT4  
			else fprintf(stderr, "WEST neighbor too high [%0.4f]\n", grid[aRow][Wcol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\tParent=WEST[%d]\n", grid[aRow][aCol].parentcode);
#endif
  return neighborCount;
}
