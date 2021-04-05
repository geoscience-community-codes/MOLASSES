#include "structs_LJC2.h"  /* Global Structures and Variables*/
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
int CHOOSE_NEW_VENT(Inputs*, Vent*,SpatialDensity*);
/* args:
Inputs:
Inputs *In, 
Vent *vent
SpatialDensity *spd_grd
Outputs:
int 0 (error code, 0 is no errors)
*/
/* Fundtions local to CHOOSE_NEW_VENT */
int load_spd_data(FILE *, Lava_flow*, int *);
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
int DISTRIBUTE(DataCell**,ActiveList*,unsigned int*,unsigned int*,Neighbor*,double*,Inputs*);
/* args:
INPUTS:
DataCell **grid
ActiveList *activeList
int *CAListSize
int *activeCount,
Neighbor *activeNeighbor
double *gridMetadata
Inputs *in
OUTPUTS:
int (O for success; <0 for error)
*/

/*########################
# MODULE INITFLOW
########################*/
ActiveList *INIT_FLOW (DataCell**, /* ActiveList*,*/ Vent*,int,unsigned int*,double*);
/* args:
INPUTS:
DataCell **grid (2D DEM Data Grid)
ActiveList **CAList (1D Active Cells List)
Vent *vent (pointer to Vent array)
int num_vents (number of erupting vents))
unsigned int *CAListSize (Max size of active List)
double *gridInfo (Metadata array )
OUTPUTS:
ActiveList * or NULL on error
*/
	

/*########################
# MODULE INITIALIZE
########################*/
int INITIALIZE(Inputs *, Outputs *, Lava_flow *);
/* args:
INPUTS:
Inputs *In
Outputs *Out
Lava_flow *active_flow) (Active_flow structure) 
OUTPUTS:
int (0 on success, 1 on error) 
*/

/*########################
# MODULE NEIGHBOR
########################*/
int NEIGHBOR_ID(ActiveList *, DataCell**,double*,Neighbor*);
/*args:
INPUTS:
ActiveList *centerCell 
DataCell **grid 
double *gridMetadata
Neighbor *neighborList
OUTPUTS:
int neighbor count or <0 on error 
*/
	
/*########################
# MODULE OUTPUT
########################*/
int OUTPUT(int, File_output_type, Outputs *, Inputs *, DataCell**, Lava_flow*, double*);
/*args:
int run (run number),
File_output_type type (Flow map type),
Outputs *Out, 
Inputs *In, 
DataCell **grid (2D DEM Data Grid), 
Lava_flow *active_flow,
double *geotransform (DEM transform metadata) */

/*########################
# MODULE PULSE
########################*/
void PULSE(ActiveList*,Lava_flow*,DataCell**, double*, double*);
/*args: 
ActiveList *actList
Lava_flow *active_flow,
DataCell **grid
double *volumeRemaining
double *gridinfo
*/

/***************************
 MODULE CHECK_VENT
****************************/
int CHECK_VENT_LOCATION(Vent*, double*, DataCell**);
/* args:
Vent *vent
double *gridinfo
DataCell **grid
*/

int SET_FLOW_PARAMS(Inputs*, Lava_flow*, double*, DataCell**);
/* args:
Inputs *In 
Lava_flow *active_flow
double *gridinfo 
DataCell **grid
*/

ActiveList *ACTIVELIST_INIT(unsigned int);
DataCell **GLOBALDATA_INIT(int,int);
