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
#define CR 13            /* Decimal code of Carriage Return char */
#define LF 10            /* Decimal code of Line Feed char */
#define MAX_LEN 256      /* Maximum number of characters for event line and config values*/
#define MAX_VAR_LEN 64  /* Maximum number of characters for config KEY */

char **load_events(FILE *in, int *num_events) {
	
	char line[MAX_LEN];
	char **events = NULL;
	int i = 0;
	
	/* Count number of events in file */
	while (fgets(line, sizeof(line), in) != NULL) 
	{
		/*if first character is comment, new line, space, return to next line*/
		if (line[0] == '#' || line[0] == '\n' || line[0] == ' ' || line[0] == '\t') continue;
		(*num_events)++;
	}
	rewind(in);
	/* Now, Allocate space for array of events. Each event will be one line from the event file*/
	// fprintf(stdout, "[load_events] Number of events = %d\n", *num_events);
	events = (char **) GC_MALLOC(( (size_t)(*num_events) * sizeof(char *) ));
	if (events == NULL) {
		fprintf(stderr, "Cannot malloc memory for event array:[%s]\n", strerror(errno));
		return NULL;
	}
	
	/* Now parse the file of events and assign lines to the event array list*/
	i = 0;
	while (fgets(line, sizeof(line), in) != NULL) {
		/*if first character is comment, new line, space, return to next line*/
		if (line[0] == '#' || line[0] == '\n' || line[0] == ' ' || line[0] == '\t') continue;
		
		line[strcspn(line, "\n")] = '\0'; // removing newline if one is found

		events[i] = (char *) GC_MALLOC(( (size_t)(sizeof(line)) * sizeof(char)));
		if (events[i] == NULL) {
		  fprintf(stderr, "Cannot malloc memory for one event line:[%s]\n", strerror(errno));
		  return NULL;
		}
		strcpy(events[i], line);
		i++;
	}
	return events;
}


AOI *setup_aoi() {
  
  AOI *aoi;
  
  aoi = (AOI *) GC_MALLOC( sizeof(AOI) );
  return aoi;
}

/*Module INITIALIZE
	Accepts a configuration file and returns model variables. 
	(format: keyword = value):
	The following KEYWORDS are accepted:
	
INPUTS:
Inputs *In: Structure of input parmaeters 
Outputs *Out: Structure of model outputs 
VentArr *Vents: Array of active vent structures 
	
RETURN: int 0=No error; 1=error

	
Checks at end for configuration file errors, where mandatory parameters were not assigned.
*/

