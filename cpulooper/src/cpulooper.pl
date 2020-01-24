#!/usr/bin/perl


# 設定
# 負荷: 開始時間
$start_hour = 0;
# 負荷: 停止時間
$end_hour   = 24;
# 負荷: 開始曜日
$start_wday = 0;
# 負荷: 停止曜日
$end_wday   = 7;

my $i, $j;

my $sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst;


while(1)
{
  # 時刻取得
  ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
  $mon++;
  $year += 1900;
  #print "$sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst\n";
  # 時間内の時は負荷をかける
  if( $wday >= $start_wday && $wday <= $end_wday &&
      $hour >= $start_hour && $hour <= $end_hour ) {
    # 負荷
    $i = rand(65536);
    $j = rand(65536);
  } else {
    # 1分 ストップ
    sleep(60);
  }
}

