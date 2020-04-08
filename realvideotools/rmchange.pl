#!/usr/bin/perl

my @filelist = (map{ $_ =~ tr/\n\r//d; $_ } `find . -name '*.rm'`,
	    map{ $_ =~ tr/\n\r//d; $_ } `find . -name '*.ram'`);

foreach my $file (@filelist){
  my ($fname, $status, $title, $auther, $copyright, $comment, $message)
       = split(/,/, `./rmview ${file} | tee -a rmview.log`);
  $message =~ s/\n$//;
  if($status ne 'success'){
    printf("%s : ERROR (rmview)\n", $file);
    next;
  }
  my $out = `mplayer -dumpstream ${fname} -really-quiet`;
  $file =~ s/\.rm$/\.mp4/;
  $file =~ s/\.ram$/\.mp4/;
  $out .= `ffmpeg -i stream.dump ${file} -loglevel quiet`;
  `rm stream.dump`;
  if($message eq 'encode SJIS to UTF8' ||
     $message eq 'change copyright symble'){
    my $opt = "";
    $opt .= " --title '${title}'" if ${title};
    $opt .= " --comment '${comment}'" if ${comment};
    $opt .= " --copyright '${copyright}'" if ${copyright};
    $out .= `AtomicParsley ${file}${opt}` if $opt; 
    `rm ${file}`;
    my $f2 = $file;
    $f2 =~ s/\.mp4$/-temp-*.mp4/;
    `mv ${f2} ${file}`;
  }
  printf("%s : converted to mp4 \n", $file);

}
