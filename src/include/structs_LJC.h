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


/*Active Cells*/
typedef struct Automata {
	int row;          /* Y of cell */
	int col;          /* X of cell */
	int vent;         /* vent cell = 1, else 0 */
	int excess;       /* excess lava flag =, 0=no excess; 1=excess lava*/ 
	double elev_diff; /* diff in elevation between parent and neighbor */
	double rise;
} Automata;

/*Global Data Locations*/
typedef struct DataCell {
	double eff_elev;          /* updated: starting dem value + lava thickness*/
	double prev_elev;         /* eff_elev before current pulse (the old eff_elev) */
	int active;               /* -1 or index on active list (vent is always 0) */
	unsigned char parentcode; /* parent code of cell on active list (set to 0 at the beginnng of each pulse */
	double elev_uncert;       /* optional */
	double residual;          /* input residual value, then changed accoding to slope (this part optional)*/
	double random_code;       /* optional */
	double dem_elev;          /* starting elevation of DEM (at each run) */
	int hit_count;            /* Output count - how many times a cell is inundated by lava */
} DataCell;

/* Spatial density grid */
typedef struct SpatialDensity {
	double easting;
	double northing;
	long double prob;
} SpatialDensity;

/*Vent Information*/
typedef struct VentArr {
	double northing;         /* Input Vent northing */
	double easting;          /* Input Vent easting */
	double volumeToErupt;    /* Input lava volume to erupt */
	double currentvolume;    /* vent's total magma budget */
	double pulsevolume;      /* Input pulse volume */
	double residual;         /* Input residual thickness */
	SpatialDensity *spd_grd; /* pointer to spatial density grid */
} VentArr;

/*Input parameters*/
typedef struct Inputs {
	char *config_file;
	char *dem_file;
	char *slope_map;
	char *residual_map;
	double residual;
	char *uncert_map;
	int elev_uncert;
	char *spd_file;
	int num_grids;
	int spd_grid_spacing;
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
	int flows;
	int parents;
	int flow_field;
} Inputs;

/*Program Outputs*/
typedef struct Outputs {
	char *ascii_flow_file;
	char *ascii_hits_file;
	char *raster_hits_file;
	char *raster_flow_file;
	char *raster_post_dem_file;
	char *raster_pre_dem_file;
	char *stats_file;
} Outputs;

/* last value is number of file types */
typedef enum  {
	ascii_flow,
	ascii_hits,
	raster_hits,
	raster_flow,
	raster_post,
	raster_pre,
	stats_file,
	num_types
} File_output_type;

typedef struct FlowStats {
	unsigned ca_list_size;
	unsigned active_count;
	unsigned vent_count;
	unsigned run;
	double residual;
	double remaining_volume;
	double total_volume;
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

/*Global Variables*/
time_t startTime;
time_t endTime;

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
