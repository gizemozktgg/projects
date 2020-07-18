#include <LiquidCrystal.h>
#include <Wire.h>
#include <string.h>

#define b1 2
#define b2 8
#define b3 3
#define b4 9
#define tc74  A0
#define address 77

LiquidCrystal lcd(12,11,4,5,6,7);

int buzzer = 13;
byte degree=0;
int zaman = 0;

int button1_s=LOW;
int button2_s=LOW;
int button3_s=LOW;
int button4_s=LOW;

int flag1 = false;
int flag2 = false;              //alarm ve saat için dakika mı saat mi ayarlanacak onlar için
int flag3 = false;
int flag4 = false;

int clockFlag = false;          //alarmın set edilebilmesini ve clock un set edilebilmesini sağlar
int AlarmFlag = false;
int toneFlag = false;           //alarmın susması veya çalması durumları için
int setted = false;
int ready_a = false;
int temperatureFlag = false;
int modeflag = false;           //12-24
int tF = true;                  //alarm'ı set ederken derece modunun değişmemesi için
int a_again = 0;


int count=0;                    //Saati ayarlarken dakikadan saat kurmaya geçme ve set etme kontrolü olarak kullanıyorum
int counta=0;                   //Alarm'ı ayarlarken dakikadan saat kurmaya geçme ve set etme kontrolü olarak kullanıyorum
int start_modeButton = 0;
int a_hour=0;
int a_min=0;

double temp;                    //Derece 

long buttonTimer = 0;
long longPressTime = 3000;

boolean buttonActive = false;
boolean longPressActive = false;


long buttonTimer1 = 0;
long longPressTime1 = 3000;

boolean buttonActive1 = false;
boolean longPressActive1 = false;

long alarmTimer = 0;

//for alarm
int starttime;
int activetime;
int prevoustime = 0;

void setup() {

  cli(); 
  
 
  TCNT1= 0; 
  TCCR1A = 0;
  TCCR1B = 0;

  TCCR1B |= (1<<WGM12); 
  TCCR1B |= (1<<CS12)|(1<<CS10); 
  OCR1A = 15624; //(16*10^6)/(1*1024)-1
  TIMSK1 |= (1<<OCIE1A); 
  
  pinMode(b1, INPUT);           
  pinMode(b2, INPUT_PULLUP);           
  pinMode(b3, INPUT_PULLUP);
  pinMode(b4, INPUT_PULLUP);             
  pinMode(buzzer, OUTPUT);  
  pinMode(tc74, INPUT);   

    
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(20,4);
  lcd.setCursor(4,1);
  lcd.print("SET THE CLOCK");
  
  attachInterrupt(digitalPinToInterrupt(b1),  Time12_24, FALLING);
  attachInterrupt(digitalPinToInterrupt(b3),  Temperature, FALLING);

  sei();
}

 int hour=0;
 int min=0;
 int sec=0;


void loop() {

  button1_s = digitalRead(b1);
  button2_s = digitalRead(b2); 
  button3_s = digitalRead(b3);
  button4_s = digitalRead(b4);

  

//Burada eğer saatin açılışı yapılmışsa derece gösteriliyor
  if(setted && tF){
  TC74();
  }
  
  
//---------------------------------------------------------

  
  ClockSetUp();  
  if(clockFlag && !setted){                               //Burada saatin ayarlama modunda saat ayarlamak için işlemler yapılıyor
        lcd.setCursor(2,1);
        if(hour<10) lcd.print("0");
        lcd.print(hour);
        lcd.print(':');
        if(min<10) lcd.print("0");
        lcd.print(min);
        lcd.print(':');
        if(sec<10) lcd.print("0");
        lcd.print(sec);
     
     C_Blink_min();
     C_Blink_hour();
     
    if(flag1){                           //dakika ayarlama kısmı
      if(button3_s){
        min++;
        if(min>=60) min=0;
        delay(500);}
        
    }if(flag2){                          //saat ayarlama kısmı
      if(button3_s){
        hour++;
        if(hour>=24)hour=0;
        delay(500);}
    }  
  }
   

  DisplayClock_Mode(hour, min);              //12-24 saat dilimleri arasında geçiş yapıyor
  
  AlarmSetUp();                             //Burada alarm'ın ayarlama modunda saat ayarlamak için işlemler yapılıyor
  if(AlarmFlag){
    if(a_again>1){  //set alarm 
      a_hour=0; 
      a_min=0;
    }
    lcd.setCursor(13,1);
    if(a_hour<10) lcd.print("0");
    lcd.print(a_hour);
    lcd.print(':');
    if(a_min<10) lcd.print("0");
    lcd.print(a_min);
       A_Blink_min();
       A_Blink_hour();
      if(flag3){                            //dakika ayarlama kısmı
      if(button3_s){
        a_min++;
        if(a_min>=60) a_min=0;
        delay(500);
      }
    }if(flag4){                             //saat ayarlama kısmı
      if(button3_s){
        a_hour++;
        if(a_hour>=24)a_hour=0;
        delay(500);
      }
    }   
    
  }
  

//------------eğer saat ile  alarm saati eşitse ve saat ayarlıysa ve alarm da kurulmuşsa çalar------------------------------------
                                                          //burda istediğim şey alarm'ı kurarken piezonun çalmaya başlamaması için
if(a_min == min && a_hour == hour && ready_a && setted){
  Alarm();
  if(button4_s){                                          //alarmın çalması ve butona basılınca alarm susması için yapılan işlemler
    toneFlag = !toneFlag;
    starttime = millis()/1000;
}
}

}

