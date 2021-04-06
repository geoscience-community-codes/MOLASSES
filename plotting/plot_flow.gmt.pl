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

# plot_flow.gmt.pl

#This script will plot the lava flow data on a DEM. 
#using GMT 6.0. Inputs include a configuration file,
# an ascii flow file (output from MOLASSES), a DEM file,
# preferably in GRD or geotif format.
# The name of the DEM and it's corrresponding intensity file should
# be listed in the config file: plot.conf

# To run this script type: perl plot_flow.gmt.pl plot.conf <flow-file>
# The output file will be a .png image file and a .eps image file 
# with the same basename as the datafile.

local $ENV{HOME} = ".";
use Carp; 

print STDERR "$0: Running .....\n";

sub open_or_die {
	my ($mode, $filename) = @_;
	open my $h, $mode, $filename
		or croak "Could not open '$filename': $!";
return $h;
}

################################
my $conf_file = $ARGV[0];
my %C;
my $conf = open_or_die("<", $conf_file);
my $key;
my $value;
while (<$conf>) {
  unless ($_ =~ /^#/ || $_ =~ /^\n/) {
  	my ($key, $value) = split "=",$_;
  	chomp($value);
	  $C{$key} = $value;
	  print STDERR "$key = $value\n";
  }
}

###############################

$in = $ARGV[1];
$out = "$in.eps";
#############################
$grd = $C{GRID_FILE};
$int = $C{INTENSITY_FILE};
$west = $C{WEST};
$east = $C{EAST};
$south = $C{SOUTH};
$north = $C{NORTH};
$grid = $C{GRID_SPACING};
$tick_int = $C{TICK_INTERVAL};
$map_scale = $C{MAP_SCALE};
#$tick = "a".$tick_int."g".($tick_int/2)."f".($tick_int/2);
$tick = "a".$tick_int."f".($tick_int/2);
$min = $C{MIN_DATA_VAL};
$max = $C{MAX_DATA_VAL};
$cint = $C{COLOR_INTERVAL};
`gmt makecpt -Ccopper -T$min/$max/1 -I -V > dem.cpt`;
$Tx = $west + 100;
$Ty = $south + 100;
$cities = $C{CITIES};
$vents = $C{VENTS};
##################################
my $flow = open_or_die("<", $in);
$line = <$flow>;
$line = <$flow>;
($s, $vol, $pulse, $resid) = split " ", $line;

my $max_color = 0;
while (<$flow>) {
	unless ($_ =~ /^#/ || $_ =~ /^\n/) {
	($a, $b, $thick, $elev1, $elev2) = split " ", $_;
	if ($thick > $max_color) {$max_color = $thick;}
	}	
}
#if ($max_color>=20 and $max_color < 30) {$max_color /= 2;}
#if ($max_color >=30 and $max_color < 40) {$max_color /= 3;}
#if ($max_color >= 40) {$max_color /= 4;}

print STDERR "max color bar = $max_color\n";
$max_color = $resid;
# $max_color = $vol;
$max_color *= 2.0;
$max_color = int($max_color);
$lavacpt = "lava.cpt";

`gmt makecpt -Chot.cpt -T0/$max_color/.1 -I -V > $lavacpt`; #CHANGE THIS TO SHOW MORE OR LESS COLORS (rule of thumb = 1.5xResidual)

# -E60/50/=/.5 -Nt0.5
# -E-45/60/.5/.2/.2/100 -Nt0.5
# -E60/60/=/.5 -Ne0.4
# -E45/20/.5/.2/.2/10 -Nt0.4165
# -Gint.grd=nb/a -Es0/30 (Fogo)

print STDERR "Intensity file = $int\n";
unless (-e $int) {
`gmt grdgradient $grd -G$int=nb/a -Es0/30 -V`;
}
`gmt set MAP_ANNOT_OFFSET_PRIMARY=2p MAP_FRAME_AXES=WSne FONT_ANNOT_PRIMARY=9p MAP_FRAME_TYPE=inside` ;

# For ascii data file: \`gmtinfo $vents -I-\`
#for grd file:  \`grdinfo $grd -I-\`

`gmt grdimage $grd -Jx1:$map_scale \`gmt grdinfo $grd -I-\` -X1i -Y1i -Cdem.cpt -I$int -P -K -V > $out`;
`gmt psxy $cities -R -Jx -Ss0.5c -Gwhite -Wthick,green -O -K -V>> $out`;
`gmt psxy $in -R -Jx -Sc0.01c -Clava.cpt -O -K -V>> $out`;
`gmt psxy $vents -R -Jx -Sc0.05c -Gred -Wthinnest,0 -O -K -V >> $out`;

#`gmt psxy   -R -Jx  -Sc0.1c -Gred -Wthinnest,0 -O -K -V <<EOF>> $out
#$ea $no 
#EOF`;

$lab = ($max_color >= 10) ? int($max_color/10) : round($max_color/10);

`gmt psscale --FORMAT_FLOAT_OUT=%.0f --FONT_LABEL=8p --MAP_ANNOT_OFFSET_PRIMARY=5p --FONT_ANNOT_PRIMARY=7p -Dx8c/-1c+w8c/0.2c+jBC+h -C$lavacpt -Bxaf+l'Lava Thickness (m)' -O -K -V >> $out`;

#-T$Tx/$Ty/2c
#`gmt pstext --FONT_ANNOT_PRIMARY=8p,Helvetica-Narrow-Bold,0 $cities  -R -Jx -O -K -V >> $out`;
`gmt pstext -F+f+a+j $cities -G250 -C0.1p -R -Jx -O -K -V >> $out`;

`gmt psbasemap -R -Jx -Bx$tick -By$tick  -O -V >> $out`;
`gmt psconvert $out -A -P -Tg`;
`rm $out *.cpt gmt.conf gmt.history`;
`rm -rf .gmt`;
#`/usr/bin/ristretto $in.png`;

sub round {
  $_[0] > 0 ? int($_[0] + .5) : -int(-$_[0] + .5)
}

