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
#define CR 13            /* Decimal code of Carriage Return char */
#define LF 10            /* Decimal code of Line Feed char */

Vent *load_vent_data(FILE *in, int *num_vents) {
	
	int maxLine = 50;
	char line[50];
	int num = 0;
	int lines_read = 0;
	int ret;
	Vent *new_vent;
	
  /* (type =string) Line from file */
	while (fgets(line, maxLine, in) != NULL) 
	{
		/*if first character is comment, new line, space, return to next line*/
		if (line[0] == '#' || line[0] == '\n' || line[0] == ' ') continue;
		num++;
	}
	rewind(in);
	
	new_vent = (Vent *)GC_MALLOC(( (size_t)num * sizeof(Vent)));
	if (new_vent == NULL) {
		fprintf(stderr, "Cannot malloc memory for vent array:[%s]\n", strerror(errno));
		return NULL;
	}
	while (lines_read < num) {
		fgets(line, maxLine, in);
		if (line[0] == '#' || line[0] == '\n' || line[0] == ' ') continue;
		
		while (ret = sscanf(line, "%lf %lf", 
		                          &(new_vent+lines_read)->easting,
                              &(new_vent+lines_read)->northing), 
                              ret != 2) {
			if (ret == EOF && errno == EINTR) continue;
			fprintf(stderr, "line %d: ret=%d]", lines_read, ret);
			
    }
     /*fprintf(stdout, "\n%.0f %.0f",(new_vent+lines_read)->easting, (new_vent+lines_read)->northing); 
     fprintf(stdout, "%25s = %-25s ",var, value); print incoming parameter value
		fflush(stdout);  */
    lines_read++; 

	}
	*num_vents = lines_read;
	
	return new_vent;
}
/*Module INITIALIZE
	Accepts a configuration file and returns model variables. 
	(format: keyword = value):
	The following KEYWORDS are accepted:
		

	(VENT PARAMETERS)	
	double VENT_EASTING (units = km)
	double VENT_NORTHING (units = km)
	
	(FLOW PARAMETERS)
	int parents
	
	Vent *source
	
	double MIN_RESIDUAL
	double MAX_RESIDUAL
	
	double MIN_TOTAL_VOLUME
	double MAX_TOTAL_VOLUME
	
	double MEAN_TOTAL_VOLUME
	double STD_TOTAL_VOLUME
	
	double MIN_PULSE_VOLUME
	double MAX_PULSE_VOLUME
	
	(Simulation Parameters)
	int FLOWS
	int RUNS
	
INPUTS:
Inputs *In: Structure of input parmaeters 
Outputs *Out: Structure of model outputs 
VentArr *Vents: Array of active vent structures 
	
RETURN: int 0=No error; 1=error

	
Checks at end for configuration file errors, where mandatory parameters were not assigned.
*/

