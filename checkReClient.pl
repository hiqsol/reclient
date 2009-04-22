#!/usr/bin/perl -s

$COMM	= shift(@ARGV);
$CONF	= shift(@ARGV);

$COMM	= "/home/sol/prj/reclient/reClient"		unless ($COMM);
$CONF	= "/home/sol/prj/revo/etc/verisign.epp01.rsx"	unless ($CONF);
@PROG	= ($COMM,$CONF);

sub get_cpu {
	my $comm = shift;
	@ps = `ps -arxo pid,%cpu,comm | grep '$comm'`;
	foreach $s (@ps) {
		chomp($s);
		print "s: '$s'\n";
		@a = split(/\s+/,$s,3);
		$c = pop(@a);
		return @a if ($c eq $comm);
	};
	return undef;
};

sub start {
	exit(exec(@PROG));
};

do {
	($pid,$cpu) = get_cpu('reClient');
	if (!defined($cpu)) {
		# start it
		start();
	} elsif ($cpu<20) {
		exit 0;
	} elsif ($n>3) {
		# restart it;
		kill(9,$pid);
		start();
	};
	$n++;
	sleep(1);
} while ($n<100);

