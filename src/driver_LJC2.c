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

#include "include/prototypes_LJC2.h"
#define BUFSIZE 15


/*
DRIVER_LJC:

DRIVER for MOLASSES:
ALGORITHM:
         Read in a Configuration File with INITIALIZE
         Load a DEM Raster with DEM_LOADER (uses gdal)
         and assign parameters to a data grid
         Set Parameters with SET_FLOW_PARAMS
         Create Cellular Automata list and assign source vent with INIT_FLOW
         
         Main Flow Loop:
           If there is more volume to erupt at source vent, call PULSE
           Move lava from cells to neighbors with DISTRIBUTE
         
         After flow is completely erupted:
           Check for Conservation of Mass
           Write out requested Model Output to user-specified files

INITIALIZE ********************************

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

	DataCell **Grid = NULL;			/* data Grid */
	ActiveList *CAList = NULL;		/* Active Cell list */
	Neighbor NeighborList[8];		/* Each cell can have at most 4 or 8 neighbors. */
	Lava_flow ActiveFlow;						/* Lava_flow structure */
	unsigned int ActiveCounter = 0;		/* current # of Active Cells */
	
	Inputs In;				/* Structure to hold model inputs named in Config file */
	Outputs Out;			/* Structure to hold model outputs named in config file */
	int size;					/* variable used for creating seed phrase */
	char *phrase;			/* seed phrase for random number generator */
	int seed1;				/* random seed number */
	int seed2;				/* random seed number */
	int i,j, ret;  
	unsigned int CAListSize  = 0;				/* Current size of active list, see INIT_FLOW */
	unsigned int pulseCount  = 0;				/* Current number of Main PULSE loops */
	double thickness;						/* thickness of lava in cell */
	double areaInundated = 0;
	double DEMmetadata[6];				/* Geographic Metadata from GDAL */
	double volumeErupted = 0;		/* Total Lava Volume in All Active Cells */
	double volumeRemaining = 0;	/* Volume Remaining to be Erupted */
	double total = 0;						/* Difference between volumeErupted-Flow.volumeToErupt */

	int run = 0;			/* Current lava flow run */ 
	int start = 0;		/* Starting run number, from command line or 0 */
	int endrun = 0;   /* Last lava flow run */
	int current_vent = 0; /* Keep track of which vent is currently erupting */
  
	GC_INIT();
	startTime = time(NULL); 
	srand(time(NULL));
	size = (size_t)(int)(startTime + 1);
	
	phrase = (char *)GC_MALLOC_ATOMIC(((size_t)size * sizeof(char)));	
  if (phrase == NULL) {
    fprintf(stderr, "Cannot malloc memory for seed phrase:[%s]\n", strerror(errno));
    return 1;
  }
  snprintf(phrase, size, "%d", (int)startTime);
  fprintf(stdout, "Seeding random number generator: %s\n", phrase);
  initialize ( );  /* Initialize the rng's */
  phrtsd ( phrase, &seed1, &seed2 ); /* Initialize all generators. */
  set_initial_seed ( seed1, seed2 );  /* Set seeds based on phrase. */
    
	fprintf(stdout, "\n\n               MOLASSES is a lava flow simulator.\n\n");
	
	if(argc < 2) {
		fprintf(stderr, "Usage: %s config-filename\n",argv[0]);
		return 1;
	}
	if (argc > 2) {
		start = atoi(argv[2]);
		fprintf(stderr, "Starting with run #%d\n", start);
		if (start < 0) start = 0;
	}
	fprintf(stdout, "Starting with run #%d\n", start);
	
	fprintf(stdout, "Beginning flow simulation...\n");	    
	In.config_file = argv[1]; 
	fprintf(stdout, "Config file: %s\n", In.config_file);
	
    /* Initialize variables with values from the config file */
	ret = INITIALIZE(
	&In,        /* (type=Inputs*) 1D Input parameters structure  */
	&Out,       /* (type=Outputs*) 1D Output parameters structure */
	&ActiveFlow);     /* (Lava_flow*) Lava_flow Structure */

	if(ret){
		fprintf(stderr, "\n[MAIN]: Error flag returned from [INITIALIZE].\n");
		fprintf(stderr, "Exiting.\n");
		return 1;
	}

	/* Read in the DEM using the gdal library */
	Grid = DEM_LOADER(
	In.dem_file,	/* (type=char*)  DEM file name */
	DEMmetadata,	/* (type=double*) 1D Metadata array */
	Grid,			/* (type=DataCell**)  pointer ->2D Data Grid */
	"TOPOG");		/* (type=string) Code for topography grid */
	                        
	if(Grid == NULL){
		fprintf(stderr, "[MAIN]: Error returned from [DEM_LOADER]. Exiting.\n");
		return 1;
	}
	
	/* This is the pixel resolution. Assumes both dimensions are the same. */
	/* In.cell_size = DEMmetadata[5]; */
	
	if(In.elev_uncert == -1) { /* user input an elevation uncertainty map*/
		Grid = DEM_LOADER(		/* see file demloader.c) */
		In.uncert_map,		/* (type=char*) uncertainty-grid filename*/
		DEMmetadata, 		/* (type=double*) Metadata array */
		Grid,    			/* (type=DataCell**)  pointer ->2D Data Grid */
		"T_UNC");			/* (type=string) Code for elevation uncertainty */
    
		if(Grid == NULL){
      	fprintf(stderr, "[MAIN]: Error returned from [DEM_LOADER]. Exiting.\n");
      	return 1;
    	}
	}
	else {		/* Select uncertainty value from config file */
		for(i=0;i<DEMmetadata[4];i++) {
			for(j=0;j<DEMmetadata[2];j++) {
				Grid[i][j].elev_uncert = In.elev_uncert;
			}
		}
	}
	
	endrun = In.runs + start;
	for (run = start; run < endrun; run++) {
		pulseCount = 0; /* Keeps tract of number of lava pulses per run. */
		ActiveCounter = 0; /* Keeps track of the current number of active cells. */
		fprintf (stderr, "RUN #%d\n\n", run);
		fprintf (stdout, "\nRUN #%d\n", run);
		
		ret = SET_FLOW_PARAMS(	/* see file set_flow_params.c  */
				&In,				/* (type=Inputs*) 1D Input parameters structure  */
				&ActiveFlow,			/* (Lava_flow*) Flow Structure */
				DEMmetadata,	/* (type=double*) Metadata array */
				Grid);			/* (type=DataCell**)  2D Data Grid */
		
		/* Select new vent from spatial density grid 
		   OR
		   If a vent coordinate is given, then use this coordinate. 
		*/
		if (In.spd_file != NULL) {
			do { 
				ret = CHOOSE_NEW_VENT(&In, ActiveFlow.source, ActiveFlow.spd_grd );
				if (ret) {
					fprintf (stderr, "\n[MAIN] Error returned from [CHOOSE_NEW_VENT].\nExiting!\n");
					return 1;
				}
				ret = CHECK_VENT_LOCATION(ActiveFlow.source, DEMmetadata, Grid); /*Check for Vent outside of map region*/
				if (ret) {
					ActiveFlow.source->easting = 0;
					ActiveFlow.source->northing = 0;
					continue; /* select another vent location */
				}
			} while (!ActiveFlow.source->easting || !ActiveFlow.source->northing);
		}
		for (i = 0; i < ActiveFlow.num_vents; i++) {
		  ret = CHECK_VENT_LOCATION(ActiveFlow.source+i, DEMmetadata, Grid); /*Check for Vent outside of map region*/
		  if (ret) {
			  fprintf (stderr, "[MAIN]Vent location outside of the grid area. Exiting\n");
		    return 1;
		  }
      fprintf (stderr, "[Run: %d] Vent: EASTING: %f\tNorthing: %f\n", run, (ActiveFlow.source+i)->easting, (ActiveFlow.source+i)->northing);
      fprintf (stdout, "[Run: %d] Vent: EASTING: %f\tNorthing: %f\n", run, (ActiveFlow.source+i)->easting, (ActiveFlow.source+i)->northing);
      /*row = (int) ( ( Vent.northing - DEMmetadata[3]) / DEMmetadata[5]); / Row (Y) of vent cell */
      /*col = (int) ((Vent.easting - DEMmetadata[0]) / DEMmetadata[1]); /Col (X) of vent cell */
     /* Grid[row][col].active = 0; / vent cell */
    }
		/* Initialize the lava flow data structures and initialize vent cell. 
		   TODO: try to figure out the maximum size possible for the active list */
		CAList = INIT_FLOW(
		Grid,					/* (type=DataCell**)  2D Data Grid */
		/* CAList,				type=ActiveList*) 1D Active Cells List */
		ActiveFlow.source,				/* (type=Lava_flow*) Lava_flow Data structure */
		ActiveFlow.num_vents, /* Number of erupting vents */
		&CAListSize,		/* (type= unsigned int*)  Max size of active List */
		/* &ActiveCounter, 	 (type= unsigned int*) Active list current cell count */
		DEMmetadata);		 /* (type=double*) Metadata array */

		if (CAList == NULL) { 
			fprintf (stderr, "[MAIN] Error returned from [INIT_FLOW]. Exiting\n");
			return 1;
		}

		/* fprintf(stdout, "\nRunning Flow #%d from (%d, %d)\n", run, CAList->row, CAList->col); */
		/* Initialize the remaining volume to be the volume of lava to erupt. */
		volumeRemaining = ActiveFlow.volumeToErupt; 
		//current_vent = (current_vent + 1) % (ActiveFlow.num_vents);
    current_vent = 0;
		/* Run the flow until the volume to erupt is exhausted. */
		while(volumeRemaining > (double) 0.0) {
      
			/* vent cell gets a new pulse of lava to distribute 
			   see file: pulse.c 
			*/
			current_vent = (current_vent + 1) % (ActiveFlow.num_vents);
			
			CAList->row = (ActiveFlow.source+current_vent)->row;
			CAList->col = (ActiveFlow.source+current_vent)->col;
			
			if (!(pulseCount % 100))
				fprintf(stdout, "[R%d]Vent: %6.0f %6.0f; Active Cells: %-3u; Volume Remaining: %10.3f Pulse count: %3u \n",
				run,	
				(ActiveFlow.source+current_vent)->easting,
				(ActiveFlow.source+current_vent)->northing,		
				ActiveCounter,
				volumeRemaining,
				pulseCount);
			
			
			PULSE(
			/* &ActiveCounter, 	 (type= unsigned int*) Active list current cell count */
			CAList,				/* (type=ActiveList*) 1D Active Cells List */
			&ActiveFlow,				/* (type=Lava_flow*) Lava_flow Data structure */
			Grid,					/* (type=DataCell**)  2D Data Grid */
			&volumeRemaining,	/* (type=double) Lava volume not yet erupted */
			DEMmetadata);		/* (type=double*) Metadata array */
		
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
				fprintf (stderr, "[MAIN} Error returned from [DISTRIBUTE].ret=%d.. ", ret);
				if (ret < 0) { 
					if (run > 0) {
					 fprintf(stdout, "Starting a new run.\n");
					 run--;
					}
					volumeRemaining = 0.0;
				}
			}	
		} /* while(volumeRemaining > (double)0.0) */
			
		

		volumeErupted = 0.0;
		ActiveCounter = 0;
		/* Sum lava volume in each active flow cell */
		
		for(i = 0; i < DEMmetadata[4]; i++) {
			for(j = 0; j < DEMmetadata[2]; j++) {
				thickness = Grid[i][j].eff_elev - Grid[i][j].dem_elev;
					/*if (Grid[i][j].active > -1)*/
				if (thickness > 0) {  
				  Grid[i][j].hit_count++; /* Increment hit codouble areaInundated = 0;unt */
				  ActiveCounter++;
				}
				volumeErupted += (thickness * DEMmetadata[1] * DEMmetadata[5]);
			}
		} 
		areaInundated = ActiveCounter *  DEMmetadata[1] * DEMmetadata[5];
		areaInundated /= 1e6;
      fprintf(stdout, "Final Distribute: %d cells inundated.\n\n", ActiveCounter);
      fprintf(stdout, "Area inundated:    %12.3f square km\n\n", areaInundated);
		fprintf(stdout, "Conservation of mass check\n");
		fprintf(stdout, " Total (IN) volume pulsed from vents:   %12.3f\n", ActiveFlow.volumeToErupt);
		fprintf(stdout, " Total (OUT) volume found in cells:     %12.3f\n\n", volumeErupted);

		total = volumeErupted - ActiveFlow.volumeToErupt;
		if(abs(total) > 1e-8) fprintf(stderr, " ERROR: MASS NOT CONSERVED! Excess: %12.3f\n", total);
		fprintf(stderr, "----------------------------------------\n");
		
		/* Save the flow thickness for each run to a file */
		ret = OUTPUT(
		run,             /* run number */
		ascii_flow,      /* file output type */
		&Out,            /* (type=Outputs*) 1D Output parameters structure */
		&In,             /* (type=Inputs*) 1D Input parameters structure */
		Grid,            /* (type = DataCell *) Global Data Grid */ 
		&ActiveFlow,           /* (type=Lava_flow*) Lava_flow Data structure */
		DEMmetadata);    /* (type=double*) Metadata array */ 
		if (ret) fprintf(stderr, "OUTPUT ERROR!\n");
		fprintf(stdout, "OK\n");
		if (In.flow_field) { /* reinitialize the data grid using new dem*/
			for(i = 0; i < DEMmetadata[4]; i++) {
				for(j = 0; j < DEMmetadata[2]; j++) {
					Grid[i][j].dem_elev = Grid[i][j].eff_elev;
					Grid[i][j].active = -1;
					Grid[i][j].parentcode = 0;
				}
			}
		}
		else { /* just reinitialize the data grid for new flow */
			for(i = 0; i < DEMmetadata[4]; i++) {
				for(j = 0; j < DEMmetadata[2]; j++) {
					Grid[i][j].eff_elev = Grid[i][j].dem_elev;
					Grid[i][j].active = -1;
					Grid[i][j].parentcode = 0;
			 }
			}
		}
	} /* END:  for (run = start; run < (In.runs+start); run++) { */	
	fprintf(stdout, "OK\n");
	if (strlen(Out.ascii_hits_file) > 2) {
	ret = OUTPUT(
		run,             /* run number */
		ascii_hits,      /* file output type */
		&Out,            /* (type=Outputs*) 1D Output parameters structure */
		&In,             /* (type=Inputs*) 1D Input parameters structure */
		Grid,            /* (type = DataCell *) Global Data Grid */ 
		&ActiveFlow,           /* (type=Lava_flow*) Lava_flow Data structure */
		DEMmetadata);    /* (type=double*) Metadata array */ 
	if (ret) fprintf(stderr, "Ascii hits OUTPUT ERROR!\n");
	}
	fprintf(stdout, "OK\n");
	if (strlen(Out.raster_hits_file) > 2) {
	ret = OUTPUT(
		run,             /* run number */
		raster_hits,      /* file output type */
		&Out,            /* (type=Outputs*) 1D Output parameters structure */
		&In,             /* (type=Inputs*) 1D Input parameters structure */
		Grid,            /* (type = DataCell *) Global Data Grid */ 
		&ActiveFlow,           /* (type=Lava_flow*) Lava_flow Data structure */
		DEMmetadata);    /* (type=double*) Metadata array */ 
	if (ret) fprintf(stderr, "Raster hits OUTPUT ERROR!\n");
	}
	fprintf(stdout, "OK\n");
	if (In.flow_field > 0 && strlen(Out.raster_post_dem_file) > 2) {
		ret = OUTPUT(
		run,             /* run number */
		raster_post,      /* file output type */
		&Out,            /* (type=Outputs*) 1D Output parameters structure */
		&In,             /* (type=Inputs*) 1D Input parameters structure */
		Grid,            /* (type = DataCell *) Global Data Grid */ 
		&ActiveFlow,           /* (type=Lava_flow*) Lava_flow Data structure */
		DEMmetadata);    /* (type=double*) Metadata array */ 

		if (ret) fprintf(stderr, "Raster post dem OUTPUT ERROR!\n");
	}
	fprintf(stdout, "OK\n");
	endTime = time(NULL); /* Calculate simulation time elapsed */
	if ((endTime - startTime) > 60) {
		fprintf(stdout, "\n\nElapsed Time of simulation approximately %0.1f minutes.\n\n",
		(double)(endTime - startTime)/60.0);
	}	
	else {
		fprintf(stdout, "\n\nElapsed Time of simulation approximately %u seconds.\n\n",
		(unsigned)(endTime - startTime));
	}
	fflush(stdout);
	return 0;
}
