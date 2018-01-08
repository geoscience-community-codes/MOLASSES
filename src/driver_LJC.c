#include <prototypes_LJC.h>
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

DISTRIBUTE*******************************************************
Distributes lava from cells to neighboring cells depending on
module specific algorithm (e.g. slope-proportional sharing).
Updates a Cellular Automata List and the active cell counter.

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
	Automata *CAList = NULL;		/* Active Cell list */
	Automata NeighborList[4];		/* Each cell can have at most 4 neighbors. */
	VentArr  Vent;						/* Source Vent structure */
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
	
	double DEMmetadata[6];				/* Geographic Metadata from GDAL */
	double volumeErupted = 0;		/* Total Lava Volume in All Active Cells */
	double volumeRemaining = 0;	/* Volume Remaining to be Erupted */
	double total = 0;						/* Difference between volumeErupted-Vent.volumeToErupt */

	int run = 0;			/* Current lava flow run */ 
	int start = 0;		/* Starting run number, from command line or 0 */
	int endrun = 0;   /* Last lava flow run */
  
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
  fprintf(stderr, "Seeding random number generator: %s\n", phrase);
  initialize ( );  /* Initialize the rng's */
  phrtsd ( phrase, &seed1, &seed2 ); /* Initialize all generators. */
  set_initial_seed ( seed1, seed2 );  /* Set seeds based on phrase. */
    
	fprintf(stderr, "\n\n               MOLASSES is a lava flow simulator.\n\n");
	
	if(argc < 2) {
		fprintf(stderr, "Usage: %s config-filename\n",argv[0]);
		return 1;
	}
	if (argc == 3) {
		start = atoi(argv[2]);
		if (start < 0) start = 0;
	}
	
	fprintf(stderr, "Beginning flow simulation...\n");	    
	In.config_file = argv[1]; 
	fprintf(stderr, "%s\n", In.config_file);
	
    /* Initialize variables with values from the config file */
	ret = INITIALIZE(
	&In,        /* (type=Inputs*) 1D Input parameters structure  */
	&Out,       /* (type=Outputs*) 1D Output parameters structure */
	&Vent);     /* (VentArr*) Vent Structure */

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
				&Vent,			/* (VentArr*) Vent Structure */
				DEMmetadata,	/* (type=double*) Metadata array */
				Grid);			/* (type=DataCell**)  2D Data Grid */
		
		/* Select new vent from spatial density grid 
		   OR
		   If a vent coordinate is given, then use this coordinate. 
		*/
		if (In.spd_file != NULL) {
			do { 
				ret = CHOOSE_NEW_VENT(&In, &Vent);
				if (ret) {
					fprintf (stderr, "\n[MAIN] Error returned from [CHOOSE_NEW_VENT].\nExiting!\n");
					return 1;
				}
				ret = CHECK_VENT_LOCATION(&Vent, DEMmetadata); /*Check for Vent outside of map region*/
				if (ret) {
					Vent.easting = 0;
					Vent.northing = 0;
					continue; /* select another vent location */
				}
			} while (!Vent.easting || !Vent.northing);
		}
    fprintf (stderr, "[Run: %d] Vent: EASTING: %f\tNorthing: %f\n", run, Vent.easting, Vent.northing);
		/* Initialize the lava flow data structures and initialize vent cell. 
		   TODO: try to figure out the maximum size possible for the active list */
		CAList = INIT_FLOW(
		Grid,					/* (type=DataCell**)  2D Data Grid */
		CAList,				/* type=Automata*) 1D Active Cells List */
		&Vent,				/* (type=VentArr*) Vent Data structure */
		&CAListSize,		/* (type= unsigned int*)  Max size of active List */
		&ActiveCounter,	/* (type= unsigned int*) Active list current cell count */
		DEMmetadata);		 /* (type=double*) Metadata array */

		if (CAList == NULL) { 
			fprintf (stdout, "[MAIN] Error returned from [INIT_FLOW]. Exiting\n");
			return 1;
		}

		fprintf(stderr, "\nRunning Flow #%d from (%d, %d)\n", run, CAList->row, CAList->col);
		/* Initialize the remaining volume to be the volume of lava to erupt. */
		volumeRemaining = Vent.volumeToErupt; 

		/* Run the flow until the volume to erupt is exhausted. */
		while(volumeRemaining > (double) 0.0) {
    
			/* vent cell gets a new pulse of lava to distribute 
			   see file: pulse.c 
			*/
			if (!(pulseCount % 100))
				fprintf(stderr, "[R%d]Active Cells: %-3u; Volume Remaining: %10.3f Pulse count: %3u \n",
				run,			
				ActiveCounter,
				volumeRemaining,
				pulseCount);
			
			
			PULSE(
			CAList,				/* (type=Automata*) 1D Active Cells List */
			&Vent,				/* (type=VentArr*) Vent Data structure */
			Grid,					/* (type=DataCell**)  2D Data Grid */
			&volumeRemaining,	/* (type=double) Lava volume not yet erupted */
			DEMmetadata);		/* (type=double*) Metadata array */
		
			pulseCount++;

			/* Distribute lava to active cells and their 8 neighbors. */
			ret = DISTRIBUTE(
			Grid,					/* (type=DataCell**)  2D Data Grid */
			CAList,				/* (type=Automata*) 1D Active Cells List */
			&CAListSize,	/* (type=unsigned int) Max size of Active list */
			&ActiveCounter,	/* (type=unsigned int*) Active list current cell count */
			NeighborList,  	/* (type=Automata*) 4 element list of cell-neighbors info */
			DEMmetadata,		/* (type=double*) Metadata array */
			&In,					/* (type=Inputs*) Inputs structure */
			&Vent);				/* (type=VentArr*) Vent Data structure */
		
			if (ret < 0) {
				fprintf (stderr, "[MAIN} Error returned from [DISTRIBUTE].ret=%d.. ", ret);
				if (run > 0) {
					fprintf(stderr, "Starting a new run.\n");
					run--;
				}
			}
		} /* while(volumeRemaining > (double)0.0) */
			
		fprintf(stderr, "Final Distribute: %d cells inundated.\n\n", ActiveCounter);

		volumeErupted = 0.0;
		/* Sum lava volume in each active flow cell */
		
		for(i = 0; i < DEMmetadata[4]; i++) {
			for(j = 0; j < DEMmetadata[2]; j++) {
					thickness = Grid[i][j].eff_elev - Grid[i][j].dem_elev;
					if (Grid[i][j].active > -1) Grid[i][j].hit_count++; /* Increment hit count */
					volumeErupted += (thickness * DEMmetadata[1] * DEMmetadata[5]);
			}
		} 

		fprintf(stdout, "Conservation of mass check\n");
		fprintf(stdout, " Total (IN) volume pulsed from vents:   %12.3f\n", Vent.volumeToErupt);
		fprintf(stdout, " Total (OUT) volume found in cells:     %12.3f\n\n", volumeErupted);

		total = volumeErupted - Vent.volumeToErupt;
		if(abs(total) > 1e-8) fprintf(stdout, " ERROR: MASS NOT CONSERVED! Excess: %12.3f\n", total);
		fprintf(stdout, "----------------------------------------\n");
		
		/* Save the flow thickness for each run to a file */
		ret = OUTPUT(
		run,             /* run number */
		ascii_flow,      /* file output type */
		&Out,            /* (type=Outputs*) 1D Output parameters structure */
		&In,             /* (type=Inputs*) 1D Input parameters structure */
		Grid,            /* (type = DataCell *) Global Data Grid */ 
		CAList,          /* (type = Automata *) Active Cells List */
		ActiveCounter,   /* (type = unsigned int)  Number of active cells */
		&Vent,           /* (type=VentArr*) Vent Data structure */
		DEMmetadata);    /* (type=double*) Metadata array */ 
		if (ret) fprintf(stdout, "OUTPUT ERROR!\n");
		
		if (In.flow_field) { /* reinitialize the data grid */
			for(i = 0; i < DEMmetadata[4]; i++) {
				for(j = 0; j < DEMmetadata[2]; j++) {
					Grid[i][j].dem_elev = Grid[i][j].eff_elev;
					Grid[i][j].prev_elev = Grid[i][j].dem_elev; 
					Grid[i][j].active = -1;
					Grid[i][j].parentcode = 0;
				}
			}
		}
	} /* END:  for (run = start; run < (In.runs+start); run++) { */	
	
	ret = OUTPUT(
		run,             /* run number */
		ascii_hits,      /* file output type */
		&Out,            /* (type=Outputs*) 1D Output parameters structure */
		&In,             /* (type=Inputs*) 1D Input parameters structure */
		Grid,            /* (type = DataCell *) Global Data Grid */ 
		CAList,          /* (type = Automata *) Active Cells List */
		ActiveCounter,   /* (type = unsigned int)  Number of active cells */
		&Vent,           /* (type=VentArr*) Vent Data structure */
		DEMmetadata);    /* (type=double*) Metadata array */ 

	if (ret) fprintf(stdout, "Ascii hits OUTPUT ERROR!\n");
	if (In.flow_field > 0) {
		ret = OUTPUT(
		run,             /* run number */
		raster_post,      /* file output type */
		&Out,            /* (type=Outputs*) 1D Output parameters structure */
		&In,             /* (type=Inputs*) 1D Input parameters structure */
		Grid,            /* (type = DataCell *) Global Data Grid */ 
		CAList,          /* (type = Automata *) Active Cells List */
		ActiveCounter,   /* (type = unsigned int)  Number of active cells */
		&Vent,           /* (type=VentArr*) Vent Data structure */
		DEMmetadata);    /* (type=double*) Metadata array */ 

		if (ret) fprintf(stdout, "Raster post dem OUTPUT ERROR!\n");
	}
		
	endTime = time(NULL); /* Calculate simulation time elapsed */
	if ((endTime - startTime) > 60) {
		fprintf(stdout, "\n\nElapsed Time of simulation approximately %0.1f minutes.\n\n",
		(double)(endTime - startTime)/60.0);
	}	
	else {
		fprintf(stdout, "\n\nElapsed Time of simulation approximately %u seconds.\n\n",
		(unsigned)(endTime - startTime));
	}
	return 0;
}
