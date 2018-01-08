$nrows = 130;
$ncols = 130;
$size = 10.0;
print "ncols	 $ncols\n";
print "nrows	 $nrows\n";
print "xllcorner	-5.0\n";
print "yllcorner	-5.0\n";
print "cellsize	$size\n";
for $row (0..129) {
	for $col (0..129) {
		$elev = 1200.0-$col;
		print "$elev ";
	}
	print "\n";
}
