# OpenBlocksファームウェア作成ガイド

ぷらっとホーム株式会社

Copyright (c) 2013-2020 Plat'Home CO., LTD.

# 1. はじめに

このソフトウェアを使えばOpenBlocksのファームウェアを作成することができます。内容は、シェルスクリプトとファームウェアに含めるファイルです。

# 2. 対応ファームウェア

OpenBlocksのファームウェアは、機種(Aファミリ, IoTファミリ, 600)とOS(Debian GNU/Linux)によって異なります。対応しているファームウェアは下の表をご覧ください。

機種|Debian|TARGET|DIST|ARCH|ファームウェアを作成するホストのOS
---|---|---|---|---|---
IXR|10|obsix9|buster|amd64|Debian GNU/Linux 10/amd64
IXR|10|obsix9r|buster|amd64|Debian GNU/Linux 10/amd64
AX3|9|obsax3|stretch|armhf|Debian GNU/Linux 9/amd64
AX3|8|obsax3|jessie|armhf|Debian GNU/Linux 8/amd64
AX3|7|obsax3|wheezy|armhf|Debian GNU/Linux 8/amd64
A7|9|obsa7|stretch|armel|Debian GNU/Linux 9/amd64
A7|8|obsa7|jessie|armel|Debian GNU/Linux 8/amd64
A7|7|obsa7|wheezy|armel|Debian GNU/Linux 8/amd64
A6|7|obsa6|wheezy|armel|Debian GNU/Linux 8/amd64
600|7|obs600|wheezy|powerpc|Debian GNU/Linux 8/amd64
BXn|9|obsbx1s|stretch|i386|Debian GNU/Linux 9/amd64
BXn|8|obsbx1|jessie|i386|Debian GNU/Linux 8/amd64
BXn|7|obsbx1|wheezy|i386|Debian GNU/Linux 8/amd64
EX1|8|obsbx1|jessie|i386|Debian GNU/Linux 8/amd64
EX1|7|obsbx1|wheezy|i386|Debian GNU/Linux 8/amd64
VX2|10|obsvx2|buster|amd64|Debian GNU/Linux 10/amd64
VX2|9|obsvx2|stretch|amd64|Debian GNU/Linux 9/amd64
VX1|8|obsvx1|jessie|amd64|Debian GNU/Linux 8/amd64

BXn: BX0, BX1, BX1S, BX3, BX5

項目TARGETとDISTは、ファームウェアを作成するときに実行するシェルスクリプトに指定する文字列です。後節で参照します。

# 3. 準備

## 3.1. 作成ホスト

作成ホストのOSは、2. 対応ファームウェアの表を参照してください。ホストを用意して、そのOSをインストールします。ホストは物理マシンと仮想マシンのどちらでもよろしいです。インターネットに接続できるよう、ネッワークの設定をしてください。

## 3.2. Gitリポジトリの取得

このソフトウェアはGitリポジトリから取得できます。

作成ホストにgitパッケージをインストールします。

```
# apt-get update
# apt-get install git
```

Gitリポジトリを取得します。

```
$ git clone https://github.com/plathome/debian_based_firmware.git
```

以下では、ディレクトリdebian_based_firmwareに移動したものとして説明します。

### 3.2.1. タグの指定

リリースしたファームウェア毎にタグを付けています。以下のコマンド

```
$ git tag
```

でタグの一覧が表示されます。

ファームウェアのバージョンとタグの対応は、以下のコマンド

```
$ git show タグ名
```

や、以下のページ

https://github.com/plathome/debian_based_firmware/releases

から知ることができます。

タグ名を指定してファームウェアを作成するには以下のコマンドを実行します。

```
$ git checkout -b ブランチ名 タグ名
```

ここでブランチ名は、お好みの文字列を指定してください。

## 3.3. クロス開発環境

ファームウェアの作成に必要なパッケージをインストールします。

* 作成するホストのOSが、Debian GNU/Linux 8/amd64の場合。

```
# cd build_ramdisk
# ./build_crossenv.sh
```

* 作成するホストのOSが、Debian GNU/Linux 9/amd64の場合。

```
# cd build_ramdisk
# ./build_crossenv-stretch.sh
```

* 作成するホストのOSが、Debian GNU/Linux 10/amd64の場合。

```
# cd build_ramdisk
# ./build_crossenv-buster.sh
```

## 3.4. カーネルソース

### 3.4.1. 取得

カーネルソースをぷらっとホームのFTPサイトftp.plathome.co.jpからダウンロードします。ファイルbuild_ramdisk/config.shのcase文中のKERNELとPATCHLEVELからカーネルソースのFTPサイトに置いてある場所が分ります。

以下は場所の例です。

