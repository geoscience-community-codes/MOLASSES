#include "structs_LJC.h"  /* Global Structures and Variables*/
#include <gdal.h>     /* GDAL */
#include <cpl_conv.h> /* GDAL for CPLMalloc() */
#include <gc.h>
#include <ranlib.h>
#include <rnglib.h>

/*#######################
# MODULE ACTIVATE
########################
int ACTIVATE(DataCell**,ActiveList*,int,int,int,int,int,int);
 args:
DataCell **grid
ActiveList *CAList
int CAListSize
int row
int col
int aCt
int parent
int vent
return: int
*/

/*#############################
# MODULE CHOOSE_NEW_VENT
##############################*/
int CHOOSE_NEW_VENT(Inputs *, VentArr *);
/* args:
Inputs:
Inputs *In, 
VentArr *vent
Outputs:
int 0 (error code, 0 is no errors)
*/
/* Fundtions local to CHOOSE_NEW_VENT */
int load_spd_data(FILE *, VentArr *, int *);
int count_rows(char file[], long len);


/*#######################
# MODULE DEMLOADER
########################*/
DataCell **DEM_LOADER(char*, double*, DataCell**, char*);
/*args: 
INPUTS:
char *DEMfilename,
double *DEMGeoTransform,
DataCell **grid, 
char *modeltype
OUTPUTS:
DataCell **grid (or NULL on error)
*/

/*########################
# MODULE DISTRIBUTE
########################*/
int DISTRIBUTE(DataCell**,ActiveList*,unsigned int*,unsigned int*,Neighbor*,double*,Inputs*,VentArr*);
/* args:
INPUTS:
DataCell **grid
ActiveList *activeList
int *CAListSize
int *activeCount,
Neighbor *activeNeighbor
double *gridMetadata
Inputs *in
VentArr *vent
OUTPUTS:
int (O for success; <0 for error)
*/

/*########################
# MODULE INITFLOW
########################*/
ActiveList *INIT_FLOW (DataCell**,ActiveList*,VentArr*,unsigned int*, unsigned int*,double*);
/* args:
INPUTS:
DataCell **grid
ActiveList **CAList 
VentArr *vent
int *CAListSize
int *aCT
double *gridInfo 
OUTPUTS:
ActiveList * or NULL on error
	*/

/*########################
# MODULE INITIALIZE
########################*/
int INITIALIZE(Inputs *, Outputs *, VentArr *);
/* args:
INPUTS:
Inputs *In
Outputs *Out
VentArr *Vents
OUTPUTS:
int (0 on success, 1 on error) 
*/

/*########################
# MODULE NEIGHBOR
########################*/
int NEIGHBOR_ID(ActiveList *, DataCell**,double*,Neighbor*,VentArr*);
/*args:
INPUTS:
ActiveList *centerCell 
DataCell **grid 
double *gridMetadata
Neighbor *neighborList
VentArr *vent 
OUTPUTS:
int neighbor count or <0 on error 
*/
	
/*########################
# MODULE OUTPUT
########################*/
int OUTPUT(int, File_output_type, Outputs *, Inputs *, DataCell**, VentArr *, double*);
	/*args:
		run number (int)),
		Flow map type (int),
		Outputs data structure,
		Inputs data structure,
		Global Data Grid (2D),
		Vent data structure,
		DEM transform metadata
	*/

/*########################
# MODULE PULSE
########################*/
void PULSE(unsigned int*, ActiveList*, VentArr*, DataCell**, double*, double*);
/*args: 
ActiveList *actList
VentArr *vent
DataCell **grid
double *volumeRemaining
double *gridinfo
*/

/***************************
 MODULE CHECK_VENT
****************************/
int CHECK_VENT_LOCATION(VentArr*, double*, DataCell**);
/* args:
VentArr *vent
double *gridinfo
DataCell **grid
*/

int SET_FLOW_PARAMS(Inputs*, VentArr*, double*, DataCell**);
/* args:
Inputs *In 
VentArr *vent
double *gridinfo 
DataCell **grid
*/

ActiveList *ACTIVELIST_INIT(unsigned int);
DataCell **GLOBALDATA_INIT(int,int);
