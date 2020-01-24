# PitchPan_for_ROS
ROSに対応したピッチ軸雲台システム
# 説明
rosserialを使用して角度制御の指令を送る電動雲台。  
3kgまで搭載可能。 
角度制御の指令値はROSのservoトピックのメッセージで指定することができる。 
また、現在角度をdegreeトピックのメッセージで知ることができる。 
# デモ動画
https://youtu.be/2PF15hPyDig
デモ動画出は3kgの大型カメラが手元に無かったため、1.5Lの炭酸飲料で代用
# 使用機材
- ArduinoMicro
- バイポーラ　ステッピングモーター　SM-42BYG011
- ポテンショメータ RDC506
- その他ギア 減速比が1/18
# 使用方法 
``` 
$ roscore 
$ rosrun rosserial_python serial_node /(arduinoが刺さっているポート/dev/ttyACM0など) 
$ rostopic echo /degree # これで角度を計測 
$ rostopic pub std_msgs/Int16 /servo -- 角度　# これで角度制御の目標値を設定、角度を変更させる 
``` 
# 回路図
![%E9%9B%B2%E5%8F%B0%E5%9B%9E%E8%B7%AF%E5%9B%B3](https://user-images.githubusercontent.com/45557879/73052619-e71c6300-3ec8-11ea-894c-6f7757b05273.png)