* TARGET=obsax3, DIST=wheezy, KERNEL=3.2.40, PATCHLEVEL=4の場合  
http://ftp.plathome.co.jp/pub/OBSAX3/wheezy/3.2.40-4/linux-3.2.40-20140220.tar.gz
* TARGET=obsa7, DIST=wheezy, KERNEL=3.2.40, PATCHLEVEL=4の場合  
http://ftp.plathome.co.jp/pub/OBSA7/wheezy/3.2.40-4/linux-3.2.40-20140220.tar.gz
* TARGET=obsa6, DIST=wheezy, KERNEL=3.2.40, PATCHLEVEL=3の場合  
http://ftp.plathome.co.jp/pub/OBSA6/wheezy/3.2.40-3/linux-3.2.40-20140220.tar.gz
* TARGET=obs600, DIST=wheezy, KERNEL=3.10.25, PATCHLEVEL=0の場合  
http://ftp.plathome.co.jp/pub/OBS600/debian/files/wheezy/3.10.25-0/linux-3.10.25-obs600.tar.gz
* TARGET=obsbx1, DIST=wheezy, KERNEL=3.10.17, PATCHLEVEL=15の場合  
http://ftp.plathome.co.jp/pub/BX1/wheezy/3.10.17-15/linux-3.10.17-20160309.tar.xz
* TARGET=obsvx1, DIST=jessie, KERNEL=4.4.26, PATCHLEVEL=1の場合  
http://ftp.plathome.co.jp/pub/OBSVX1/jessie/4.4.26-1/linux-4.4.26-obs-20161027.tar.xz

### 3.4.2. 展開

ディレクトリsourceには、TARGETをその名前としたディレクトリがあるので、そこにカーネルソースを展開してください。その結果、ディレクトリsource/TARGET/linux-KERNEL(およびlinux-KERNEL.orig)が作成されます。TARGETとKERNELは、ご自身の文字列に置き換えてください。

## 3.5. DVDイメージ

DebianのDVDイメージをDebianのミラーサイトからダウンロードしてディレクトリisofilesにコピーします。

ファイルbuild_ramdisk/config.shのcase文中のISOFILEからDVDイメージのファイル名が分ります。

以下はファイル名の例です。

* TARGET=obsax3, DIST=wheezyの場合  
debian-7.7.0-armhf-DVD-1.iso
* TARGET=obsa7, obsa6, DIST=wheezyの場合  
debian-7.7.0-armel-DVD-1.iso
* TARGET=obs600, DIST=wheezyの場合  
debian-7.4.0-powerpc-DVD-1.iso
* TARGET=obsbx1, DIST=wheezyの場合  
debian-7.8.0-i386-DVD-1.iso
* TARGET=obsvx1, DIST=jessieの場合  
debian-8.5.0-amd64-DVD-1.iso

これらのファイルは  
http://ftp.plathome.co.jp/pub/cdimages/debian/  
https://cdimage.debian.org/mirror/cdimage/archive/  
からも取得できます。

# 4. 作成

ファームウェアを作成します。シェルスクリプトbuild_ramdisk/buildall.shのオプション-MにTARGET、-DにDISTを指定します。TARGET=obsax3、DIST=wheezyの場合、

```
# cd build_ramdisk
# ./buildall.sh -M obsax3 -D wheezy
```

と実行します。

作成されたファームウェア一式は、ディレクトリrelease/TARGET/DIST/KERNEL-PATCHLEVELに置かれます。TARGET, DIST,KERNEL, PATCHLEVELは、ご自身の文字列に置き換えてください。

例えば、ディレクトリrelease/obsax3/wheezy/3.2.40-3には、以下のファイルが作成されます。

* MD5.obsax3  
MD5チェックサムファイル
* System.map  
カーネルのシステムマップファイル
* kernel-image-3.2.40-3.deb  
コマンドdpkgなどでインストールするためのパッケージ
* ramdisk-wheezy.obsax3.img.lzma  
ファームウェアのRAMディスクイメージ
* uImage.initrd.obsax3  
コマンドflashcfgでインストールするためのファームウェア
* zImage.gz  
ファームウェアのカーネルイメージ

# 5. カスタマイズ

本節ではファームウェアのカスタマイズの手順を説明します。

シェルスクリプトbuild_ramdisk/buildall.shを実行することは、DIST=wheezy, TARGET=obsax3の場合、以下の通り実行することと同じです。

```
# DIST=wheezy TARGET=obsax3 ./build_debootstrap.sh
# DIST=wheezy TARGET=obsax3 ./build_kernel.sh
# DIST=wheezy TARGET=obsax3 ./build_ramdisk.sh
# DIST=wheezy TARGET=obsax3 ./release_firmware.sh
```

TARGETとDISTについては、予め次のように設定することで、毎回の指定は不要になります。

```
# export DIST=wheezy TARGET=obsax3
```

