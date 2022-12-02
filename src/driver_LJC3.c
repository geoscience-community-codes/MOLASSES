/*############################################################################
# MOLASSES (MOdular LAva Simulation Software for the Earth Sciences) 
# The MOLASSES model relies on a cellular automata algorithm to 
# estimate the area inundated by lava flows.
#
#    Copyright (C) 2015-2021  
#    Laura Connor (lconnor@usf.edu)
#    Jacob Richardson 
#    Charles Connor
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################*/ 

#include "include/prototypes_LJC3.h"
#define BUFSIZE 25


/*
DRIVER_LJC:

DRIVER for MOLASSES:
ALGORITHM:
         Read in a Configuration File with INITIALIZE
         Load a DEM Raster with DEM_LOADER (uses gdal)
         and assign parameters to a data grid
         Set Parameters with SET_FLOW_PARAMS
         Create active cell list and assign source vent with INIT_FLOW
         
         Main Flow Loop:
           If there is more volume to erupt at source vent, call PULSE
           Move lava from cells to neighbors with DISTRIBUTE
         
         After flow is completely erupted:
           Check for Conservation of Mass
           Write out requested Model Output to user-specified files


DEM_LOADER*********************************

Loads Raster into Global Data Grid based on code:
TOPOG - Assign Topography to Data Grid Locations
        DataCell Grid.dem_elev        
RESID - Loads a raster into the data grid's residual value
T_UNC - Loads a raster into the data grid's elev_uncert value
Returns a list of geographic coordinates of the raster   
DEMmetadata format:
[0] lower left x
[1] w-e pixel resolution
[2] number of cols, assigned manually
[3] lower left y
[4] number of lines, assigned manually
[5] n-s pixel resolution (negative value)
******************************************

Select a residual value from a log-normal distribution between
In.min_residual and In.max_residual, otherwise,
Select a flow residual from a uniform random distribution between
In.min_residual and In.max_residual.

Select a flow pulse voloume from a uniform random distribution between
In.min_pulse_volume and In.max_pulse_volume, otherwise,
Select a flow volume from a log-normal distribution between
In.min_total_volume and In.max_total_volume.

Select a total volume to erupt from a uniform random distribution between
In.min_total_volume and In.max_total_volume

Assign Residual, elevation, and elevation uncertainty, 
to each DataCell (grid) Location.

CHOOSE_NEW_VENT**********************************
Select new vent from spatial density grid or from config_file.
RETURN:  1 if error, 0 if no errors).
*************************************************

INIT_FLOW **********************************************************
Creates Active Cellular Automata list and activates vent.
Also creates bookkeeping variables: 
size of CA list
total number of CA lists
total number of active automata in the CA list
total volume to erupt

MAIN FLOW LOOP: PULSE LAVA AND DISTRIBUTE TO CELLS

PULSE************************************************************
Deliver lava to vent based on information in Vent Data Array.
RETURN: total volume remaining to be erupted
new active list with each pulse.

DISTRIBUTE*******************************************************
Distributes lava from cells to neighboring cells depending on
module specific algorithm (e.g. slope-proportional sharing).
Creates active list with cells getting new lava. Distributes lava to neighboring cells
(8 or 4).

CONSERVATION OF MASS CHECK:
Volume erupted = (lava thickness + lava residual) * cell length * cell width
Sum lava inundations per cell for Hit Map
Double data types are precise to 1e-8, so make sure that volume IN and
volume OUT are within this precision.

OUTPUT*************************************************************
Writes out flow locations and thickness to a file.
File Output type (int):
0 = flow_map, format = X Y Z, easting northing thickness(m)
*/

