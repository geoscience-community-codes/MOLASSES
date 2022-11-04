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
#define MAXLEN 25
/*****************************
MODULE: OUTPUT
Print out flow data to a file.
File types to output:
	0 = flow_map, format = X Y Z, easting northing thickness(m)
	1 = hits_map, format = X Y Z, easting northing hits(count)
	not implemented:
	2: Thickness Raster
	3: Elevation Raster
	4: Elevation + Lava Raster (2+3)
	5: Update internal DEMgrid to build volcano from lavaflows
	

GeoTransform Double List:	
	Transform Index    This Code                        GDAL
	geotransform[0]    lower left x                     top left x (SAME)
	geotransform[1]    w-e pixel res                    SAME
	geotransform[2]    number of cols                   0
	geotransform[3]    lower left y                     top left y
	geotransform[4]    number of lines                  0
	geotransform[5]    n-s pixel res (positive value)   (negative value)
	  
GDALDatasetH hDstDS;
	double GDALGeoTransform[6];
	const char *pszFormat = "GTiff";
	GDALDriverH hDriver;
	GDALRasterBandH hBand;
	GByte *RasterDataB;
	double *RasterDataD;

Flow_map:
For each cell in the current worker list
Retrieve Active Cell values and print to flowfile:
x y z: easting  northing  thickness

Hits_map:
For each cell in the dem,
Retrieve hit count values and print to hit map:
x y z: easting  northing  count
*******************************/