int INITIALIZE(
    Inputs *In, /* Structure - input parameters from config file*/
    Outputs *Out /* Structure - model output filenames  */
    )
{

	int maxLineLength = MAX_LEN;
	char line[MAX_LEN];             /* (type =string) Line from file */
	char var[MAX_VAR_LEN];               /* (type=string) Parameter Name  in each line*/
	char value[MAX_LEN];            /* (type=string) Parameter Value in each line*/
	char *ptr;
	FILE* ConfigFile;
	FILE* Opener;     /* File handle for testing and opening events file*/
	double dval;
	
	/* Initialize input parameters */
	strcpy(In->events_file, "");
	In->num_events = 0;
	In->Events = NULL;
	strcpy(In->dem_file, "");
	strcpy(In->slope_map, "");
	In->residual = 0;
	strcpy(In->uncert_map, "");
	In->elev_uncert = 0;
	In->num_grids = 0;
	In->parents = 0;
	In->min_residual = 0;
	In->max_residual = 0;
	In->log_mean_residual = 0;
	In->log_std_residual = 0;
	In->min_pulse_volume = 0;
	In->max_pulse_volume = 0;
	In->min_total_volume = 0;
	In->max_total_volume = 0;
	In->log_mean_volume = 0;
	In->log_std_volume = 0;
	In->runs = 1;
	In->flow_field = 0;
	In->aoi = NULL;
	
	
	/* Initialize output parameters */ 
	
	Out->ascii_flow_file = -1;
	Out->ascii_hits_file = 0;
	Out->raster_flow_file = 0;
	Out->raster_hits_file = 0;
	Out->raster_post_dem_file = 0;
	Out->raster_pre_dem_file = 0;
	Out->stats_file = 0;
	strcpy(Out->id, "");
	
	fprintf(stdout, "Reading in Parameters...\n");
	ConfigFile = fopen(In->config_file, "r"); /*open configuration file*/
	if (ConfigFile == NULL) 
	{
		fprintf(stderr, 
					"\nERROR [INITIALIZE]: Cannot open configuration file=[%s]:[%s]!\n", 
					In->config_file, strerror(errno));
		return 1;
	}
	
	while (fgets(line, maxLineLength, ConfigFile) != NULL) 
	{
		/*if first character is comment, new line, space, return to next line*/
		if (line[0] == '#' || line[0] == '\n' || line[0] == ' ') continue;
		
		/*print incoming parameter*/
		strcpy(var, "");
		strcpy(value, "");
		sscanf (line, "%s = %s", var, value); /*split line into before ' = ' and after*/
		fprintf(stdout, "%20s = %-20s ",var, value); /*print incoming parameter value*/
		fflush(stdout);
		
		if (!strncmp(var, "DEM_FILE", strlen("DEM_FILE"))) {
			strcpy(In->dem_file, value);
		} /* DEM_FILE */
			
		else if (!strncmp(var, "RESIDUAL", strlen("RESIDUAL"))) {
			dval = strtod(value, &ptr);
			if (strlen(ptr) > 0) {
				strcpy(In->slope_map, ptr);
				In->residual = -1;
				Opener = fopen(In->slope_map, "r");
				if (Opener == NULL) {
					fprintf(stderr, 
					        "\nERROR [INITIALIZE]: Failed to open input file at [%s]:[%s]!\n", 
					        In->slope_map, strerror(errno));
					return 1;
				}
				else (void) fclose(Opener);
			}
			else if (dval > 0) In->residual = dval;
		} /* RESIDUAL */
		
		else if (!strncmp(var, "ELEVATION_UNCERT", strlen("ELEVATION_UNCERT"))) {
			dval = strtod(value, &ptr);
			if (strlen(ptr) > 0) {
				strcpy(In->uncert_map, ptr);
				In->elev_uncert = -1;
				Opener = fopen(In->uncert_map, "r");
				if (Opener == NULL) {
					fprintf(stderr, 
								"\nERROR [INITIALIZE]: Failed to open input file: [%s]:[%s]!\n",
								In->uncert_map, strerror(errno));
					return 1;
				}
				(void) fclose(Opener);
			}
			else if (dval > 0) 	In->elev_uncert = dval;
		} /* 	ELEVATION_UNCERT */
			
		else if (!strncmp(var, "EVENTS_FILE", strlen("EVENTS_FILE"))) {
			strcpy(In->events_file, value);
			Opener = fopen(In->events_file, "r");
			if (Opener == NULL) {
				fprintf(stderr, 
							"\nERROR [INITIALIZE]: Failed to open events file: [%s]:[%s]!\n",
							In->events_file, strerror(errno));
				return 1;
			}
			In->Events = load_events(Opener, &In->num_events);
			if (In->Events == NULL) {
				fprintf (stderr, 
							"\nERROR [INITIALIZE]: no events loaded [load_events].\n");
				return 1;
			} 
			(void)fclose(Opener);
		} /* EVENT_FILE */
		
		else if (!strncmp(var, "ASCII_FLOW_MAP", strlen("ASCII_FLOW_MAP"))) {
			Out->ascii_flow_file = ascii_flow;
		} /* ASCII_FLOW_MAP */
		
		else if (!strncmp(var, "ASCII_HIT_MAP", strlen("ASCII_HIT_MAP"))) {
			Out->ascii_hits_file = ascii_hits;
		} /* ASCII_HIT_MAP */
		
		else if (!strncmp(var, "RASTER_FLOW_MAP", strlen("RASTER_FLOW_MAP"))) {
			Out->raster_flow_file = raster_flow;
		} /* RASTER_FLOW_MAP */
		
		else if (!strncmp(var, "RASTER_HIT_MAP", strlen("RASTER_HIT_MAP"))) {
			Out->raster_hits_file = raster_hits;
		} /* RASTER_HIT_MAP */
		
		else if (!strncmp(var, "RASTER_POST_DEM", strlen("RASTER_POST_DEM"))) {
			Out->raster_post_dem_file = raster_post;
		} /* RASTER_POST_DEM */
		
		else if (!strncmp(var, "RASTER_PRE_DEM", strlen("RASTER_PRE_DEM"))) {
			Out->raster_pre_dem_file = raster_pre;
		} /* RASTER_PRE_DEM */
		
		else if (!strncmp(var, "STATS_FILE", strlen("STATS_FILE"))) {
		  Out->stats_file = stats_file;
		} /* STATS_FILE */
		
		/*VENT PARAMETERS*/
		else if (!strncmp(var, "MIN_PULSE_VOLUME", strlen("MIN_PULSE_VOLUME"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->min_pulse_volume = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MIN_PULSE_VOLUME\n");
				return 1;
			}
		} /* MIN_PULSE_VOLUME */
		
		else if (!strncmp(var, "MAX_PULSE_VOLUME", strlen("MAX_PULSE_VOLUME"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->max_pulse_volume = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MAX_PULSE_VOLUME\n");
				return 1;
			}
		} /* MAX_PULSE_VOLUME */
		
		else if (!strncmp(var, "MIN_TOTAL_VOLUME", strlen("MIN_TOTAL_VOLUME"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->min_total_volume = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MIN_TOTAL_VOLUME\n");
				return 1;
			}
		} /* MIN_TOTAL_VOLUME */
		
		else if (!strncmp(var, "MAX_TOTAL_VOLUME", strlen("MAX_TOTAL_VOLUME"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->max_total_volume = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MAX_TOTAL_VOLUME\n");
				return 1;
			}
		} /* MAX_TOTAL_VOLUME */
				
		else if (!strncmp(var, "LOG_MEAN_TOTAL_VOLUME", strlen("LOG_MEAN_TOTAL_VOLUME"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_mean_volume = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for LOG_MEAN_TOTAL_VOLUME\n");
				return 1;
			}
		} /* LOG_MEAN_TOTAL_VOLUME */
		
		else if (!strncmp(var, "LOG_STD_DEV_TOTAL_VOLUME", strlen("LOG_STD_DEV_TOTAL_VOLUME"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_std_volume = dval;
			else {
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for LOG_STD_DEV_TOTAL_VOLUME\n");
				return 1;
			}
		} /* LOG_STD_DEV_TOTAL_VOLUME */
		
		else if (!strncmp(var, "MIN_RESIDUAL", strlen("MIN_RESIDUAL"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->min_residual = dval;
			else {
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for MIN_RESIDUAL\n");
				return 1;
			}
		} /* MIN_RESIDUAL */
		
		else if (!strncmp(var, "MAX_RESIDUAL", strlen("MAX_RESIDUAL"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->max_residual = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MAX_RESIDUAL\n");
				return 1;
			}
		} /* MAX_RESIDUAL */
		
		else if (!strncmp(var, "LOG_MEAN_RESIDUAL", strlen("LOG_MEAN_RESIDUAL"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_mean_residual = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for LOG_MEAN_RESIDUAL\n");
				return 1;
			}
		} /* LOG_MEAN_RESIDUAL */
		
		else if (!strncmp(var, "LOG_STD_DEV_RESIDUAL", strlen("LOG_STD_DEV_RESIDUAL"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_std_residual = dval;
			else {
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for LOG_STD_DEV_RESIDUAL\n");
				return 1;
			}
		} /* LOG_STD_DEV_RESIDUAL */
		
		else if (!strncmp(var, "RUNS", strlen("RUNS"))) {
			dval = strtod(value, &ptr);
			if (dval > 0) In->runs = (int)dval;
			else {
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for RUNS\n");
				return 1;
			}
		} /* RUNS */
		
		else if (!strncmp(var, "CREATE_FLOW_FIELD", strlen("CREATE_FLOW_FIELD"))) {
			In->flow_field = 1;
		} /* CREATE_FLOW_FIELD */
		
		else if (!strncmp(var, "PARENTS", strlen("PARENTS"))) {
			In->parents = 1;
		} /* PARENTS */
		
		else if (!strncmp(var, "ID", strlen("ID"))) {
		  strcpy(Out->id, value);
		} /* ID */
		
		else if (!strncmp(var, "AOI_EASTING", strlen("AOI_EASTING"))) {
		  if (In->aoi == NULL) {
		    In->aoi = setup_aoi();
		    if (In->aoi == NULL) {
		      fprintf(stderr, "\n[INITIALIZE]: Unable to create aoi (area of interest).\n");
		      return 1;
		    }
		  }
		  dval = strtod(value, &ptr);
		  if (dval > 0) In->aoi->easting = dval;
		  else {
		    fprintf(stderr, "\n[INITIALIZE]: Unable to read value for AOI_EASTING\n");
				return 1;
		  }
		} /* AOI easting */
		
		else if (!strncmp(var, "AOI_NORTHING", strlen("AOI_NORTHING"))) {
		  if (In->aoi == NULL) {
		    In->aoi = setup_aoi();
		    if (In->aoi == NULL) {
		      fprintf(stderr, "\n[INITIALIZE]: Unable to create aoi (area of interest).\n");
		      return 1;
		    }
		  }
		  dval = strtod(value, &ptr);
		  if (dval > 0) In->aoi->northing = dval;
		  else {
		    fprintf(stderr, "\n[INITIALIZE]: Unable to read value for AOI_NORTHING\n");
				return 1;
		  }
		} /* AOI easting */
		
		else if (!strncmp(var, "AOI_RADIUS", strlen("AOI_RADIUS"))) {
		  if (In->aoi == NULL) {
		    In->aoi = setup_aoi();
		    if (In->aoi == NULL) {
		      fprintf(stderr, "\n[INITIALIZE]: Unable to create aoi (area of interest).\n");
		      return 1;
		    }
		  }
		  dval = strtod(value, &ptr);
		  if (dval > 0) In->aoi->radius = dval;
		  else {
		    fprintf(stderr, "\n[INITIALIZE]: Unable to read value for AOI_RADIUS\n");
				return 1;
		  }
		} /* AOI easting */
		
		else if (strlen(var) > 2) {
			fprintf (stdout, "[not assigned]\n");
			continue;
		} /* not assigned */
		
		fprintf (stdout,"[assigned]\n");
	} /* END while more parameters */

	/*Check for missing parameters*/
	fprintf(stderr, "Checking for missing parameters ....\n");
	if (In->events_file == NULL) {
	  fprintf (stderr, "\nERROR [INITIALIZE]: No file of events assigned. Unable to continue.\n");
	  return 1;
	} /* missing events file */
  
  if (!In->num_events) { 
    fprintf(stderr, "\nERROR [INITIALIZE]: Number of assigned events is 0. Unable to continue.\n");
    return 1;
  } /* no events */
  
	if (In->min_residual <= 0 || In->max_residual <= 0) { /*Residual <= 0.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: Flow residual thickness <= 0!!\n");
		return 1;
	} /* incorrect residual */
	
	if (strlen (In->dem_file) < 2) { /*DEM Filename is missing.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: No DEM file assigned. Unable to continue.\n");
		return 1;
	} /* missing DEM */
	
	if (In->min_pulse_volume <= 0 || In->max_pulse_volume <= 0) { /*Pulse_volume <= 0.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: Lava pulse volume <= 0. Unable to continue.\n");
		return 1;
	} /* incorrect pulse volume */
	
	if (In->min_total_volume <= 0 || In->max_total_volume <= 0) { /*Total_volume <= 0.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: Total lava volume <= 0. Unable to continue.\n");
		return 1;
	} /* incorrect total_volume */
	
	fprintf(stdout, "Nothing missing.\n");
	(void) fclose(ConfigFile);
	return 0;
}