int main(int argc, char *argv[]) {

	DataCell **Grid = NULL;			      /* data Grid */
	ActiveList *CAList = NULL;		    /* Active Cell list */
	Neighbor NeighborList[8];		      /* Each cell can have at most 4 or 8 neighbors. */
	Lava_flow ActiveFlow;						  /* Lava_flow structure */
	int ActiveCounter = 0;		/* current # of Active Cells */
	
	Inputs In;				/* Structure to hold model inputs named in Config file */
	Outputs Out;			/* Structure to hold model output filenames named in config file */
	int size = BUFSIZE;		/* variable used for creating seed phrase */
	char *phrase;			/* seed phrase for random number generator */
	int seed1;				/* random seed number */
	int seed2;				/* random seed number */
	int i,j, ret;  
	int CAListSize  = 0;				/* Current size of active list, see INIT_FLOW */
	int pulseCount  = 0;				/* Current number of Main PULSE loops */
	double thickness;						        /* thickness of lava in cell */
	double areaInundated = 0;
	double DEMmetadata[6];				/* Geographic Metadata from GDAL */
	double volumeErupted = 0;		  /* Total Lava Volume in All Active Cells */
	double volumeRemaining = 0;	  /* Volume Remaining to be Erupted */
	double total = 0;						  /* Difference between volumeErupted-Flow.volumeToErupt */

	int run = 0;			    /* Current lava flow run */ 
	int start = 0;		    /* Starting run number, from command line or 0 (default) */
	int endrun = 0;       /* Last lava flow run */
	int ev = 0;           /* Event number */
	int current_vent = 0; /* Keep track of which vent is currently erupting */
  time_t startTime;
  time_t endTime;
  File_output_type type; /* integer representing file output type */
  FlowStats stats;      /* Flow statistics */
  
	GC_INIT();
	startTime = time(NULL); 
	srand(time(NULL));
	
	phrase = (char *)GC_MALLOC_ATOMIC(((size_t)size * sizeof(char)));	
  if (phrase == NULL) {
    fprintf(stderr, "Cannot malloc memory for seed phrase:[%s]\n", strerror(errno));
    return 1;
  }
  sprintf(phrase, "%d", (int)startTime);
  fprintf(stdout, "Seeding random number generator: %s\n", phrase);
  initialize ( );                     /* Initialize the rng's */
  phrtsd ( phrase, &seed1, &seed2 );  /* Initialize all generators. */
  set_initial_seed ( seed1, seed2 );  /* Set seeds based on phrase. */
    
	fprintf(stdout, "\n\n               MOLASSES is a lava flow simulator.\n\n");
	
	if(argc < 2) {
		fprintf(stderr, "Usage: %s config-filename\n",argv[0]);
		return 1;
	}
	if (argc > 2) {
		start = atoi(argv[2]);
		fprintf(stdout, "Starting with run #%d\n", start);
		if (start < 0) start = 0;
	}
	    
	In.config_file = argv[1]; 
	fprintf(stdout, "Config file: %s\n", In.config_file);
	
  /* Initialize variables with values from the config file */
	ret = INITIALIZE(
	      &In,        // (type=Inputs*) 1D Input parameters structure  
	      &Out       // (type=Outputs*) 1D Output parameters structure
  );
	if(ret){
		fprintf(stderr, "\n[MAIN]: Error flag returned from [INITIALIZE].\n");
		fprintf(stderr, "Exiting.\n");
		return 1;
	}

	/* Read in the DEM using the gdal library */
	
	Grid = DEM_LOADER(
	In.dem_file,	   /* (type=char*)  DEM file name */
	DEMmetadata,	   /* (type=double*) 1D Metadata array */
	Grid,			       /* (type=DataCell**)  pointer ->2D Data Grid */
	"TOPOG");		     /* (type=string) Code for topography grid */
	                        
	if(Grid == NULL){
		fprintf(stderr, "[MAIN]: Error returned from [DEM_LOADER]. Exiting.\n");
		return 1;
	} else fprintf (stdout, "loaded.\n\n"); 
	
	/* This is the pixel resolution. Assumes both dimensions are the same. */
	/* In.cell_size = DEMmetadata[5]; */
	
	if(In.elev_uncert == -1) { /* user input an elevation uncertainty map*/
		Grid = DEM_LOADER(		   /* see file demloader.c) */
		In.uncert_map,		       /* (type=char*) uncertainty-grid filename*/
		DEMmetadata, 		         /* (type=double*) Metadata array */
		Grid,    			           /* (type=DataCell**)  pointer ->2D Data Grid */
		"T_UNC");			           /* (type=string) Code for elevation uncertainty */
    
		if(Grid == NULL){
      	fprintf(stderr, "[MAIN]: Error returned from [DEM_LOADER]. Exiting.\n");
      	return 1;
    	}
	} else {	/* Select uncertainty value from config file */
		for(i=0;i<DEMmetadata[4];i++) {
			for(j=0;j<DEMmetadata[2];j++) {
				Grid[i][j].elev_uncert = In.elev_uncert;
			}
		}
	} // END if-else
	
	// DEMmetadata	   (type=double*) 1D Metadata array 
	// Grid			       (type=DataCell**)  pointer ->2D Data Grid
	// In.aoi          (type= AOI *) pointer to AOI structure 
	if (In.aoi != NULL)  ret = add_aoi_to_dem(Grid, DEMmetadata, In.aoi);
	fprintf(stdout, "[MAIN] Marking %d AOI cells on DEM grid\n", ret);
	
	for (ev = 0; ev < In.num_events; ev++) {
	  /* Initialize new event parameters */
	  ActiveFlow.source = NULL;
	  ActiveFlow.currentvolume = 0;
	  ActiveFlow.volumeToErupt = 0;
	  ActiveFlow.pulsevolume = 0;
	  ActiveFlow.residual = 0;
	  stats.event_id = ev;
	  stats.run = 0;
	  stats.hit = 0;
	  stats.off_map = 0;
	  stats.cells_inundated = 0;
	  stats.pulse_count = 0;
	  stats.volume_erupted = 0;
	  stats.area_inundated = 0;
	  stats.runtime = 0;
	  stats.residual = 0;	
	  stats.total_volume = 0;
	  stats.pulse = 0;
	  stats.vent_count = 0;	
	  stats.vents = NULL;
	  stats.aoi = NULL;
	  stats.dem_data = NULL;
	  
	  // Load first event into Active_flow structure */
	  fprintf(stdout, "[%d-of-%d] Events\n", ev, In.num_events);
	  
	  ActiveFlow.source = CHOOSE_NEW_VENT(&In, ev, &ActiveFlow.num_vents);
	  if ((ActiveFlow.source) == NULL) {
	    fprintf (stderr, "Error returned from [CHOOSE NEW VENT]. Exiting.\n");
	    return 1;
	  }
	  
	  i = 0;
		printf("Number of vents is %d\n", ActiveFlow.num_vents);
		while (i < ActiveFlow.num_vents) {
		  printf("\tE: %lf N: %lf, ", (ActiveFlow.source+i)->easting, (ActiveFlow.source+i)->northing);
		  i++;
		}
		printf("\n");
		
	  // Check if each Vent is inside of map region
		for (i = 0; i < ActiveFlow.num_vents; i++) {
		  ret = CHECK_VENT_LOCATION((ActiveFlow.source+i), DEMmetadata, Grid);
		  if (ret) {
			  fprintf (stderr, "[MAIN] Vent location outside of the grid area. Exiting\n");
			  fprintf (stdout, "[MAIN] Vent location outside of the grid area. Exiting\n");
		    return 1;
		  }
		  /*
      fprintf (stderr, "[Ev: %d][Run: %d] Vent: EASTING: %f\tNorthing: %f\n", 
                       ev, run, 
                       (ActiveFlow.source+i)->easting, 
                       (ActiveFlow.source+i)->northing);
      fprintf (stdout, "[Ev: %d][Run: %d] Vent: EASTING: %f\tNorthing: %f\n", 
                       ev, run, 
                       (ActiveFlow.source+i)->easting, 
                       (ActiveFlow.source+i)->northing);
      // Row (Y) of vent cell
      // row = (int) ( ( Vent.northing - DEMmetadata[3]) / DEMmetadata[5]);
      // Col (X) of vent cell 
      // col = (int) ((Vent.easting - DEMmetadata[0]) / DEMmetadata[1]); 
      // Grid[row][col].active = 0;  vent cell 
      */
    }
       
	    fprintf(stdout, "Beginning flow simulation...\n");	
	    endrun = In.runs + start;
	    for (run = start; run < endrun; run++) {
	    
		    pulseCount = 0; /* Keeps tract of number of lava pulses per run. */
		    ActiveCounter = 0; /* Keeps track of the current number of active cells. */
		  
		  
		    // Set flow parameters for this event and it's vents
		    ret = SET_FLOW_PARAMS(	   /* see file set_flow_params.c  */
				    &In,				         /* (type=Inputs*) 1D Input parameters structure  */
				    &ActiveFlow,			   /* (Lava_flow*) Flow Structure */
				    DEMmetadata,	       /* (type=double*) Metadata array */
				    Grid);			         /* (type=DataCell**)  2D Data Grid */
				    
		  
		    /* Initialize the lava flow data structure. 
		       Only do this if CAList is unallocated or equal to NULL.
		       TODO: try to figure out the maximum size possible for the active list */
		    if (CAList == NULL) { 
		      CAList = INIT_FLOW(
		           Grid,					            /* (type=DataCell**)  2D Data Grid */
		           //ActiveFlow.source,				/* (type=Lava_flow*) Lava_flow Data structure */
		           //ActiveFlow.num_vents,     /* Number of erupting vents */
		           &CAListSize,		          /* (type= unsigned int*)  Max size of active List */
		           DEMmetadata);		          /* (type=double*) Metadata array */

		      if (CAList == NULL) { 
			      fprintf (stderr, "[MAIN] Error returned from [INIT_FLOW]. Exiting\n");
			      return 1;
		      }
		    }
		   
		    /* Initialize the remaining volume to be the volume of lava to erupt. */
		    volumeRemaining = ActiveFlow.volumeToErupt; 		   
        current_vent = 0;
       
        stats.event_id = ev;
        stats.run = run;
        stats.residual = ActiveFlow.residual;
        stats.total_volume = ActiveFlow.volumeToErupt;
        stats.pulse = ActiveFlow.pulsevolume;
        stats.vent_count = ActiveFlow.num_vents;
        stats.vents = ActiveFlow.source;
        stats.aoi = In.aoi;
        stats.dem_data = DEMmetadata;
       
		     /* Run the flow until the volume to erupt is exhausted. */
		     
		    while(volumeRemaining > (double)0) {
         
          /* These stats get updated with each pulse */		      
          stats.pulse_count = pulseCount;
			    stats.volume_erupted = ActiveFlow.volumeToErupt - volumeRemaining;
			    stats.cells_inundated = ActiveCounter;
			    
			    /* vent cell gets a new pulse of lava to distribute 
			      see file: pulse.c 
			    */
			    current_vent = (current_vent + 1) % (ActiveFlow.num_vents);
			
			    CAList->row = (ActiveFlow.source+current_vent)->row;
			    CAList->col = (ActiveFlow.source+current_vent)->col;
			
			    PULSE(
			      /* &ActiveCounter, 	 (type= unsigned int*) Active list current cell count */
			         CAList,				/* (type=ActiveList*) 1D Active Cells List */
			         &ActiveFlow,				/* (type=Lava_flow*) Lava_flow Data structure */
			         Grid,					/* (type=DataCell**)  2D Data Grid */
			         &volumeRemaining,	/* (type=double) Lava volume not yet erupted */
			         DEMmetadata		/* (type=double*) Metadata array */
		      );
			    pulseCount++;
         
			    /* Distribute lava to active cells and their 8 neighbors. */
			    ret = DISTRIBUTE(
			         Grid,					/* (type=DataCell**)  2D Data Grid */
			         CAList,				/* (type=ActiveList*) 1D Active Cells List */
			         &CAListSize,	/* (type=unsigned int) Max size of Active list */
			         &ActiveCounter,	/* (type=unsigned int*) Active list current cell count */
			         NeighborList,  	/* (type=Neighbor*) 8 element list of cell-neighbors info */
			         DEMmetadata,		/* (type=double*) Metadata array */
			         &In					/* (type=Inputs*) Inputs structure */
			      /* &ActiveFlow.source				(type=Lava_flow*) Lava_flow Data structure */
			    );
			    
			    if (ret) {
				    switch (ret) {
				      case 2:
				        stats.hit = 1;   
					      fprintf(stdout, "AOI has been penetrated!\n");
					    break;
					      
				      case -1:
				        stats.off_map = 1;
				      case -2:
				        stats.off_map = 1;
				      case -3:
				        stats.off_map = 1;
				      case -4: 
				        stats.off_map = 1;
				      case 1:
				      default:
				        fprintf(stderr, "[MAIN] Error returned from DISTRIBUTE[%d]\n", ret);
					    
				    }
				    volumeRemaining = 0.0;
				    if (run > 0) {
					     fprintf(stdout, "Starting a new run.\n");
					     run--;
					  }
			    }
			   
			    
			     
			    if (!(pulseCount % 100))
				    fprintf(stdout, "[Ev: %d][Run: %d]Vent: %6.0f %6.0f; Active Cells: %-3u; Volume Remaining: %10.3f Pulse count: %3u \n",
				                   ev, run,	
				                   (ActiveFlow.source+current_vent)->easting,
				                   (ActiveFlow.source+current_vent)->northing,		
				                   ActiveCounter,
				                   volumeRemaining,
				                   pulseCount);	
			   
		    } /* while(volumeRemaining > (double)0.0) */
			 
			  volumeErupted = 0.0;
		    ActiveCounter = 0;	   
		    /* Sum lava volume in each active flow cell */
		    for(i = 0; i < DEMmetadata[4]; i++) {
			    for(j = 0; j < DEMmetadata[2]; j++) {
				    thickness = Grid[i][j].eff_elev - Grid[i][j].dem_elev;
					  /*if (Grid[i][j].active > -1)*/
				    if (thickness > 0) {  
				      Grid[i][j].hit_count++; // Increment hit count 
				      ActiveCounter++;
				    }
				    volumeErupted += (thickness * DEMmetadata[1] * DEMmetadata[5]);
			    }
		    } 
		    areaInundated = ActiveCounter *  DEMmetadata[1] * DEMmetadata[5];
		    areaInundated /= 1e6;
		    stats.cells_inundated = ActiveCounter;
		    stats.area_inundated = areaInundated;
        fprintf(stdout, "Final Distribute: %d cells inundated.\n\n", ActiveCounter);
        fprintf(stdout, "Area inundated:    %12.3f square km\n\n", areaInundated);
		    fprintf(stdout, "Conservation of mass check\n");
		    fprintf(stdout, " Total (IN) volume pulsed from vents:   %12.3f\n", ActiveFlow.volumeToErupt);
		    fprintf(stdout, " Total (OUT) volume found in cells:     %12.3f\n\n", volumeErupted);

		    total = volumeErupted - ActiveFlow.volumeToErupt;
		    if(abs(total) > 1e-8) fprintf(stderr, " ERROR: MASS NOT CONSERVED! Excess: %12.3f\n", total);
		    fprintf(stderr, "----------------------------------------\n");
		
		    /* Save the flow thickness for each run to a file */
		   
		    if (!Out.ascii_flow_file) {
		      type = ascii_flow;
		      ret = OUTPUT(
		         ev,              /* run number */
		         Out.id,         /* (type=char*) character string, ID for data file name */
		         Grid,            /* (type = DataCell *) Global Data Grid */ 
		         &ActiveFlow,     /* (type=Lava_flow*) Lava_flow Data structure */
		         DEMmetadata,     /* (type=double*) Metadata array */ 
		         type,            /* file output type */ 
		         &stats   
	        ); 
		      if (ret) fprintf(stderr, "[OUTPUT ERROR] type = %d\n", type);
		    }
		    if (In.flow_field) { /* reinitialize the data grid using new dem*/
			    for(i = 0; i < DEMmetadata[4]; i++) {
				    for(j = 0; j < DEMmetadata[2]; j++) {
					    Grid[i][j].dem_elev = Grid[i][j].eff_elev;
					    Grid[i][j].active = -1;
					    Grid[i][j].parentcode = 0;
				    }
			    }
		    } else { /* just reinitialize the data grid for new flow */
			    for(i = 0; i < DEMmetadata[4]; i++) {
				    for(j = 0; j < DEMmetadata[2]; j++) {
					    Grid[i][j].eff_elev = Grid[i][j].dem_elev;
					    Grid[i][j].active = -1;
					    Grid[i][j].parentcode = 0;
			      }
			    }
		    } // END else
	    } // END:  for (run = start; run < (In.runs+start); run++) {
	      
    /*	  
	    0 ascii_flow,
	    1 ascii_hits,
	    2 raster_flow,
	    3 raster_hits,
	    4 raster_pre,
	    5 raster_post,
	    6 stats_file,
	    7 num_types
	  
	    int ascii_flow_file;
	    int ascii_hits_file;
	    int raster_flow_file;
	    int raster_hits_file;
	    int raster_pre_dem_file;	    
	    int raster_post_dem_file;    
	    int stats_file;
	  */
	  endTime = time(NULL); /* Calculate simulation time elapsed */
	  stats.runtime = endTime - startTime;
	  if ((endTime - startTime) > 60) {
		  fprintf(stdout, "\n\nElapsed Time of simulation approximately %0.1f minutes.\n\n",
		         (double)(endTime - startTime)/60.0);
	  } else {
		  fprintf(stdout, "\n\nElapsed Time of simulation approximately %ld seconds.\n\n",
		          stats.runtime);
		}
		
	  for ( type = ascii_hits; type < num_types; type++ ) {
	   
	    if ( Out.ascii_hits_file == type || Out.raster_flow_file == type || Out.raster_hits_file == type || Out.raster_post_dem_file == type || Out.stats_file == type || (Out.raster_pre_dem_file == type && In.flow_field) ) {
	          ret = OUTPUT(
		             ev,              /* run number */
		             Out.id,         /* (type=char*) character string, ID for data file name */
		             Grid,            /* (type = DataCell *) Global Data Grid */ 
		             &ActiveFlow,     /* (type=Lava_flow*) Lava_flow Data structure */
		             DEMmetadata,     /* (type=double*) Metadata array */ 
		             type,            /* file output type */
		             &stats    
	        ); 
	        if (ret) fprintf(stderr, "[OUTPUT ERROR] type = %d\n", type);
	    }
	    else fprintf(stderr, "Not printing type = %d\n", type);
	  }
	  fflush(stdout);
	  //if (ev == 10) return 0;
	  startTime = time(NULL);
  } // END for each event
  return 0;
}