int OUTPUT(
int run,
char *Id,
DataCell **grid,
Lava_flow *active_flow,
double *geotransform,
File_output_type type,
FlowStats *stats) {

	FILE     *out;
	int row, col, i, j, k;
	double easting, northing, thickness, new_elev, orig_elev, value;
	char file[MAXLEN];
	/* GDAL variables */
	char projection[] = "";
	GDALDatasetH hDstDS;
	double GDALGeoTransform[6];
	GDALDriverH hDriver = GDALGetDriverByName("GTiff"); /*Try this*/
	GDALRasterBandH hBand;
	float   *RasterDataF = NULL;
	int raster_double_file = 0;
	CPLErr IOErr;
	
	/*Modify Metadata back to GDAL format*/
	GDALGeoTransform[0] = geotransform[0];
	GDALGeoTransform[1] = geotransform[1];
	GDALGeoTransform[3] = geotransform[3] + (geotransform[5] * geotransform[4]);
	GDALGeoTransform[5] = -1.0 * geotransform[5];
	GDALGeoTransform[2] = GDALGeoTransform[4] = 0;
	
	/*Create Data Block*/
	if (type == raster_hits || type == raster_flow || type == raster_pre || type == raster_post) {
		if((RasterDataF = GC_MALLOC (sizeof (float) * geotransform[2] *
						                     geotransform[4])) == NULL) {
			printf("[OUTPUT] Out of Memory creating Outgoing Raster Data Array\n");
			return(1);
		}
	}
	
		
		fprintf(stderr,"\nWriting output file: \n\n");
		switch(type) {
	
		case ascii_flow :
			sprintf (file, "flow_%s-%d", Id, run);
			fprintf(stdout, "Opening output file: %s\n", file);
			out  = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open ASCII FLOW file=[%s]:[%s]!\n",
				file, strerror(errno));
				return 1;
			}	
			/* Print file header */
			fprintf (out, "# VOLUME PULSE RESIDUAL VENTS\n");
		
			fprintf(out, "# %0.4f\t%0.4f\t%0.1f ",
			        active_flow->volumeToErupt, active_flow->pulsevolume, active_flow->residual ); 
			        
      for (i = 0; i < active_flow->num_vents; i++) {
				fprintf(out, " %0.3f\t%.03f\t", (active_flow->source+i)->easting, (active_flow->source+i)->northing);
			}
						
			fprintf (out, "\n# EAST NORTH THICKNESS NEW_ELEV ORIG_ELEV");
			
			/* Print data */
			for(row=0; row < geotransform[4]; row++) { 
				for(col=0; col < geotransform[2]; col++) {
					thickness = grid[row][col].eff_elev - grid[row][col].dem_elev;
					if (thickness > 0) {
						easting = geotransform[0] + (geotransform[1] * col);
					   northing = geotransform[3] + (geotransform[5] * row);
				   	new_elev = grid[row][col].eff_elev;
				   	orig_elev = grid[row][col].dem_elev;
				   	fprintf(out, "\n%0.3f\t%0.3f\t%f\t%f\t%f", easting, northing, thickness, new_elev, orig_elev);	
					}
				}
			}
			
			raster_double_file = 0;
			fflush(out);
		 fclose(out);
		 fprintf(stdout, " ASCII Output file: %s successfully written.\n \n", file);
		break;
		
		case ascii_hits :
			sprintf (file, "hits_%s-%d", Id, run);
			out = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open hits file: file=[%s]:[%s]! Exiting.\n",
	              file, strerror(errno));
	    	return 1;
			}

			for(row=0; row < geotransform[4]; row++) { 
				for(col=0; col < geotransform[2]; col++) {
					easting = geotransform[0] + (geotransform[1] * col);
					northing = geotransform[3] + (geotransform[5] * row);
					value = (double) grid[row][col].hit_count;
					if (value > 0) fprintf(out, "\n%0.3f\t%0.3f\t%0.0f", easting, northing, value);
				}
			}
			raster_double_file = 0;
			fflush(out);
			fclose(out);
			fprintf(stdout, " ASCII Hits file: %s successfully written.\n (x,y,hit count)\n", file);
		break;
		
		case raster_hits : /* Raster hits file */
			sprintf (file, "raster-hits_%s-%d", Id, run);
			out = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open hits file: file=[%s]:[%s]! Exiting.\n",
	              file, strerror(errno));
	    	return 1;
			}
			/*Assign Model Data to Data Block*/	
			k=0; /*Data Counter*/
			for (i = geotransform[4]; i > 0; i--) { 			/*For each row, TOP DOWN*/
				for(j=0; j < geotransform[2]; j++) {		/*For each col, Left->Right*/
						RasterDataF[k++] = (float) (grid[i-1][j].hit_count); 
					}
				}
			raster_double_file = 1;
		break;
		
		case raster_flow : /* Lava Thickness Raster */
			sprintf (file, "raster-flow_%s-%d", Id, run);
			out = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open raster flow file: file=[%s]:[%s]! Exiting.\n",
	              file, strerror(errno));
	    	return 1;
			}
			/*Assign Model Data to Data Block*/	
			k=0; /*Data Counter*/
			for (i = geotransform[4]; i > 0; i--) { 			/*For each row, TOP DOWN*/
				for(j=0; j < geotransform[2]; j++) {		/*For each col, Left->Right*/
					if(grid[i-1][j].active >= 0) {	
						RasterDataF[k++] = (float) (grid[i-1][j].eff_elev - grid[i-1][j].dem_elev); /* Calculate lava thickness */
					}
					else RasterDataF[k++] = (float) 0.0; /* Else print out 0  */
				}
			}
			raster_double_file = 1;
		break;
		
		case raster_post : /*POST FLOW TOPOGRAPHY RASTER*/
			sprintf (file, "post-dem_%s-%d", Id, run);
			out = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open post dem file=[%s]:[%s]! Exiting.\n",
	              file, strerror(errno));
	    	return 1;
			}
			/*Assign Model Data to Data Block*/
			k=0; /*Data Counter*/
			for (i = geotransform[4]; i > 0; i--) { /*For each row, TOP DOWN*/
				for (j = 0; j < geotransform[2]; j++) { /*For each col, Left->Right*/
					RasterDataF[k++] = (float) grid[i-1][j].eff_elev;
				}
			}
			raster_double_file = 1;
			
		break;
		
		case raster_pre :
		 sprintf (file, "pre-dem_%s-%d", Id, run);
		 fprintf(stderr, "[OUTPUT] Raster File Type: %d not implemented\n", raster_pre);
		 raster_double_file = 1;
		break;
		
		case stats_file :
		  sprintf(file, "stats_%s", Id);
		  out = fopen(file, "a");
		  if (out == NULL) {
				fprintf(stderr, "Cannot open stats file=[%s]:[%s]! Exiting.\n",
	              file, strerror(errno));
	    	return 1;
			}
			/* Write some stats to file */
			/* Print file header */
			if (!run) fprintf (out, "\nEvent,Runtime(s),Hit,Volume(km^3),Volume-Erupted(km^3),Cells-Inundated,Area-Inundated(km^2),Pulse-volume(m^3),Residual(m),Vents\n");
		  /* Print out data for this flow event */
			fprintf(out, "%d,%ld,%d,%0.2f,%0.2f,%d,%0.2f,%0.2f,%0.1f",
			        stats->event_id, stats->runtime, stats->hit,
			        stats->total_volume/1e9,
			        stats->volume_erupted/1e9,
			        stats->cells_inundated,
			        stats->area_inundated,
			        stats->pulse, 
			        stats->residual
			         ); 
			 for (i = 0; i < active_flow->num_vents; i++) {
				fprintf(out, ",%0.0f,%0.0f", (active_flow->source+i)->easting, (active_flow->source+i)->northing);
			}       
			fprintf(out, "\n");
			raster_double_file = 0;
			fflush(out);
		  fclose(out);
		  fprintf(stdout, " Stats file: %s successfully written.\n \n", file);
		break;
		case num_types :
		default :
			fprintf (stderr, "[OUTPUT] No ouput files specified!\n");
		}
	  //Write the raster file here if cells have double floating point values.
	  if (raster_double_file) {
		
		/*Setup Raster Dataset*/
		hDstDS = GDALCreate(hDriver, file, geotransform[2], 
			                  geotransform[4], 1, GDT_Float32, NULL);		
		GDALSetGeoTransform( hDstDS, GDALGeoTransform ); /*Set Transform*/
		GDALSetProjection( hDstDS, projection );     /*Set Projection*/
		hBand = GDALGetRasterBand( hDstDS, 1 );      /*Set Band to 1*/
		
		/*Write the formatted raster data to a file*/
		IOErr = GDALRasterIO(hBand, GF_Write, 0, 0, geotransform[2], geotransform[4],
		        RasterDataF, geotransform[2], geotransform[4], GDT_Float32, 0, 0);
		if(IOErr) {
			fprintf(stderr, "ERROR [OUTPUT]: Error from GDALRasterIO!!\n");
			return 1;
		}
		
		/*Properly close the raster dataset*/
		GDALClose( hDstDS );
		
		fprintf(stdout, "Raster Output file: %s successfully written.\n", file); 
	}
	return(0);
}
