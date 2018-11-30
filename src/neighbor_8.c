#include "include/prototypes_LJC.h"
#define SQRT2 1.0
/* (sqrt(2)) */
#define DIAGONAL_DISTANCE(x,y) (sqrt(((x)*(x))+((y)*(y))))
#define NORMALIZE_DISTANCE(x,y) ((x)/(y))

/**********************************
Module: NEIGHBOR_ID (8 Directions)
Identify 8-neighbors, List of Cells (8 long) updated

INPUTS:
ActiveList *active
DataCell **grid
double *gridMetadata
ActiveList *ActiveList
Neighbor *neighborList
VentArr *vent

RETURN:
(int) number of neighbors

RETURN:
Automata *neighborList->Pointer to list of eligible-for-lava neighbors
	
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
	
	SOUTHWEST: if 5th parent bit False, LEFT-DOWNWARD cell is not parent
	SOUTHEAST: if 4th parent bit False, RIGHT-DOWNWARD cell is not parent
	NORTHEAST: if 7th parent bit False, RIGHT-UPWARD cell is not parent
	NORTHWEST: if 6th parent bit False, LEFT-UPWARD cell is not parent

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
	int neighborCount = 0;									/* Initialize neighbor counter */
	
	/* Calculate row and column locations for active cell and its neighbor cells */
	aRow = active->row;
	aCol = active->col;
	Nrow = aRow + 1;
	Srow = aRow - 1;
	Ecol = aCol + 1;
	Wcol = aCol - 1;

	if (Srow < 0) {
		printf("\nFLOW IS OFF THE MAP! (South) [NEIGHBOR_ID]\n");
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
	if (!code) { /* NORTH cell is not the parent of active cell*/
#ifdef PRINT  
			fprintf(stderr," not NORTH-Cell[4] * ");
#endif
			if (grid[aRow][aCol].eff_elev > grid[Nrow][aCol].eff_elev) { /* active cell is higher than North neighbor */
				/* Calculate elevation difference between active cell and its North neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[Nrow][aCol].eff_elev; /* 1.0 is the weight for a cardinal direction cell */
				(neighborList+neighborCount)->row  = Nrow;
				(neighborList+neighborCount)->col  = aCol;
				neighborCount +=1;
			}
#ifdef PRINT4  
			else fprintf(stderr, "NORTH neighbor too high [%0.4f]\n", grid[Nrow][aCol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=4NORTH[%d]\n", (int)grid[aRow][aCol].parentcode);
#endif
	
	/*EAST*/
	code = grid[aRow][aCol].parentcode & 2;
	if (!code) { /* EAST cell is not the parent of active cell*/
#ifdef PRINT  
			fprintf(stderr," not EAST-Cell[2] * ");
#endif
			if (grid[aRow][aCol].eff_elev > grid[aRow][Ecol].eff_elev) { /* active cell is higher than EAST neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[aRow][Ecol].eff_elev; /* 1.0 is the weight for a cardinal direction cell */
				(neighborList+neighborCount)->row  = aRow;
				(neighborList+neighborCount)->col  = Ecol;
				neighborCount +=1;
			}			
#ifdef PRINT4  
			else fprintf(stderr, "EAST neighbor too high [%0.4f]\n", grid[aRow][Ecol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=2EAST[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
/*SOUTH*/
	code = grid[aRow][aCol].parentcode & 1;
	if (!code) { /* SOUTH cell is not the parent of active cell*/
#ifdef PRINT  
			fprintf(stderr," not SOUTH-Cell[1] * ");
#endif			
			if(grid[aRow][aCol].eff_elev > grid[Srow][aCol].eff_elev) { /* active cell is higher than SOUTH neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[Srow][aCol].eff_elev; /* 1.0 is the weight for a cardinal direction cell */
				(neighborList+neighborCount)->row  = Srow;
				(neighborList+neighborCount)->col  = aCol;
				neighborCount +=1;
			}			
#ifdef PRINT4  
			else fprintf(stderr, "SOUTH neighbor too high [%0.4f]\n", grid[Srow][aCol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=1SOUTH[%d]\n", grid[aRow][aCol].parentcode);
#endif

	/*WEST*/
	code = grid[aRow][aCol].parentcode & 8;
	if (!code) { /* WEST cell is not the parent of active cell*/
#ifdef PRINT  
			fprintf(stderr," not WEST-Cell[8] * ");
#endif			
			if(grid[aRow][aCol].eff_elev > grid[aRow][Wcol].eff_elev) {/* active cell is higher than WEST neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = grid[aRow][aCol].eff_elev - grid[aRow][Wcol].eff_elev; /* 1.0 is the weight for a cardinal direction cell */
				(neighborList+neighborCount)->row  = aRow;
				(neighborList+neighborCount)->col  = Wcol;
				neighborCount +=1;
			}				
#ifdef PRINT4  
			else fprintf(stderr, "WEST neighbor too high [%0.4f]\n", grid[aRow][Wcol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=8WEST[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
	
	/*DIAGONAL CELLS*/
	/*SOUTHWEST*/
	code = grid[aRow][aCol].parentcode & 9;
	if (!code) { /* SW cell is not the parent cell of active cell*/
#ifdef PRINT  
			fprintf(stderr," not SW-Cell[9] * ");
#endif	
  if(grid[aRow][aCol].eff_elev > grid[Srow][Wcol].eff_elev) {/* active cell is higher than SW neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = (grid[aRow][aCol].eff_elev - grid[Srow][Wcol].eff_elev)/SQRT2; /* SQRT2 is the weight for a diagonal cell */
				(neighborList+neighborCount)->row  = Srow;
				(neighborList+neighborCount)->col  = Wcol;
				neighborCount +=1;
			}				
#ifdef PRINT4  
			else fprintf(stderr, "SW neighbor too high [%0.4f]\n", grid[Srow][Wcol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=9SW[%d]\n\n", grid[aRow][aCol].parentcode);
#endif
		

	/*SOUTHEAST*/
	code = grid[aRow][aCol].parentcode & 3;
	if (!code) { /* SE cell is not the parent of active cell*/
#ifdef PRINT  
			fprintf(stderr," not SE-Cell[3] * ");
#endif	
  if(grid[aRow][aCol].eff_elev > grid[Srow][Ecol].eff_elev) {/* active cell is higher than SE neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = (grid[aRow][aCol].eff_elev - grid[Srow][Ecol].eff_elev)/SQRT2; /* SQRT2 is the weight for a diagonal cell */
				(neighborList+neighborCount)->row  = Srow;
				(neighborList+neighborCount)->col  = Ecol;
				neighborCount +=1;
			}				
#ifdef PRINT4  
			else fprintf(stderr, "SE neighbor too high [%0.4f]\n", grid[Srow][Ecol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=3SE[%d]\n", grid[aRow][aCol].parentcode);
#endif
	
	/*NORTHEAST*/
	code = grid[aRow][aCol].parentcode & 6;
	if (!code) { /* NE cell is not the parent of active cell*/
#ifdef PRINT  
			fprintf(stderr," not NE-Cell[6] * ");
#endif	
  if(grid[aRow][aCol].eff_elev > grid[Nrow][Ecol].eff_elev) {/* active cell is higher than NE neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = (grid[aRow][aCol].eff_elev - grid[Nrow][Ecol].eff_elev)/SQRT2; /* SQRT2 is the weight for a diagonal cell */
				(neighborList+neighborCount)->row  = Nrow;
				(neighborList+neighborCount)->col  = Ecol;
				neighborCount +=1;
			}				
#ifdef PRINT4  
			else fprintf(stderr, "NE neighbor too high [%0.4f]\n", grid[Nrow][Ecol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=6NE[%d]\n", grid[aRow][aCol].parentcode);
#endif	
	
	/*NORTHWEST*/
code = grid[aRow][aCol].parentcode & 12;
	if (!code) { /* NW cell is not the parent of active cell*/
#ifdef PRINT  
			fprintf(stderr," not NW-Cell[12] * ");
#endif	
  if(grid[aRow][aCol].eff_elev > grid[Nrow][Wcol].eff_elev) {/* active cell is higher than NW neighbor */
				/* Calculate elevation difference between active and neighbor */
				(neighborList+neighborCount)->elev_diff = (grid[aRow][aCol].eff_elev - grid[Nrow][Wcol].eff_elev)/SQRT2; /* SQRT2 is the weight for a diagonal cell */
				(neighborList+neighborCount)->row  = Nrow;
				(neighborList+neighborCount)->col  = Wcol;
				neighborCount +=1;
			}				
#ifdef PRINT4  
			else fprintf(stderr, "NW neighbor too high [%0.4f]\n", grid[Nrow][Wcol].eff_elev);
#endif
	}
#ifdef PRINT  
	else fprintf(stderr, "\nParent=12NW[%d]\n", grid[aRow][aCol].parentcode);
#endif	

	return neighborCount;
}
