# plot.gmt.pl

# To run this script type: perl plot.gmt.pl plot.conf flow0
# The output file will be a .png image file and a .eps image file 
# with the same basename as the datafile.
 
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
  }
}

###############################

$in = $ARGV[1];
$out = "$in.eps";
#############################
$grd_file = $C{GRID_FILE};
$west = $C{WEST};
$east = $C{EAST};
$south = $C{SOUTH};
$north = $C{NORTH};
$grid = $C{GRID_SPACING};
$tick_int = $C{TICK_INTERVAL};
$map_scale = $C{MAP_SCALE};
$tick = "a".$tick_int."f".$tick_int/2;

$Tx = $west + 100;
$Ty = $south + 100;
##################################
my $flow = open_or_die("<", $in);
$line = <$flow>;
$line = <$flow>;
($s, $ea, $no, $vol, $pulse, $resid) = split " ", $line;
print $line;

my $max_color = 0;
while (<$flow>) {
	unless ($_ =~ /^#/ || $_ =~ /^\n/) {
	($a, $b, $thick, $elev1, $elev2) = split " ", $_;
	if ($thick > $max_color) {$max_color = $thick;}
	}	
}
print "max_color=$max_color\n";
# if ($max_color>=20 and $max_color < 30) {$max_color /= 2;}
# if ($max_color >=30 and $max_color < 40) {$max_color /= 3;}
# if ($max_color >= 40) {$max_color /= 4;}


$max_color = $resid;
$max_color *= 2.0;
$lavacpt = "lava.cpt";
print "max color bar = $max_color\n";
#-E60/50/=/.5 -Nt0.5
# -E-45/60/.5/.2/.2/100 -Nt0.5
# -E60/60/=/.5 -Ne0.4
#`gmt grdgradient $grd -G$int -E25/75/.5/.2/.2/100 -Nt0.5 -V`;
# MAP_FRAME_TYPE=inside

`gmtset MAP_ANNOT_OFFSET_PRIMARY=2p MAP_FRAME_AXES=WSNE FONT_ANNOT_PRIMARY=8p`;
`gmt makecpt -Chot -T0/$max_color/.1 -I -V > $lavacpt`; #CHANGE THIS TO SHOW MORE OR LESS COLORS (rule of thumb = 1.5xResidual)
#`gmt grdimage $grd -Jx1:$map_scale -R$west/$east/$south/$north -X1i -Y1i -Cdem.cpt -E300 -P -K -V > $out`;

`gmt psxy $in -Jx1:$map_scale -R$west/$east/$south/$north -X1i -Y1i -Ss0.2c -C$lavacpt -Wthinnest,100 -K -P -V > $out`;
`gmt grdcontour $grd_file -Jx -R -C10 -A5 -Wthinnest,150 -V -K -O >> $out`;
`gmt psxy   -R -Jx  -Sc0.1c -Gred -Wthinner,0 -O -K -V <<EOF>> $out
$ea $no 
EOF`;
#`gmt psxy  -R -Jx  -Sc0.1c -Gblack -O -K <<EOF>> $out
#590579 1325032  
#EOF`;
$lab = ($max_color >= 10) ? int($max_color/10) : 0.2;
$lab = sprintf "%.1f", ($max_color/10.0);
`gmt psscale --FORMAT_FLOAT_OUT=%.0f --FONT_LABEL=8p --MAP_ANNOT_OFFSET_PRIMARY=5p --FONT_ANNOT_PRIMARY=9p -D9c/-1.0c/10c/0.2ch -C$lavacpt -Ba$lab:'Lava Thickness (m)':/:'': -O -K >> $out`;

#-T$Tx/$Ty/2c
`gmt psbasemap -R -Jx -Bx$tick -By$tick  -O -V >> $out`;
`gmt psconvert $out -A -P -Tg -V`;
`rm $out $lavacpt`;

sub round {
  $_[0] > 0 ? int($_[0] + .5) : -int(-$_[0] + .5)
}