int last_blink = 0;
boolean seen = false;

void C_Blink_min(){                   //Burada delay yerine millis kullandım  blink yapıyor -> clock saati ayarlama esnasında
 
 if(flag1 && !setted){
  if(seen){
  lcd.setCursor(5,1);
  if(min<10) lcd.print("0"); lcd.print(min);
  }
  else{
  lcd.setCursor(5,1);
  lcd.print("  ");
  }
 
 } if(millis()-last_blink > 1000){
   seen = !seen;
   last_blink = millis();}
}

void C_Blink_hour(){
  
 if(flag2 && !setted){
  if(seen){
  lcd.setCursor(2,1);
  if(hour<10) lcd.print("0"); lcd.print(hour);
  }
  else{
  lcd.setCursor(2,1);
  lcd.print("  ");
  }
  if(millis()-last_blink > 1000){
   seen = !seen;
   last_blink = millis();}
 }
}


void A_Blink_min(){
                                   //Burada delay yerine millis kullandım  blink yapıyor -> alarm saati ayarlama esnasında
 if(flag3 && !ready_a){
  if(seen){
  lcd.setCursor(16,1);
  if(a_min<10) lcd.print("0"); lcd.print(a_min);
  }
  else{
  lcd.setCursor(16,1);
  lcd.print("  ");
  }
  if(millis()-last_blink > 1000){
   seen = !seen;
   last_blink = millis();}
 }
}

void A_Blink_hour(){
  
 if(flag4 && !ready_a){
  if(seen){
  lcd.setCursor(13,1);
  if(a_hour<10) lcd.print("0"); lcd.print(a_hour);
  }
  else{
  lcd.setCursor(13,1);
  lcd.print("  ");
  }
  if(millis()-last_blink > 1000){
   seen = !seen;
   last_blink = millis();}
 }
}


void setScreen()
{                                         //ekran görüntüsü oluşturma kısmı
  lcd.setCursor(0,0);
  lcd.print("CLOCK");
  lcd.setCursor(11,0);
  lcd.print("|");
  lcd.setCursor(11,1);
  lcd.print("|");
  lcd.setCursor(11,2);
  lcd.print("|");
  lcd.setCursor(11,3);
  lcd.print("|");
  lcd.setCursor(11,4);
  lcd.print("|");
  lcd.setCursor(12,2);
  lcd.print("--");
  lcd.setCursor(13,2);
  lcd.print("--");
  lcd.setCursor(14,2);
  lcd.print("--");
  lcd.setCursor(15,2);
  lcd.print("--");
  lcd.setCursor(16,2);
  lcd.print("--");
   lcd.setCursor(17,2);
  lcd.print("--");
  lcd.setCursor(18,2);
  lcd.print("--");
  lcd.setCursor(19,2);
  lcd.print("--");

  lcd.setCursor(17,0);
  lcd.print("OFF");

  lcd.setCursor(15,4);
  lcd.print("-");
    
}
void Time12_24(){
  modeflag = !modeflag;
}

