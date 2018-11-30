#include <prototypes_LJC.h>

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
File_output_type type,
Outputs *Out, 
Inputs *In, 
DataCell **grid, 
VentArr *vent,
double *geotransform) {

	FILE     *out;
	int row, col, i, j, k;
	double easting, northing, thickness, new_elev, orig_elev, value;
	char file[25];
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
	GDALGeoTransform[5] = -1 * geotransform[5];
	GDALGeoTransform[2] = GDALGeoTransform[4] = 0;
	
	/*Create Data Block*/
	if (type > 1) {
		if((RasterDataF = GC_MALLOC (sizeof (float) * geotransform[2] *
						                     geotransform[4])) == NULL) {
			printf("[OUTPUT] Out of Memory creating Outgoing Raster Data Array\n");
			return(1);
		}
	}
		
		fprintf(stderr,"\nWriting output file: \n\n");
		switch(type) {
	
		case ascii_flow :
			sprintf (file, "%s%d", Out->ascii_flow_file, run);
			out  = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open ASCII FLOW file=[%s]:[%s]!\n",
				file, strerror(errno));
				return 1;
			}	
			/* Print file header */
			fprintf (out, "# EAST NORTH VOLUME PULSE RESIDUAL\n");
			fprintf(out, "# %0.3f\t%.03f\t%0.4f\t%0.4f\t%0.1f\n",
			        vent->easting, vent->northing, vent->volumeToErupt, vent->pulsevolume, vent->residual );
			fprintf (out, "# EAST NORTH THICKNESS NEW_ELEV ORIG_ELEV");
			
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
		 fprintf(stderr, " ASCII Output file: %s successfully written.\n \n", file);
		break;
		
		case ascii_hits :
			sprintf (file, "%s%d", Out->ascii_hits_file, run);
			out = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open ASCII HITS file=[%s]:[%s]! Exiting.\n",
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
			fprintf(stderr, " ASCII Hits file: %s successfully written.\n (x,y,hit count)\n", file);
		break;
		
		case raster_hits : /* Raster hits file */
			sprintf (file, "%s%d", Out->raster_hits_file, run);
			out = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, "Cannot open RASTER HITS file=[%s]:[%s]! Exiting.\n",
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
			sprintf (file, "%s%d", Out->raster_flow_file, run);
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
				  thickness = grid[i-1][j].eff_elev - grid[i-1][j].dem_elev;
					if(thickness > 0) {	
						RasterDataF[k++] = (float) thickness; /* lava thickness */
					}
					else RasterDataF[k++] = (float) 0.0; /* Else print out 0  */
				}
			}
			raster_double_file = 1;
		
		break;
		
		case raster_post : /*POST FLOW TOPOGRAPHY RASTER*/
			sprintf (file, "%s%d", Out->raster_post_dem_file, run);
			out = fopen(file, "w");
			if (out == NULL) {
				fprintf(stderr, 
				  "Cannot open POST FLOW TOPOGRAPHY RASTER file=[%s]:[%s]! Exiting.\n",
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
		
		break;
		
		case stats_file :
		
		break;
		
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
		/* free(RasterDataF); */
		
		fprintf(stderr, "Raster Output file: %s successfully written.\n", file); 
	}
	return(0);
}