int INITIALIZE(
Inputs *In, /* Structure of input parmaeters */
Outputs *Out, /* Structure of model outputs */
Lava_flow *active_flow) /* Active_flow structure */ 
{

	int maxLineLength = 256;
	char line[256];             /* (type =string) Line from file */
	char var[64];               /* (type=string) Parameter Name  in each line*/
	char value[256];            /* (type=string) Parameter Value in each line*/
	char *ptr;
	FILE* ConfigFile;
	FILE* Opener;     /*Dummy File variable to test valid output file paths */
	/* enum ASCII_types ascii_file; */
	double dval;
	int ret = 0;
	int i;
	
	/* Initialize vent parameters */
	active_flow->source = NULL;
	active_flow->currentvolume = 0;
	active_flow->volumeToErupt = 0;
	active_flow->pulsevolume = 0;
	active_flow->residual = 0;
	active_flow->spd_grd = NULL;
	
	/* Initialize input parameters */
	In->vents_file = NULL;
	In->dem_file = NULL;
	In->slope_map = NULL;
	In->residual = 0;
	In->uncert_map = NULL;
	In->elev_uncert = 0;
	In->spd_file = NULL;
	In->num_grids = 0;
	In->spd_grid_spacing = 0;
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
	In->flows = 1;
	In->flow_field = 0;
	
	
	/* Initialize output parmaeters */
	Out->ascii_flow_file = "";
	Out->ascii_hits_file = "";
	Out->raster_hits_file = "";
	Out->raster_flow_file = "";
	Out->raster_post_dem_file = "";
	Out->raster_pre_dem_file = "";
	Out->stats_file = "";
	
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
		var[0] = '\n';
		value[0] = '\n';
		sscanf (line,"%s = %s",var,value); /*split line into before ' = ' and after*/
		fprintf(stdout, "%25s = %-25s ",var, value); /*print incoming parameter value*/
		fflush(stdout);
		
		if (!strncmp(var, "DEM_FILE", strlen("DEM_FILE"))) 
		{
			In->dem_file = (char*) GC_MALLOC(sizeof(char) * (strlen(value)+1));
			if (In->dem_file == NULL) 
			{
				fprintf(stderr, 
				        "\n[INITIALIZE] Out of Memory assigning filenames!\n");
				return 1;
			}
			strncpy(In->dem_file, value, strlen(value)+1);
		}		
		else if (!strncmp(var, "RESIDUAL", strlen("RESIDUAL"))) 
		{
			dval = strtod(value, &ptr);
			if (strlen(ptr) > 0) 
			{
				In->slope_map = (char *) GC_MALLOC(sizeof(char) * (strlen(ptr)+1));
				if (In->dem_file == NULL) 
				{
					fprintf(stderr, 
					        "\n[INITIALIZE] Out of Memory assigning filenames!\n");
					return 1;
				}
				strncpy(In->slope_map, ptr, strlen(ptr)+1);
				In->residual = -1;
				Opener = fopen(In->slope_map, "r");
				if (Opener == NULL) 
				{
					fprintf(stderr, 
					        "\nERROR [INITIALIZE]: Failed to open input file at [%s]:[%s]!\n", 
					        In->slope_map, strerror(errno));
					return 1;
				}
				else (void) fclose(Opener);
			}
			else if (dval > 0) In->residual = dval;
		}
		else if (!strncmp(var, "ELEVATION_UNCERT", strlen("ELEVATION_UNCERT"))) 
		{
			dval = strtod(value, &ptr);
			if (strlen(ptr) > 0) 
			{
				In->uncert_map = (char *) GC_MALLOC(sizeof(char) * (strlen(ptr)+1));
				if (In->uncert_map == NULL) 
				{
					fprintf(stderr, "\n[INITIALIZE] Out of Memory assigning filenames!\n");
					return 1;
				}
				strncpy(In->uncert_map, ptr, strlen(ptr)+1);
				In->elev_uncert = -1;
				Opener = fopen(In->uncert_map, "r");
				if (Opener == NULL) 
				{
					fprintf(stderr, 
								"\nERROR [INITIALIZE]: Failed to open input file: [%s]:[%s]!\n",
								In->uncert_map, strerror(errno));
					return 1;
				}
				(void) fclose(Opener);
			}
			else if (dval > 0) 	In->elev_uncert = dval;
		}
		else if (!strncmp(var, "SPATIAL_DENSITY_FILE", strlen("SPATIAL_DENSITY_FILE"))) 
		{
			In->spd_file = (char *)GC_MALLOC(((strlen(value)+1) * sizeof(char)));	
			if (In->spd_file == NULL) 
			{
				fprintf(stderr, 
							"Cannot malloc memory for spatial density file:[%s]\n", 
							strerror(errno));
				return 1;
			}
			strncpy(In->spd_file, value, strlen(value)+1);
			Opener = fopen(In->spd_file, "r");
			if (Opener == NULL) 
			{
				fprintf(stderr, 
							"\nERROR [INITIALIZE]: Failed to open input file: [%s]:[%s]!\n",
							In->spd_file, strerror(errno));
				return 1;
			}
			ret = load_spd_data(Opener, active_flow, &In->num_grids);
			(void)fclose(Opener);
			if (ret) 
			{
				fprintf (stderr, 
							"\nERROR []INITIALIZE: error returned from [load_spd_file].\n");
				return 1;
			}
		}
		else if (!strncmp(var, "VENTS_FILE", strlen("VENTS_FILE"))) 
		{
			In->vents_file = (char *)GC_MALLOC(((strlen(value)+1) * sizeof(char)));	
			if (In->vents_file == NULL) 
			{
				fprintf(stderr, 
							"Cannot malloc memory for vents file:[%s]\n", 
							strerror(errno));
				return 1;
			}
			strncpy(In->vents_file, value, strlen(value)+1);
			Opener = fopen(In->vents_file, "r");
			if (Opener == NULL) 
			{
				fprintf(stderr, 
							"\nERROR [INITIALIZE]: Failed to open vents file: [%s]:[%s]!\n",
							In->vents_file, strerror(errno));
				return 1;
			}
			active_flow->source = load_vent_data(Opener, &active_flow->num_vents);
			if (active_flow->source == NULL) 
			{
				fprintf (stderr, 
							"\nERROR []INITIALIZE: error returned from [load_vent_data].\n");
				return 1;
			} 
			(void)fclose(Opener);
			
		}
		else if (!strncmp(var, "SPD_GRID_SPACING", strlen("SPD_GRID_SPACING"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->spd_grid_spacing = dval;
			else 
			{
				fprintf(stderr, 
							"\nERROR [INITIALIZE]: unable to read in a value for the spatial density grid spacing!\n");
				fprintf(stderr, 
				        "Please set a value for SPD_GRID_SPACING in config file\n" );
				return 1;
			}
		}
		
		else if (!strncmp(var, "ASCII_FLOW_MAP", strlen("ASCII_FLOW_MAP"))) 
		{
			/* Add extra room for a nujmber at end of file name */
			Out->ascii_flow_file = (char *)GC_MALLOC(((strlen(value)+1) * sizeof(char)));
			if (Out->ascii_flow_file == NULL) 
			{
				fprintf(stderr, 
							"Cannot malloc memory for ascii_flow file:[%s]\n", 
							strerror(errno));
				return 1;
			}
			strncpy(Out->ascii_flow_file, value, strlen(value)+1);
		}
		else if (!strncmp(var, "ASCII_HIT_MAP", strlen("ASCII_HIT_MAP"))) 
		{
			Out->ascii_hits_file = (char *)GC_MALLOC(((strlen(value)+1) * sizeof(char)));	
			if (Out->ascii_hits_file == NULL) 
			{
				fprintf(stderr, 
							"Cannot malloc memory for ascii_hits file:[%s]\n", 
							strerror(errno));
				return 1;
			}
			strncpy(Out->ascii_hits_file, value, strlen(value)+1);
		}
		else if (!strncmp(var, "RASTER_FLOW_MAP", strlen("RASTER_FLOW_MAP"))) 
		{
			Out->raster_flow_file = (char *)GC_MALLOC(((strlen(value)+1) * sizeof(char)));	
			if (Out->raster_flow_file == NULL) 
			{
				fprintf(stderr, 
							"Cannot malloc memory for raster flow file:[%s]\n", 
							strerror(errno));
				return 1;
			}
			strncpy(Out->raster_flow_file, value, strlen(value)+1);
		}
				else if (!strncmp(var, "RASTER_HIT_MAP", strlen("RASTER_HIT_MAP"))) 
		{
			Out->raster_hits_file = (char *)GC_MALLOC(((strlen(value)+1) * sizeof(char)));	
			if (Out->raster_hits_file == NULL) 
			{
				fprintf(stderr, 
							"Cannot malloc memory for raster hits file:[%s]\n", 
							strerror(errno));
				return 1;
			}
			strncpy(Out->raster_hits_file, value, strlen(value)+1);
		}
		else if (!strncmp(var, "RASTER_POST_DEM", strlen("RASTER_POST_DEM"))) 
		{
			Out->raster_post_dem_file = (char *)GC_MALLOC(((strlen(value)+1) * sizeof(char)));	
			if (Out->raster_post_dem_file == NULL) 
			{
				fprintf(stderr, 
							"Cannot malloc memory for post raster dem file:[%s]\n", 
							strerror(errno));
				return 1;
			}
			strncpy(Out->raster_post_dem_file, value, strlen(value)+1);
		}
		
		/*VENT PARAMETERS*/
		else if (!strncmp(var, "MIN_PULSE_VOLUME", strlen("MIN_PULSE_VOLUME"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->min_pulse_volume = dval;
			else 
			{
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MIN_PULSE_VOLUME\n");
				return 1;
			}
		}
		else if (!strncmp(var, "MAX_PULSE_VOLUME", strlen("MAX_PULSE_VOLUME"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->max_pulse_volume = dval;
			else 
			{
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MAX_PULSE_VOLUME\n");
				return 1;
			}
		}
		else if (!strncmp(var, "MIN_TOTAL_VOLUME", strlen("MIN_TOTAL_VOLUME"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->min_total_volume = dval;
			else 
			{
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MIN_TOTAL_VOLUME\n");
				return 1;
			}
		}
		else if (!strncmp(var, "MAX_TOTAL_VOLUME", strlen("MAX_TOTAL_VOLUME"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->max_total_volume = dval;
			else {
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MAX_TOTAL_VOLUME\n");
				return 1;
			}
		}		
		else if (!strncmp(var, "LOG_MEAN_TOTAL_VOLUME", strlen("LOG_MEAN_TOTAL_VOLUME"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_mean_volume = dval;
			else 
			{
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for LOG_MEAN_TOTAL_VOLUME\n");
				return 1;
			}
		}
		else if (!strncmp(var, "LOG_STD_DEV_TOTAL_VOLUME", strlen("LOG_STD_DEV_TOTAL_VOLUME"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_std_volume = dval;
			else 
			{
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for LOG_STD_DEV_TOTAL_VOLUME\n");
				return 1;
			}
		}
		else if (!strncmp(var, "MIN_RESIDUAL", strlen("MIN_RESIDUAL"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->min_residual = dval;
			else 
			{
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for MIN_RESIDUAL\n");
				return 1;
			}
		}
		else if (!strncmp(var, "MAX_RESIDUAL", strlen("MAX_RESIDUAL"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->max_residual = dval;
			else 
			{
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for MAX_RESIDUAL\n");
				return 1;
			}
		}
		else if (!strncmp(var, "LOG_MEAN_RESIDUAL", strlen("LOG_MEAN_RESIDUAL"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_mean_residual = dval;
			else 
			{
				fprintf(stderr, 
							"\n[INITIALIZE]: Unable to read value for LOG_MEAN_RESIDUAL\n");
				return 1;
			}
		}
		else if (!strncmp(var, "LOG_STD_DEV_RESIDUAL", strlen("LOG_STD_DEV_RESIDUAL"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->log_std_residual = dval;
			else 
			{
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for LOG_STD_DEV_RESIDUAL\n");
				return 1;
			}
		}
		
		else if (!strncmp(var, "FLOWS", strlen("FLOWS"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->flows = (int)dval;
			else 
			{
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for number of lava flows\n");
				return 1;
			}
		}
		else if (!strncmp(var, "RUNS", strlen("RUNS"))) 
		{
			dval = strtod(value, &ptr);
			if (dval > 0) In->runs = (int)dval;
			else 
			{
				fprintf(stderr, "\n[INITIALIZE]: Unable to read value for RUNS\n");
				return 1;
			}
		}
		else if (!strncmp(var, "CREATE_FLOW_FIELD", strlen("CREATE_FLOW_FIELD"))) 
		{
			In->flow_field = 1;
		}
		else if (!strncmp(var, "PARENTS", strlen("PARENTS"))) 
		{
			In->parents = 1;
		}
		else 
		{
			fprintf (stderr, "[not assigned]\n");
			continue;
		}
		fprintf (stdout,"[assigned]\n");
	} /* END while */

	/*Check for missing parameters*/
	fprintf(stderr, "Checking for missing parameters ....\n");
	if (active_flow->num_vents < 1)
	{
		 fprintf(stderr, "\nERROR [INITIALIZE]: Number of assigned vents is less than 1. Unable to continue.\n");
		 return 1;
	}
	else 
	{
		fprintf(stdout, "Active Vents asssigned:\n");
		for (i=0; i < active_flow->num_vents; i++)
		  fprintf(stdout, "\t%.0f %.0f\n",(active_flow->source+i)->easting, (active_flow->source+i)->northing); 
	}
	if(!In->elev_uncert) 
	{ /*Elevation uncertainty is either missing or is 0.*/
		In->elev_uncert = 0;
		fprintf(stderr, "ELEVATION UNCERTAINTY = 0: DEM values are assumed to be true.\n");
	}
	if(In->min_residual <= 0 || In->max_residual <= 0) 
	{ /*Residual <= 0.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: Flow residual thickness <= 0!!\n");
		return 1;
	}
	if(strlen (In->dem_file) < 2) 
	{ /*DEM Filename is missing.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: No DEM Filename Given!!\n");
		return 1;
	}
	if(In->min_pulse_volume <= 0 || In->max_pulse_volume <= 0) 
	{ /*Pulse_volume <= 0.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: Lava pulse volume <= 0!!\n");
		return 1;
	}
	if(In->min_total_volume <= 0 || In->max_total_volume <= 0) 
	{ /*Total_volume <= 0.*/
		fprintf(stderr, "\nERROR [INITIALIZE]: Total lava volume <= 0!!\n");
		return 1;
	}
	fprintf(stderr, "Nothing missing.\n");
	
	(void) fclose(ConfigFile);
	return 0;
}
