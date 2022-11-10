#ifndef _LAVA_STRUCTS_
#define _LAVA_STRUCTS_
#define PRINT40

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <float.h>
#include <string.h>
#include <ctype.h>

#define MAX_FILENAME 100
#define MAX_FILENAME_LONG 150

/*Active Cells*/
typedef struct ActiveList {
	int row;          /* Y of flow cell (not vent) */
	int col;          /* X of flow cell (not vent) */
  int excess;
} ActiveList;

typedef struct Neighbor {
  int row;          /* Y of cell */
	int col;          /* X of cell */
	double run;
	double elev_diff; /* diff in elevation between parent and neighbor */ 
 } Neighbor;
 
/*Global Data Locations*/
typedef struct DataCell {
	double eff_elev;          /* updated: starting dem value + lava thickness*/
	int active;               /* -1 or index on active list (current vent is always 0) */
	unsigned char parentcode; /* parent code of cell on active list */
	double elev_uncert;       /* optional */
	double residual;          /* input residual value, then changed accoding to slope (this part optional)*/
	double random_code;       /* optional */
	double dem_elev;          /* starting elevation of DEM (at each run) */
	int hit_count;            /* Output count - how many times a cell is inundated by lava */
	unsigned char aoi;
} DataCell;

/* Spatial density grid 
typedef struct SpatialDensity {
	double easting;
	double northing;
	long double prob;
} SpatialDensity;

*/

typedef struct Vent {
	double northing;  /* Vent northing */
	double easting;   /* Vent easting */
	int row;          /* Y of vent cell */
	int col;          /* X of vent cell */
} Vent;

typedef struct AOI {
  double northing; /* AOI northing */
  double easting;  /* AOI easting */
  double radius;   /* distance from AOI center */
} AOI;

/*Vent Information*/
typedef struct Lava_flow {
	Vent *source;             /* Input - pointer to an array of erupting vents */
	int num_vents;            /* Number of erupting vents */
	double volumeToErupt;     /* Input - lava volume to erupt */
	double currentvolume;     /* Output - current remaining lava volume */
	double pulsevolume;       /* Input - pulse volume */
	double residual;          /* Input - residual thickness */
} Lava_flow;

/*Input parameters*/
typedef struct Inputs {
	char *config_file;
	char dem_file[MAX_FILENAME_LONG];
	char events_file[MAX_FILENAME_LONG];
	int num_events;
	char **Events;
	char slope_map[MAX_FILENAME];
	char residual_map[MAX_FILENAME];
	double residual;
	char uncert_map[MAX_FILENAME];
	int elev_uncert;
	int num_grids;
	int cell_size;
	double min_residual;
	double max_residual;
	double log_mean_residual;
	double log_std_residual;
	double min_pulse_volume;
	double max_pulse_volume;
	double min_total_volume;
	double max_total_volume;
	double log_mean_volume;
	double log_std_volume;
	int runs;
	int parents;
	int flow_field;
	AOI *aoi;
} Inputs;

/*Program Outputs
typedef struct Outputs {
	char *ascii_flow_file;
	char *ascii_hits_file;
	char *raster_hits_file;
	char *raster_flow_file;
	char *raster_post_dem_file;
	char *raster_pre_dem_file;
	char *stats_file;
} Outputs; */

typedef struct Outputs {
  char id[10];
  int ascii_flow_file;
	int ascii_hits_file;
	int raster_hits_file;
	int raster_flow_file;
	int raster_post_dem_file;
	int raster_pre_dem_file;
	int stats_file;
} Outputs;

/* last value is number of file types */
typedef enum File_output_type {
	ascii_flow,
	ascii_hits,
	raster_flow,
	raster_hits,
	raster_pre,
	raster_post,
	stats_file,
	num_types
} File_output_type;

typedef struct FlowStats {
  int event_id;
	int run;
	int hit;
	int off_map;
	int cells_inundated;
	int pulse_count;
	double volume_erupted;
	double area_inundated;
	long int runtime;
	double residual;	
	double total_volume;
	double pulse;
	int vent_count;	
	Vent *vents;
	AOI *aoi;
	double *dem_data; 
} FlowStats;


enum {
	Topog,
	Resid,
	T_unc,
}; 

enum new_vent {
	True,
	False
};

/*Global Variables
time_t startTime;
time_t endTime;
*/
/*      ,``                 .`,   
      `,.+,               ``+`    
   ,.`#++``               ,;++`,  
 , +++'`.                  ,++++ `
` +++++++;.``  ,   ,     `#+++++ `
``+++++++++++`,   ,'++++++++++'`` 
  .,`  `++++ .   .++++++;.  ,,    
     ,+++++.   ` +++++ ,          
     ,`+++;, ``'++++`,            
    ,;+++`, . +++++,              
  . ++++;;:::++++',               
   ,'++++++++++#`,                
    ,.....````,`                  
                                  
        GO BULLS                */

#endif /*#ifndef _LAVA_STRUCTS_*/
