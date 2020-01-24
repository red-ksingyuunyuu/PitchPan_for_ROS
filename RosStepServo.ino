#define USE_USBCON　　
#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif
 
#include <ros.h>            //ROS用ヘッダファイル
#include <std_msgs/Int16.h> //ROS用ヘッダファイル
#include <MsTimer2.h>       //割り込み用ヘッダファイル
 /*
#define START_A 190     //ポテンショメータの開始位置
#define END_A 743       //ポテンショメータの終了位置
#define START_B 200     //ポテンショメータの開始位置
#define END_B 753       //ポテンショメータの終了位置
*/
#define START_A 223     //ポテンショメータの開始位置
#define END_A 776       //ポテンショメータの終了位置
#define START_B 233     //ポテンショメータの開始位置
#define END_B 786       //ポテンショメータの終了位置
#define DEG_180 553     //180度回った時のAD値
#define START_OFFSET 0  //最初のオフセット
#define GEAR 6.67  //最初のオフセット
 
#define analogPinA 0     //ポテンショメーターの真ん中の端子をアナログピンの0番に接続
#define analogPinB 1     //ポテンショメーターの真ん中の端子をアナログピンの1番に接続
                        //両端の端子は，GNDと+5Vに接続
#define STEP_DIR 2
#define STEP_PULSE 3
int step_val = 0;       //ステッピングモータステップ変数
float deg_before;       //前回角度
 
int valA = 0;           //読み取った値を格納する変数
int valB = 0;           //読み取った値を格納する変数
int valC = 0;
 
ros::NodeHandle  nh;
 
//割り込みで2つのポテンショメータからAD値を算出する,前回AD値と今回AD値を比較して不感帯にいるか判断する
void get_analogsensor() {
  float raw_deg = 0.0;
  int diff_deg = 0;
  static boolean output = HIGH;
  valA = analogRead(analogPinA);
  valB = analogRead(analogPinB);
  //Bが不感帯なので、Aで角度を読み込む
  if(valA < (END_A + START_OFFSET) && valA > (START_A + START_OFFSET)){
    valA = analogRead(analogPinA) - (START_A + START_OFFSET);
    valB = 0;
   //Aが不感帯なのでBで角度を読み込む
  }else{
    valB = analogRead(analogPinB) - (START_B + START_OFFSET) + DEG_180;
    valA = 0;
  }
  //角度のAD値データ0-1160くらい
  valC = valA + valB;
  //valC = valC + START_OFFSET;
  //前回角度との偏差を求める
  diff_deg = valC - deg_before;
  //不感帯の処理
  if(diff_deg > 200 && diff_deg < 700){
    valC = analogRead(analogPinA) - (START_A + START_OFFSET);
  } else if( diff_deg < -200 && diff_deg > -700 ){
    valC = analogRead(analogPinB) - (START_B + START_OFFSET) + DEG_180;
  }
  //前回角度更新
  deg_before = valC;
  //動作確認用LED
  digitalWrite(13, output);
  output = !output;
}
 
void servo_cb( const std_msgs::Int16& cmd_msg){
  // step_motor
  servo_move(cmd_msg.data);
  //位置が微妙にずれている場合があるので、少し時間をおいてから、もう一度初期化を行う
  delay(500);
  servo_move(cmd_msg.data);
  //位置が微妙にずれている場合があるので、少し時間をおいてから、もう一度初期化を行う(3回くらいやれば、だいたいいい感じになる)
  delay(500);
  servo_move(cmd_msg.data);
  //end step
}
 
void servo_move(int angle){
  double tmp;
  //ステッピングモータの動作パルス=角度変換関数(目標値)*減速比
  //角度変換関数は現在角度から目標角度の偏差をAD値から度数に変換したもの
  tmp = servo_control(angle) * GEAR;
  // int型にキャスト
  step_val = tmp;
  //Serial.println(step_val);
  // 角度がプラスの場合
  if(step_val > 0.0){
      digitalWrite(STEP_DIR,LOW);
      for(int x = 0; x < step_val; x++) {
        digitalWrite(STEP_PULSE,HIGH); 
        delay(1); 
        digitalWrite(STEP_PULSE,LOW); 
        delay(1); 
      }
  // 角度がマイナスの場合
  }else{
      step_val = step_val * -1;
      digitalWrite(STEP_DIR,HIGH);
      for(int x = 0; x < step_val; x++) {
        digitalWrite(STEP_PULSE,HIGH); 
        delay(1); 
        digitalWrite(STEP_PULSE,LOW); 
        delay(1); 
      }
    }  
}
int servo_control(int angle){
  //引数:目標角度
  double step_pulse = 0.0;
  //元が-180だったので、180度のAD値を足せば0度
  double tmp = (angle * 3.072) + DEG_180;
  int deg = 0;
  //AD値を代入
  deg = valC;
  if(deg < tmp){
    step_pulse = ((deg / 3.072) - angle);
    step_pulse *= -1;
  } else {
    step_pulse = (int)(( 1106 - deg ) / 3.072)- angle;
  }
  return step_pulse;
}
 
ros::Subscriber<std_msgs::Int16> sub("servo", servo_cb);
std_msgs::Int16 deg;
ros::Publisher pub_deg("degree", &deg);
void setup(){
  pinMode(STEP_DIR,OUTPUT); 
  pinMode(STEP_PULSE,OUTPUT);
  pinMode(13, OUTPUT);
  MsTimer2::set(10, get_analogsensor);
  MsTimer2::start();
  //Serial.begin(9600);
 
  servo_move(0);
  //位置が微妙にずれている場合があるので、少し時間をおいてから、もう一度初期化を行う
  delay(500);
  servo_move(0);
  //位置が微妙にずれている場合があるので、少し時間をおいてから、もう一度初期化を行う(3回くらいやれば、だいたいいい感じになる)
  delay(500);
  servo_move(0);
  nh.initNode();
  nh.subscribe(sub);
  nh.advertise(pub_deg);
}
 
void loop(){
 
  float raw_deg = 0.0;
  raw_deg = (valC / 3.072);
  /*
  Serial.println("DEG=");
  Serial.println(raw_deg);
  Serial.println("A=");
  Serial.println(analogRead(analogPinA));
  Serial.println("B=");
  Serial.println(analogRead(analogPinB));
  delay(50);
  */
  deg.data = raw_deg;
  pub_deg.publish( &deg );
  nh.spinOnce();
  delay(1);
 
}