void DisplayClock_Mode(int hour,int min){
if(setted){                                   //saat ayarlandıktan sonra devreye girer
    if(modeflag && count==0)                  
    {
          lcd.setCursor(7, 1);
          lcd.print(" "); 
          lcd.setCursor(2,1);
          lcd.print(" "); 
          
          if(hour>11)
          {
          lcd.setCursor(8, 1);
          lcd.print("PM"); 
          }
          else
          {
          lcd.setCursor(8, 1);
          lcd.print("AM"); 
          }
          lcd.setCursor(3,1);
          if(hour<12)   lcd.print(hour);
          else  lcd.print(hour-12);
          
          lcd.print(':');
          //lcd.setCursor(5,1);
          if (min<10) lcd.print("0");
          lcd.print(min);
    }

    else
    {   
        lcd.setCursor(2,1);
        if(hour<10) lcd.print("0");
        lcd.print(hour);
        lcd.print(':');
        if(min<10) lcd.print("0");
        lcd.print(min);
        lcd.print(':');
        if(sec<10) lcd.print("0");
        lcd.print(sec);

    }
}  
     
}
//------------------------------------Clock Set Up---------------------------------------------------------
void ClockSetUp(){
  if(digitalRead(b1))
  {
    if (buttonActive == false) 
    {
      buttonActive = true;
      if(clockFlag && flag2){count++;Serial.print(count);Serial.print(" ");if(count==2){flag2=false;tF=true;count=0;setted=true;Serial.println();}}  //timer başlatmak için
      buttonTimer = millis();
    }
    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false))
    {
      tF=false;                                       //burada tf saati kurarken derecede modun değişmesini engelliyor.
      longPressActive = true;                                                 //Eğer 3sn den fazla basılmışsa ayarlama modu önce flag1 true yapar yani dakika ayarlama aktif olur
      lcd.clear();                                                            //tekrar basılıncada flag1 false flag 2 true olur yani count==1 kısmında ve saat ayarlanır tekrar basılıncada
      setScreen();                                                            //count==2 olmuş olur set flag true olur ve timer başlatılır . Aynı mantık alarm içinde geçerlidir.
      clockFlag=true;
      flag1=true;
      Serial.println("clock set active");
      start_modeButton++;
      Serial.print(count);Serial.print(" ");
    }
  } else {
    if (buttonActive == true) {
      if (longPressActive == true) {
          longPressActive = false;
          count++;
         Serial.print(count);Serial.print(" ");
      } else {
        Serial.print(count);Serial.print(" ");
        if(count==1){
          flag1=false;
          flag2=true;
        }   
      }
      buttonActive = false;
    }
  }
}
//--------------------------------Alarm Set Up--------------------------------------
void AlarmSetUp(){
  if(digitalRead(b2))
  {
    if (buttonActive1 == false) 
    {
      buttonActive1 = true;
      if(AlarmFlag && flag4){counta++;Serial.print(counta);Serial.print(" ");if(counta==2){flag4=false;tF=true; ready_a = true;digitalWrite(b3,LOW); lcd.setCursor(17,0);
      lcd.print(" ON");lcd.setCursor(12,0);lcd.print("ALARM");counta=0;Serial.println();}}  
      buttonTimer1 = millis();
    }
    if ((millis() - buttonTimer1 > longPressTime1) && (longPressActive1 == false))
    {
      a_again++;
      tF=false;                                         //burada tf alarm'ı kurarken derecede modun değişmesini engelliyor.
      longPressActive1 = true;
      AlarmFlag=true;
      flag3=true;
      Serial.println("Alarm set active");
      Serial.print(counta);Serial.print(" ");
    }
  } else {
    if (buttonActive1 == true) {
      if (longPressActive1 == true) {
          longPressActive1 = false;
          counta++;
          Serial.print(count);Serial.print(" ");
      } else {
        Serial.print(count);//Serial.print(" ");
        if(counta==1){
          flag3=false;
          flag4=true;
        }   
      }
      buttonActive1 = false;
    }
  }
}
//---------------------tone-----------------------

void AlarmStop(){
    toneFlag = !toneFlag;
    starttime = millis()/1000;
}

void Alarm(){
  
      if(!toneFlag){
      tone(buzzer,30);
      delay(30);
      noTone(buzzer);
      delay(30);
      tone(buzzer,30);
      delay(50);
      
    }

     else{
        //Serial.println("Alarm stop");
        activetime = (millis() / 1000) - starttime;
        if(activetime - prevoustime <= 5 ){
          noTone(buzzer);
          a_min+=5;
          prevoustime = activetime;
        }
        else{ 
          Serial.println(activetime);
          toneFlag = !toneFlag;
        }
     } 
 
}

//----------------------------------------------------------------------------------

ISR(TIMER1_COMPA_vect){
  if(setted){                   //saatin ayarlanması tamamlandıktan sonra devreye girer
  sec++;
  if(sec>=60){
   min++;
   sec=0;   
  }
  if(min>=60){
   hour++;
   min=0;
  }
  if(hour>=24){
   hour=0; 
  }
  }
 
}


void Temperature(){
  temperatureFlag = ~temperatureFlag;
}

void TC74(){

  Wire.beginTransmission(address);
  Wire.write(degree);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);


  double celcius = Wire.read();
  double fahrenheit = celcius * 9.0/5.0 + 32.0;
  
  if(temperatureFlag){
    lcd.setCursor(13,3);
    Serial.println(fahrenheit);
    lcd.print(String(fahrenheit) + " F");
    
    }
  else{
  lcd.setCursor(13,3);
  Serial.println(celcius);
  lcd.print(String(celcius) + " C");
 
  }
  //delay(500);
}
