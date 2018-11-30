#include "include/prototypes_LJC.h"

/*DISTRIBUTE_PROPORTIONAL2SLOPE_LJC
		Determine how much lava to share between neighbors PROPORTIONAL to elevation difference
		between active cell and neghboring cells.
		The active Cell will SHARE ALL VOLUME of lava it contains ABOVE its RESIDUAL VOLUME.
*/
/*Module: DISTRIBUTE_equal4_LJC
INPUTS:
dataCell grid - a 2D Global Data Grid
ActiveList activeList - a list of active cells (where active means that this cell has excess lava to give away)
unsigned int activeCount  - the number of elements on the activeList
double gridMetadata - geometry of the Global Data Grid from gdal
double residual
          
Algorithm:
	Do for each cell on ActiveList:
		Calculate excess lava in active cell
			Identify neighboring cells (N S E W se sw ne nw)
			Shuffle the order of cells for lava distribution
			Total weights for ea. neighbor for proportion of lava received 
				For each neighbor (i.e., getting new lava):
					Update parent-code
					Calculate amount of receiving lava
					Add lava to eff_elev of neighbor cell
					if cell has excess lava and cell not on active list, then add neighbor to active list
			   Remove lava given from active cell
  Move to next active cell
RETURN:
0 = SUCCESS
<0 = some ERROR

NEIGHBOR_ID:
Identifies lower-elevation, valid-to-spread-to cells around
the active cell.

Parent bit-code is 4-bits, denotes which cell(s) is/are the "parent"
If parent cell is SOUTH, code = 1 (0001)
If parent cell is EAST, code = 2 (0010)
If parent cell is NORTH, code = 4 (0100)
If parent cell is WEST, code = 8 (1000)
IF parent cell is SW, code = 9 (1001)
IF parent cell is SE, code = 3 (0011)
IF parent cell is NE, code = 6 (0110)
IF parent cell is NW, code = 12 (1100)

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
	double total_elev_diff;
	double myResidual, thickness;
	int n = 0;  					/* neighbor counter*/
	double lavaOut, lavaIn;      /* lava volume to advect away from center cell    */
	unsigned char parentCode = 0;            /* bitwise parent-child relationship code         */
	ActiveList *more = NULL;
	int active_neighbor;
	int i, j, max, temp, r, nc;   
  int shuffle[8];
  double total_wt;
 
	
	do { /* for all active cells */
	
		myResidual = grid[(activeList+ct)->row][(activeList+ct)->col].residual;
		thickness = grid[(activeList+ct)->row][(activeList+ct)->col].eff_elev 
		          - grid[(activeList+ct)->row][(activeList+ct)->col].dem_elev;
		lavaOut = thickness - myResidual;	
		//fprintf(stderr, "\n[%d]Active Cell Parent[%d][%d] = %d\n", ct, (activeList+ct)->row, (activeList+ct)->col, grid[(activeList+ct)->row][(activeList+ct)->col].parentcode);
	  if (lavaOut <=0) return 0;	
		/* Find neighbor cells which are not parents and have lower elevation than active cell */		
		neighborCount = NEIGHBOR_ID(
									(activeList+ct),		/*Automata Center Cell (parent))*/
									grid,							/*DataCell Global Data Grid */
									gridinfo,					/*double   grid data */
									activeNeighbor,		/* list for active neighbors */
									vent					   /* VentArr* Pointer to the vent data structure */	
									); /* sum of elevation difference between neighbors and parents */
		
    
	
		/* If neighbors are found */
		if (neighborCount > 0) {
      max = neighborCount - 1;
      total_wt = 0.0;
      lavaIn = 0.0;
      for (i = 0; i < neighborCount; i++) {
        shuffle[i] = i;
         
       /* total_wt += (activeNeighbor+i)->weight; */
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
				
				//fprintf(stderr, "\nNeighbor[%d][%d] = %d\n", 
			//	(activeNeighbor+n)->row, (activeNeighbor+n)->col , grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].parentcode);
				
				/* Now Calculate the amount of lava this neighbor gets.*/	
						
						lavaIn = lavaOut/neighborCount;

			 /* fprintf(stderr, "N-%d [%d][%d] parentCode=%d P[%d][%d] : lava = %f\n", n,
        (activeNeighbor+n)->row,
         (activeNeighbor+n)->col,
         grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].parentcode,
        (activeList+ct)->row, (activeList+ct)->col, lavaIn);   */ 
					
				/* Distribute lava to neighbor */			
				grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev += lavaIn;
				
				myResidual = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].residual;
				
				thickness = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev
					          - grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].dem_elev;
					          
				/* NOW, IF neighbor has excess lava and is not on active list */ 
				if (thickness > myResidual){
				  
				  /* check if this neighbor is on active list */
				  active_neighbor = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active;		
				  	
				  if (active_neighbor < 0) { /* If neighbor cell is not on active list */
					
						/* Add neighbor to end of active list */
						(activeList + *activeCount)->row = (activeNeighbor+n)->row;
					  (activeList + *activeCount)->col = (activeNeighbor+n)->col;
					  grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active = *activeCount;
					  active_neighbor = *activeCount;
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
					 
					 }  /* END if (active < 0) Neighbor not on active list */
            
           /*else if (active_neighbor < active) {} */
				   }  
				 } /* END for each neighbor */
				
				/*REMOVE LAVA FROM Parent CELL**************************/
				/* Subtract lavaOut  from activeCell's  effective elevation*/
				grid[(activeList+ct)->row][(activeList+ct)->col].eff_elev -= lavaOut;
			/*	grid[(activeList+c)->row][(activeList+c)->col].active = -1;*/
			}
			else if (neighborCount < 0) { /* might be off the grid */
				fprintf(stdout, 
				"ERROR [DISTRIBUTE]:  neighbor count=%d\n", 
				neighborCount);
				return neighborCount;
			}
			ct++;
	} while (ct < *activeCount); /*Keep looping until all active cells have been tested*/
	
	/* All active cells have given away their access lava */
	for(i = 0; i < gridinfo[4]; i++) {
		for(j = 0; j < gridinfo[2]; j++) {
		    grid[i][j].active = -1;
		}
	}
	/*return 0 for a successful round of distribution.*/
/*	fflush(stderr);*/
	return 0;
}
