==========================================
インストールと実行
==========================================

Teaching Pluginはユーザが教示により作成したロボットの動作に関するデータを一旦保存し、類似した作業に対して保存したデータを再利用するためのツールです。


環境
========
* Ubuntu Linux 16.04LTS
* Choreonoid 1.6 （githubから開発版をダウンロード）


インストールとビルド
========

ダウンロード::

  $ cd $CHOREONOID_DIR/ext
  $ git clone https://bitbucket.org/hanai/teachingplugin.git teachingPlugin
  $ git clone https://bitbucket.org/hanai/samplehirocontrollerplugin.git SampleHiroControllerPlugin

SQLiteのインストール::

  $ sudo apt-get install libqt4-sql-sqlite libsqlite3-0 libsqlite3-dev sqlite3

ビルド::

  $ cd $CHOREONOID_DIR/build
  $ ccmake ..
  BUILD_TEACHING_PLUGIN=ON, BUILD_SAMPLE_HIRO_CONTROLLER=ONにする（デフォルトでON）
  $ make
  make -j8などとすると時間短縮になります。

実行
========

実行::

  $ cd $CHOREONOID_DIR/ext/teachingPlugin/share/project
  $ $CHOREONOID_DIR/build/bin/choreonoid teaching_plugin.cnoid
