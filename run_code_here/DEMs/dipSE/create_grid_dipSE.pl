$nrows = 130;
$ncols = 130;
$size = 10.0;
print "ncols	 $ncols\n";
print "nrows	 $nrows\n";
print "xllcorner	-5.0\n";
print "yllcorner	-5.0\n";
print "cellsize	$size\n";
$corner = 1200.0;
$skip = 0.0;
for $row (0..129) {
	for $col (0..129) {
		$elev = $corner - $skip;
		$skip += 1.0;
		print "$elev ";
	}
	print "\n";
	$corner = 1200.0;
	$skip = 1.0; 
	$corner -= ($skip * $row);
}
