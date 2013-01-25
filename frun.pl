#!/usr/bin/perl -s

$ival	= shift();
$num	= shift()+0;
$cmd	= join(' ',@ARGV);

die("usage: hh:mm-HH:MM N command with arguments\n") unless ($ival && $num && $cmd);
die("wrong interval: $ival, must be hh:mm-HH:MM\n") unless ($ival =~ /^(\d{1,2}):(\d{2})-(\d{1,2}):(\d{2})$/);
$from = $1*100+$2;
$till = $3*100+$4;
@_ = localtime(time);
$time = $_[2]*100+$_[1];
$frun = $till>=$from ? ($time>=$from && $time<$till) : ($time>=$from || $time<$till);
$len = length($cmd);

my @pids;
my @ps = `/bin/ps xww -o pid= -o command= 2> /dev/null`;
foreach (@ps) {
	chomp;
	s/^ +//;
	($p,$c) = split(/ /,$_,2);
	push(@pids,$p) if (substr($c,0,$len) eq $cmd);
};

$run = @pids;

print "$from-$till time:$time frun:$frun num:$num run:$run cmd:$cmd\n";

if (!$frun || $num<$run) {
	for ($i=($frun ? $num : 0);$i<$run;$i++) {
		$p = $pids[$i];
		print "kill $p\n";
		kill(9,$p) || die("failed kill $p: @!");
	};
} elsif ($num>$run) {
	for ($i=$run;$i<$num;$i++) {
		print "run $cmd\n";
		system("$cmd > /dev/null 2> /dev/null &") && warn("failed run $cmd: @!");
	};
};

