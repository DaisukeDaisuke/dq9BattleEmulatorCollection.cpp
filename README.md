## dq9BattleEmulatorCollection.cpp

dq9のRTA用バトルエミュレーター  


## how to build
Please build with [jetBrains clion](https://www.jetbrains.com/ja-jp/clion/) using cmake, ninja, or mingw.  
[jetBrains clion](https://www.jetbrains.com/ja-jp/clion/)のデフォルト設定(mingw)向けで作られています。なのでclionとかいうペイウォールさえ突破できればビルドできます。  

[稀に不安定なバージョンを無料で使えたりする](https://www.jetbrains.com/ja-jp/clion/nextversion/)ので、それでビルドするのはありかもしれません。ストレージめっちゃ食うけど

## branches
ブランチ紹介

### yo2
イシュダルのレベル11 RTA装備用バトルエミュです。このブランチは最新のバグパッチが適応されています。  
デフォルトで総当たりモードになってたりなかったりします。総当たりモードにするには、debug.hのフラグを全て折ってください  

https://github.com/DaisukeDaisuke/dq9BattleEmulatorCollection.cpp/tree/yo2


### buru_BruteForce

ブルドーガのバトルエミュレーターです。yo2出は修正されたバグを何個か抱えてますが、動作に問題ないので放置してます。

https://github.com/DaisukeDaisuke/dq9BattleEmulatorCollection.cpp/tree/buru_BruteForce

