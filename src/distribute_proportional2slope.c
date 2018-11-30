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
ActiveList *activeList,
unsigned int *CAListSize,
unsigned int *activeCount,
Neighbor *activeNeighbor,
double *gridinfo,
Inputs *in,
VentArr *vent)
{

	int ct = 0;
	int neighborCount = 0;              /* Number of lava-accepting cells in neighborhood */
	double myResidual, thickness;
	int n = 0;  					/* neighbor counter*/
	double lavaOut, lavaIn;      /* lava volume to advect away from center cell    */
	unsigned char parentCode = 0;            /* bitwise parent-child relationship code         */
	ActiveList *more = NULL;
	int active_neighbor;
	double total_wt, my_wt;
	int i, j, max, temp, r, nc;   
  int shuffle[8];
 
	*activeCount = 1;
	do { /* for all active cells */
	  
		myResidual = grid[(activeList+ct)->row][(activeList+ct)->col].residual;
		thickness = grid[(activeList+ct)->row][(activeList+ct)->col].eff_elev 
		          - grid[(activeList+ct)->row][(activeList+ct)->col].dem_elev;
		lavaOut = thickness - myResidual;	
    if (lavaOut <=0) return 0;
		
		/* Find neighbor cells which are not parents and have lower elevation than active cell */		
		neighborCount = NEIGHBOR_ID(
									(activeList+ct),		/*Automata Center Cell (parent))*/
									grid,							/*DataCell Global Data Grid */
									gridinfo,					/*double   grid data */
									activeNeighbor,		/* list for active neighbors */
									vent					   /* VentArr* Pointer to the vent data structure */	);
		
    
	
		/* If neighbors are found */
		if (neighborCount > 0) {
      max = neighborCount - 1;
      total_wt = 0.0;
      lavaIn = 0.0;
      for (i = 0; i < neighborCount; i++) {
        total_wt += (activeNeighbor+i)->elev_diff;
        shuffle[i] = i;
      }  
		
      if (neighborCount > 1) { /* then shuffle list */
        for (i = 0; i < neighborCount-1; i++) {
         r = (rand() % max);
  	     temp = shuffle[r];
         shuffle[r] = shuffle[max];
  	     shuffle[max] = temp;
  	     max--;
       }
      } 

			/* For each neighbor */				
			for (nc = 0; nc < neighborCount; nc++) {
        n = shuffle[nc]; 
			
				/* Find parent of each neighbor cell */ 
				
				
				if ( (activeNeighbor+n)->row > (activeList+ct)->row  && (activeNeighbor+n)->col < (activeList+ct)->col ) parentCode = 3; /* (0011) this neighbor's parent is SE */
				
				else if ( (activeNeighbor+n)->row > (activeList+ct)->row && (activeNeighbor+n)->col > (activeList+ct)->col ) parentCode = 9; /* (1001) this neighbor's parent is SW */
				
				else if ( (activeNeighbor+n)->row < (activeList+ct)->row && (activeNeighbor+n)->col < (activeList+ct)->col ) parentCode = 6; /* (0110) this neighbor's parent is NE */
				
				else if ( (activeNeighbor+n)->row < (activeList+ct)->row && (activeNeighbor+n)->col > (activeList+ct)->col ) parentCode = 12; /* (1100) this neighbor's parent is NW */
				
				else if ((activeNeighbor+n)->row > (activeList+ct)->row) parentCode = 1; /* (0001) this neighbor's parent is SOUTH */
					
				else if ((activeNeighbor+n)->col < (activeList+ct)->col) parentCode = 2; /*  (0010) this neighbor's parent is EAST */
						
				else if ((activeNeighbor+n)->row < (activeList+ct)->row) parentCode = 4; /* (0100) this neighbor's parent is NORTH */
						
				else if ((activeNeighbor+n)->col > (activeList+ct)->col) parentCode = 8; /* (1000) this neighbor's parent is WEST */
					
				/* Assign parentCode to neighbor grid cell */
				grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].parentcode = parentCode; 
		/*		if (grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active >= *activeCount)
          grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active = -1;*/
				/* Now Calculate the amount of lava this neighbor gets		
					This neighbor gets lava proportional to the elevation difference with its parent;
					lower neighbors get more lava, higher neighbors get less lava.
				*/	
			
        if (total_wt > 0.0) {	
            my_wt = 	(activeNeighbor+n)->elev_diff;
						lavaIn = lavaOut * ( my_wt / total_wt);
				}
				else { 
					fprintf (stderr,  
						"PROBLEM: Cannot divide by zero or difference is less than 0: total_wt = %f\n", 
						total_wt);
						fflush(stderr);
						exit(1);
				}
					
				/* Distribute lava to neighbor */			
				grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev += lavaIn;
				
				myResidual = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].residual;
				
				thickness = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev
					          - grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].dem_elev;
					          
				/* NOW, IF neighbor has excess lava */ 
				if (thickness > myResidual){ 
            				  
				  /* check if this neighbor is active */
				  active_neighbor = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active;		
				  	
				  if (active_neighbor < 0) { 
          /* If neighbor cell is not on active list (first time with excess lava) or 
             neighbor was on active list of previous pulse */
					
					 /* Add neighbor to end of current active list */
					 (activeList + *activeCount)->row = (activeNeighbor+n)->row;
					 (activeList + *activeCount)->col = (activeNeighbor+n)->col;
           (activeList + *activeCount)->excess = 1;
					 grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active = *activeCount;
					 *activeCount += 1;				
						
						if (*activeCount == *CAListSize) { /* resize active list if more space is needed */
							fprintf (stderr, 
							"Number of cells = %u; active list out of memory (%u) reallocation happening .....\n", 
							*activeCount, *CAListSize);
							*CAListSize *= 2;
							more = (ActiveList *) GC_REALLOC( activeList, (size_t)(*CAListSize) * sizeof(ActiveList) );
							if (more != NULL) activeList = more;
							else {
								fprintf(stderr, "[DISTRIBUTE]\n");
								fprintf(stderr, 
								"   NO MORE MEMORY: Tried to re-allocate memory for activelist (%u)\n", *CAListSize);
								fflush(stderr);
								return 1;
							}
					 } 
					 
				}  /* END if (active_neighbor < 0) Neighbor not on active list */

        /*else if (active_neighbor < ct)
          (activeList + active_neighbor)->excess = 1; * Active cell receives more lava */

		  } /* END thickness > residual */
     
		} /* END for each neighbor */
				
		/*REMOVE LAVA FROM Parent CELL**************************/
		/* Subtract lavaOut  from activeCell's  effective elevation*/
		grid[(activeList+ct)->row][(activeList+ct)->col].eff_elev -= lavaOut;
    (activeList + ct)->excess = 0;
	}
	 else if (neighborCount < 0) { /* might be off the grid */
				fprintf(stdout, 
				"ERROR [DISTRIBUTE]:  neighbor count=%d\n", 
				neighborCount);
				return neighborCount;
	  }
	  ct++;
	} while (ct < *activeCount); /*Keep looping until all active cells have been tested*/
	
	/* All active cells have given away their access lava 
	for(i = 0; i < gridinfo[4]; i++) {
		for(j = 0; j < gridinfo[2]; j++) {
		    grid[i][j].active = -1;
		}
	}*/
  for (j = 1; j < *activeCount; j++) 
    grid[(activeList+j)->row][(activeList+j)->col].active = -1;
	/*return 0 for a successful round of distribution.*/
/*	fflush(stderr);*/
	return 0;
}
