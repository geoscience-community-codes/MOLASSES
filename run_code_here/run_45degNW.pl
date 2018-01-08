
$config_file = "config.45degNW";
$plot_file = "plot.gmt.pl";
$plot_conf = "plot.45degNW";

##### STOP - Do not edit past this line ###########################
system "date";
print "Running molasses .....\n";
system "nohup ../bin/molasses.ljc $config_file 0 1 2>./logfile >./output < /dev/null ";
print "........finished. Plotting lavaflow.\n";
system "perl $plot_file $plot_conf flow0";
print "Done.\n";
system "date";
##############################################################
