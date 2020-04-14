#!/usr/bin/perl

use JSON;
use Encode;

my @filelist = (map{ $_ =~ tr/\n\r//d; $_ } `find . -name '*.rm'`,
	    map{ $_ =~ tr/\n\r//d; $_ } `find . -name '*.ram'`);

foreach my $file_rm (@filelist){
  my $file_mp4   = $file_rm;
  my $file_rm_e  = $file_rm;
  $file_rm_e     =~ s/([() ])/\\$1/g; # エスケープ文字挿入
  my $file_mp4_e = $file_rm_e;
  $file_mp4   =~ s/\.rm$/\.mp4/;
  $file_mp4   =~ s/\.ram$/\.mp4/;
  $file_mp4_e =~ s/\.rm$/\.mp4/;
  $file_mp4_e =~ s/\.ram$/\.mp4/;
  next if(-f $file_mp4);
  # SJISのタイトル取得
  my $idata 
       = `./rmview ${file_rm_e} | tee -a rmview.log`;
  my $data = decode_json ($idata);
  $message =~ s/\n$//;
  # 取得エラーがある場合は終了
  if($data->{state} ne 'success'){
    printf("%s : ERROR (rmview)\n", $file_rm);
    next;
  }
  # mplayerでストリームダンプ
  my $out = `mplayer -dumpstream ${file_rm_e} -really-quiet`;
  # ストリームダンプからmp4を作成
  $out .= `ffmpeg -i stream.dump ${file_mp4_e} -loglevel quiet`;
  # 不要なストリームダンプを削除
  `rm stream.dump`;
  # タイトル部分がSJISやcopyrightマークがある場合は以下の処理を行う
  if($data->{msg} eq 'encode SJIS to UTF8' ||
     $data->{msg} eq 'change copyright symble'){
    # タイトル、コメント、著作権の上書き
    my $d = $data->{data};
    my $opt = "";
    $opt .= " --title '$d->{title}'" if $d->{title};
    $opt .= " --comment '$d->{comment}'" if $d->{comment};
    $opt .= " --copyright '$d->{copyright}'" if $d->{copyright};
    $opt .= " --keyword '$d->{Keywords}'" if $d->{Keywords};
    $opt .= " --description '$d->{Description}'" if $d->{Description};
    $opt = encode('UTF-8',$opt);
    $out .= `AtomicParsley ${file_mp4_e} ${opt}` if $opt; 
    # xxx-temp-xxxx.mp4 ファイルができるので古いファイルの削除して名前変更
    `rm ${file_mp4_e}`;
    my $file_mp4_temp = $file_mp4_e;
    $file_mp4_temp    =~ s/\.mp4$/-temp-*.mp4/;
    `mv ${file_mp4_temp} ${file_mp4_e}`;
  }
  printf("%s : converted to mp4 \n", $file_rm);
  printf($out);

}
