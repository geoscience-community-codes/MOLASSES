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
	static double total_lava_Out = 0.0;
	static double total_lava_In = 0.0;
	Automata *more = NULL;
	int excess = 0; /* Initialize this DISTRIBUTE with only the vent cell having lava to give */
	int count = 0;
	int active;
	

	
	do { /* for all active cells */
		myResidual = grid[(activeList+c)->row][(activeList+c)->col].residual;
		thickness = grid[(activeList+c)->row][(activeList+c)->col].eff_elev 
		          - grid[(activeList+c)->row][(activeList+c)->col].dem_elev;
		lavaOut = thickness - myResidual;
		
#ifdef PRINT3 
		fprintf (stderr, 
			"\nActiveCell[%d of %u]: lavaOut=%.2f, thick=%.2f residual=%.2f current elev=%.2f excess lava = %d cells\n",
			c, *activeCount, lavaOut, thickness, myResidual, grid[(activeList+c)->row][(activeList+c)->col].eff_elev, excess);
#endif					

		if (lavaOut > 0.0) { /* if this active cell has lava to give */
			if (count >= 10000) fprintf (stderr, "residual=%f, thickness=%f lavaOut=%f\n", myResidual, thickness, lavaOut);
			/* Find neighbor cells which are not parents and have lower elevation than active cell */		
			neighborCount = NEIGHBOR_ID(
									(activeList+c),		/*Automata Center Cell (parent))*/
									grid,							/*DataCell Global Data Grid */
									gridinfo,					/*double   grid data */
									activeNeighbor,		/* list for active neighbors */
									vent,						   /* VentArr* Pointer to the vent data structure */	
									&total_elev_diff); /* sum of elevation difference between neighbors and parents */
			if (count >= 10000) fprintf (stderr, "** AL[%d](%d,%d) Parent=%u Neighbors=%d **\n", 
			                    c, (activeList+c)->row, (activeList+c)->col, (unsigned int) grid[(activeList+c)->row][(activeList+c)->col].parentcode, neighborCount);

#ifdef PRINT3 
			fprintf (stderr, "parent (%d, %d) * neighbors=%d\n", (activeList+c)->row, (activeList+c)->col, neighborCount);
#endif

			if (neighborCount > 0) { 
				if ( !(activeList+c)->excess ) {
					excess +=1; /* Vent cell has excess lava and neighbors */
				(activeList+c)->excess = 1;
				}
								
				/* Now Calculate the amount of lava each neighbor gets */				
				lavaIn = lavaOut / neighborCount;

				for (n = 0; n < neighborCount; n++) { 
						
					/* Get previous parentCode of neighbor from data grid */
					parentCode = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].parentcode;
					
					
#ifdef PRINT3 
						fprintf (stderr,
						"N[%d] (%d, %d) ParentCode=%u eff_elev=%f ** ",
						n, (activeNeighbor+n)->row, (activeNeighbor+n)->col, (unsigned int) parentCode, grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev);
#endif
			
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
						(activeList + *activeCount)->excess = 0; /* Initialize new AL member */
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
								return -1;
							}
						}
					} /* END if (active < 0) Neighbor not on active list */ 
					
					
					/* Calculate lava amount to give neighbor */
					active = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active;
					grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev += lavaIn;
					total_lava_In += lavaIn;
					myResidual = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].residual;
					thickness = grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].eff_elev
					          - grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].dem_elev;
					

					/* IF neighbor has excess lava and has not yet been flagged */ 
					if (thickness > myResidual)
						if (!(activeList+active)->excess) {
							excess += 1;  /*Flag neighbor to distribute excess lava */
							(activeList+active)->excess = 1;
						}
					
	#ifdef PRINT3 
						fprintf (stderr,
						"  AL[%d] ParentCode=%u thickness=%f excess = %d\n",
						grid[(activeNeighbor+n)->row][(activeNeighbor+n)->col].active, 
						(unsigned int) parentCode, 
						thickness, excess);
#endif				
					} /* END for each neighbor */
				
				/*REMOVE ALREADY SPREAD LAVA FROM CENTER CELL**************************/
				/* Subtract lavaOut  from activeCell's  effective elevation*/
				grid[(activeList+c)->row][(activeList+c)->col].eff_elev -= lavaOut;
				total_lava_Out += lavaOut;
				excess -= 1;
				(activeList+c)->excess = 0;
				
	#ifdef PRINT3 
	fprintf (stderr, 
	"[%d of %d]Excess Lava = %d cells\n", 
	c, *activeCount, excess);
	#endif
	
			} /*End (neighborCount > 0)*/
			else if (!neighborCount) {
				excess -=1;
				(activeList + c)->excess = 0;
			}
			
			else if (neighborCount < 0) { /* might be off the grid */
				fprintf(stdout, 
				"ERROR [DISTRIBUTE]:  neighbor count=%d\n", 
				neighborCount);
				exit(1);
			}
		} /*End IF lava is thick enough to advect */
		else (activeList + c)->excess = 0;
		
		c++; /*Move to next cell */
		if (c == *activeCount) { 
			if (excess > 0) {
				count++;
				if (!(count % 10000)) { 
					fprintf (stderr,"Distribute[%d] excess=%d active cells=%d\n", count, excess, c);
					if (excess <= 1) {
			 		break;
					}
				}
				c=0;
			}
			else if (excess < 0) { 
				break;
		 }
		} 
	} while (c < *activeCount); /*Keep looping until all active cells have been tested*/

c=0;
do {
	grid[(activeList+c)->row][(activeList+c)->col].prev_elev = grid[(activeList+c)->row][(activeList+c)->col].eff_elev;
	grid[(activeList+c)->row][(activeList+c)->col].parentcode = 0;
	c++;
		} while(c < *activeCount); /*Keep looping until all active cells have been tested*/
	
 /* Keep distributing if excess lava has not been fully distributed */
	
#ifdef PRINT3 
	fprintf (stderr, 
	"End pulse[%d active cells]: lavaOut=%0.4f lavaIn=%0.4f excess=%d cells\n\n", 
	c, total_lava_Out, total_lava_In, excess);
#endif

	/*return 0 for a successful round of distribution.*/
/*	fflush(stderr);*/
	return 0;
}