内容の詳細は各シェルスクリプトを読んでいただくことにして、簡単に説明しておきます。以下のTARGETとDISTは、ご自身の文字列に置き換えてください。

* build_debootstrap.sh  
debootstrapを実行し、ディレクトリrootfs/DIST_TARGET以下にDebianを仮インストールしています。
* build_kernel.sh  
カーネルとカーネルモジュールのコンパイルをしています。
* build_ramdisk.sh  
ディレクトリrootfs/DIST_TARGET以下に仮インストールしたDebianをOpenBlocks用に変更して、RAMディスクイメージを作成しています。
* release_firmware.sh  
ファームウェア一式を作成しています。

上記のシェルスクリプトを手動で実行すれば、その途中で、カーネルとRAMディスクイメージのカスタマイズができます。

ファイルbuild_ramdisk/config.shのcase文中のPATCHLEVELには文字列などを追加、例えば3customとすることをお勧めします。公式ファームウェアとの区別がはっきりします。

カスタマイズの前に、その準備として、シェルスクリプトbuild_ramdisk/buildall.shを実行しておいてください。

## 5.1. カーネル

カーネルのコンフィグレーションを変更してカーネルをカスタマイズすることができます。カーネルをカスタマイズしない場合には、「5.2. RAMディスクイメージ」節に進んでください。

### 5.1.1. コンフィグレーションの変更

次のコマンドを実行して、カーネルのコンフィグレーションを変更します。TARGETとDISTは、ご自身の文字列に置き換えてください。

```
# DIST=wheezy TARGET=obsax3 ./kernel_menuconfig.sh
```

変更したコンフィグレーションファイルはsource/TARGET/linux-KERNEL.dot.configにコピーされます。TARGETとKERNELは、ご自身の文字列に置き換えてください。

### 5.1.2. コンパイル

シェルスクリプトbuild_ramdisk/build_kernel.shから順番に実行するか、シェルスクリプトbuild_ramdisk/buildall.shを実行してください。RAMディスクイメージをカスタマイズする場合、シェルスクリプトbuild_ramdisk/build_kernel.shを実行して、次節に進んでください。

## 5.2. RAMディスクイメージ

シェルスクリプトbuild_ramdisk/build_ramdisk.shの実行が終了すると、ディレクトリrootfs/DIST_TARGETに、RAMディスクに含まれるファイルが仮インストールされます。ディレクトリrootfs/DIST_TARGET以下のファイルを変更したり、パッケージを追加すれば、RAMディスクイメージのカスタマイズができます。DISTとTARGETは、ご自身のものに置き換えてください。

### 5.2.1. パッケージの取得

追加するパッケージをDebianのミラーサイトからダウンロードして、ディレクトリextradebs/DISTにコピーします。DISTは、ご自身の文字列に置き換えてください。

### 5.2.2. パッケージの追加

次のコマンドを実行してパッケージを追加します。DIST=wheezy,TARGET=obsax3の場合、

```
# DIST=wheezy TARGET=obsax3 ./08_add_extra_packages.sh
```

と実行します。

パッケージが追加されたかは、ファイルrootfs/DIST_TARGET/var/lib/dpkg/statusで確認できます。TARGETとDISTは、ご自身の文字列に置き換えてください。

### 5.2.3. RAMディスクイメージの作成

ディレクトリrootfs/DIST_TARGET以下のファイルをもとにRAMディスクイメージを作成します。TARGETとDISTは、ご自身の文字列に置き換えてください。DIST=wheezy, TARGET=obsax3の場合、

```
# DIST=wheezy TARGET=obsax3 ./99_create_ramdisk.sh
```

と実行します。

ファームウェアを作成します。DIST=wheezy, TARGET=obsax3の場合、

```
# DIST=wheezy TARGET=obsax3 ./release_firmware.sh
```

と実行します。

## 5.3. まとめ

一度「5.1.1. コンフィグレーションの変更」と「5.2.1. パッケージの取得」をすませておけば、シェルスクリプトbuild_ramdisk/buildall.shを実行するだけでカスタマイズのすんだファームウェアが作成できます。

# 6. ファームウェアの更新

ファームウェアパッケージkernel-image-KERNEL-PATCHLEVEL-TARGET.debをOpenBlocksにネットワーク経由もしくはUSBメモリでコピーしてください。KERNEL，PATCHLEVELとTARGETは、ご自身の文字列に置き換えてください。

次のコマンド

```
# dpkg -i kernel-image-KERNEL-PATCHLEVEL-TARGET.deb
```

を実行してください。

RAMディスクモードの場合はflashcfgコマンドを実行することをお勧めします。ファームウェアのバージョンとパッケージに関する情報内のバージョンが合わなくなる場合があります。（合わなくても動作に影響はありません。）

```
# flashcfg -S
```

再起動すれば、ファームウェアの更新は終了です。
