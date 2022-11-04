#include "structs_LJC3.h"  /* Global Structures and Variables*/
#include <gdal.h>     /* GDAL */
/* #include <cpl_conv.h>  GDAL for CPLMalloc() */
#include <gc.h>
#include <ranlib.h>
#include <rnglib.h>
#include <assert.h>
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
/*int CHOOSE_NEW_VENT(Inputs*, Vent*,SpatialDensity*);
 args:
Inputs:
Inputs *In, 
Vent *vent
SpatialDensity *spd_grd
Outputs:
int 0 (error code, 0 is no errors)
*/
/* Fundtions local to CHOOSE_NEW_VENT 
int load_spd_data(FILE *, Lava_flow*, int *);
int count_rows(char file[], long len);
*/
Vent *CHOOSE_NEW_VENT(Inputs *, int, int *);
/*
args:
  Inputs:
    Inputs *In, -- Input data structure     
    int ev_num, -- current event number  
    int *, pointer to number of vents assigned
  Outputs:
  Vent *vent, -- Active Vents array structure
     or NULL for error
*/
    
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
int DISTRIBUTE(DataCell**,ActiveList*,int*,int*,Neighbor*,double*,Inputs*);
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
ActiveList *INIT_FLOW (DataCell**,int*,double*);
/* args:
INPUTS:
DataCell **grid (2D DEM Data Grid)
int *CAListSize (Max size of active List)
double *gridInfo (Metadata array )
OUTPUTS:
ActiveList * or NULL on error
*/
	

/*########################
# MODULE INITIALIZE
########################*/
int INITIALIZE(Inputs *, Outputs *);
/* args:
INPUTS:
Inputs *In
Outputs *Out
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
int OUTPUT(int, char *, DataCell **, Lava_flow *, double *, File_output_type, FlowStats *);
/*args:
int run, (event number),
char *Id,  Id string from config file  
DataCell **grid, (2D DEM Data Grid), 
Lava_flow *active_flow,
double *geotransform, (DEM transform metadata) 
File_output_type type, (type of output file))
FlowStats *stats, (Flow statistics data structure)
*/

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

int add_aoi_to_dem(DataCell **, double *, AOI *);
/* args:
  DataCell **grid,
  double *geotransform, 
  AOI *aoi
*/ 

//ActiveList *ACTIVELIST_INIT(unsigned int);
DataCell **GLOBALDATA_INIT(int, int);
