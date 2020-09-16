#!/usr/bin/perl

# AWS Cloud Trail log簡易分析
# 
# Cloud Trailのログをjson形式でディレクトリに保存して
# このスクリプトを実行するとARN毎のログ件数をカウントするだけの
# 簡易プログラムです。

use JSON;

@list = glob('*.json');

$count = {};

foreach my $f (@list){
  my $jtext = do { open FI, $f; <FI> };
  my $json = decode_json($jtext);
  my $rnum = @{$json->{'Records'}};
  #print $f, ':' , $rnum,"\n";  
  for my $r (@{$json->{'Records'}}) {
    my $arn = $r->{'userIdentity'}->{'arn'};
    if ( exists($count->{$arn}) ) { $count->{$arn} += 1; }
    else {$count->{$arn} = 1;}
  }
}

foreach my $k (keys(%{$count} )){
  print $k, ':' , $count->{$k}, "\n";
}
