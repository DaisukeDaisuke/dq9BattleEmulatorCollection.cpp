## dq9BattleEmulatorCollection.cpp

dq9のRTA用バトルエミュレーター  


## how to build
Please build with [jetBrains clion](https://www.jetbrains.com/ja-jp/clion/) using cmake, ninja, or mingw.  
[jetBrains clion](https://www.jetbrains.com/ja-jp/clion/)のデフォルト設定(mingw)向けで作られています。なのでclionとかいうペイウォールさえ突破できればビルドできます。  

[稀に不安定なバージョンを無料で使えたりする](https://www.jetbrains.com/ja-jp/clion/nextversion/)ので、それでビルドするのはありかもしれません。ストレージめっちゃ食うけど

## branches
ブランチ紹介

### yo2
イシュダルの旅芸人レベル11 一人旅RTA装備用バトルエミュです。このブランチは最新のバグパッチが適応されています。  
デフォルトで総当たりモードになってたりなかったりします。総当たりモードにするには、debug.hのフラグを全て折ってください。   
i7 6700kでは総当たり(+-1.5秒)に0.3秒かかります。最新のcpuだと0.15秒程度しかかからないらしいです(最適化で爆速化に成功した)  

#### 引数解説
```
0 3 48 18 10 9 18 10 17 17 18 10 h 
```

- 1個目: X時間 (戦闘開始時の画面がひねるエフェクトが発生した時のリアルタイムタイマー、ライブスピリットとの誤差-15秒+-1秒想定です。)  
- 2個目: Y分  
- 3個目: Z秒  
- 4個目以降: ダメージ値。0ダメージの場合入力せず、ホイミはhです。

※2時間15分を越えるとバグって絶対に特定できない仕様があります。どちらかというとバグですが、あんまり影響ないのであえて放置します。  

https://github.com/DaisukeDaisuke/dq9BattleEmulatorCollection.cpp/tree/yo2


### buru_BruteForce

ブルドーガのバトルエミュレーターです。yo2では修正されたバグを何個か抱えてますが、動作に問題ないので放置してます。  
レベル1討伐とかで役に立つのに完全にリアルタイムタイマーとの誤差特定用になってます。  

https://github.com/DaisukeDaisuke/dq9BattleEmulatorCollection.cpp/tree/buru_BruteForce


### 追加要件

- 許可なく自作発言禁止(自作発言は一部の人のみ許可しています)
- これを使用したことによる実行したコンピーターやds、またはバグなどのミスによる不利益、損傷、または意図した通りに動作するかどうかについては一切保証せず、現状のままでの提供になります。これはバグ報告を制限するものではなく、自由にissueで報告することができます。
  - また、mingw版は悪意のあるプログラムと誤検知されるため配布する予定はありません。フォークしてvs版をGithub Actionsでビルドするか、クローンしてclionを使って各自でビルドしてください。
  - exeをVenusTotalにかけるなど、万全を期していますが、vs版や、mingw版(非公開)を実行した場合、プログラムに悪意のあるプログラムが混じっている可能性があることに同意することとします。
