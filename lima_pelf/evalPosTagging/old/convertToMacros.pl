#! /usr/bin/perl -s

open(TABLE,$ARGV[0]) || die ("impossible d ouvrir le fichier $ARGV[0]");

#lit la table

while ($ligne = <TABLE>)
{
	chomp $ligne;
    @data = split(/\|/,$ligne);

	$micro=$data[0];
	$macro=$data[1];

    $mapping{ $micro } = $macro;

}

#pour chaque fichier fait les remplacements
for ($i=1;$i<scalar(@ARGV);$i++)
{
    push(@allFiles,$ARGV[$i]);
}

while (scalar(@allFiles)>0)
{

    $inputFile = pop(@allFiles);
	$outputFile = "$inputFile.macro";

    open(INPUT,$inputFile) || die ("impossible d ouvrir le fichier $inputFile");
    open(OUTPUT,">$outputFile") || die ("impossible d ouvrir le fichier $outputFile");

	print "process $inputFile into $outputFile\n";

	while ($ligne = <INPUT>) {
	chomp $ligne;
        @data = split(/\|/,$ligne);

    	for ($i=2;$i<scalar(@data);$i++)
    	{
	        chomp($data[$i]);
    	    $data[$i]=~ s/^\s*//;
    	    $data[$i]=~ s/\s*$//;

			$macro = $mapping{$data[$i]};
			if ($macro eq "")
			{
			    print "WARNING : micro $data[$i] has no corresponding macro !\n";
			}
			push(@macros, $macro);
	    }
		sort(@macros);

		print OUTPUT "$data[0] | $data[1]";
		$lastMacro="truc";
		while (scalar(@macros)>0)
		{
		    $current=pop(@macros);
			if ($current eq $lastMacro)
			{
			    next;
			} else {
			    $lastMacro=$current;
				print OUTPUT " | $current";
			}
		}
		print OUTPUT "\n";

	}

}

