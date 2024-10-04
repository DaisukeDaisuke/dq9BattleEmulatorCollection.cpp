## dq9BattleEmulatorCollection.cpp

dq9のRTA用バトルエミュレーター  


## how to build
Please build with [jetBrains clion](https://www.jetbrains.com/ja-jp/clion/) using cmake, ninja, or mingw.  
[jetBrains clion](https://www.jetbrains.com/ja-jp/clion/)のデフォルト設定(mingw)向けで作られています。なのでclionとかいうペイウォールさえ突破できればビルドできます。  

[稀に不安定なバージョンを無料で使えたりする](https://www.jetbrains.com/ja-jp/clion/nextversion/)ので、それでビルドするのはありかもしれません。ストレージめっちゃ食うけど

## branches
ブランチ紹介

### yo2
イシュダルの旅芸人レベル11 RTA装備用バトルエミュです。このブランチは最新のバグパッチが適応されています。  
デフォルトで総当たりモードになってたりなかったりします。総当たりモードにするには、debug.hのフラグを全て折ってください。   
i7 6700kでは総当たり(+-1秒)に18秒かかります。最新のcpuだと8秒程度しかかからないらしいです。  

#### 引数解説
```
0 3 48 18 10 9 18 10 17 17 18 10 h 
```

- 1個目: X時間 (戦闘開始時の画面がひねるエフェクトが発生した時のリアルタイムタイマー、ライブスピリットとの誤差-15秒+-1秒想定です。)  
- 2個目: Y分  
- 3個目: Z秒  
- 4個目以降: ダメージ値。0ダメージの場合入力せず、ホイミはhです。

https://github.com/DaisukeDaisuke/dq9BattleEmulatorCollection.cpp/tree/yo2


### buru_BruteForce

ブルドーガのバトルエミュレーターです。yo2では修正されたバグを何個か抱えてますが、動作に問題ないので放置してます。

https://github.com/DaisukeDaisuke/dq9BattleEmulatorCollection.cpp/tree/buru_BruteForce

