# RealTime ビデオを操作ツール群

|ツール名|概要|注意事項|
|-|-|-|
| rmenc | rm ファイルコメント UTF-8変換ツール |開発中止 不具合あり|
| rmview | rm ファイルのコメント部分を UTF-8のCSV形式で出力する ||

# セットアップ方法

gcc と libiconvのインストール
make を実行

````
$ sudo apt install gcc libiconv-hook-dev
$ make
````

# rmenc 利用手順

````
$ ./rmenc [SJISタイトルの.rmファイル] [出力.rmファイル]
````

# rmview 利用手順

````
$ ./rmview [入力ファイル.rmファイル] [出力ファイル]
````

ファイル名,success,title,Auther,copyright,comment,message
のように出力します。

./rmchange.pl は rmファイルをmp4ファイル変換するサンプルスクリプト
