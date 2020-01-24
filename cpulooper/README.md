# CPU Looper

簡易的なループスクリプトでCPU負荷を高めるために利用
処理優先度を下げているため、CPU高付加状態でもSSHなどを行えるように調整しています。


# 注意事項

本ツールを利用するとCPU使用率が高くなります。
CPU使用率を好意的に上昇させる場合のみご利用ください。

# 利用方法

## 動作環境

perl 5 がインストールされている Linux環境

## インストール方法

```
sudo ./install.sh
```

## 利用方法

負荷をかける場合

```
$ sudo systemctl start cpulooper
```

負荷を止める場合

```
$ sudo systemctl stop cpulooper
```
 
