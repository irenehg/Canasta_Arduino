#include <EEPROM.h>

/////// Biblioteca button

#define PULLUP HIGH
#define PULLDOWN LOW

#define CURRENT 0
#define PREVIOUS 1
#define CHANGED 2

class Button{
  public:
    Button(uint8_t buttonPin, uint8_t buttonMode=PULLDOWN);
    void pullup();
    void pulldown();
    bool isPressed();
    bool wasPressed();
    bool stateChanged();
    bool uniquePress();
  private:
    uint8_t pin;
    uint8_t mode;
    uint8_t state;
};

Button::Button(uint8_t buttonPin, uint8_t buttonMode){
  this->pin=buttonPin;
  pinMode(pin,INPUT);
  buttonMode==PULLDOWN ? pulldown() : pullup();
  state = 0;
  bitWrite(state,CURRENT,!mode);
}

/*
|| Set pin HIGH as default
*/
void Button::pullup(void){
  mode=PULLUP;
  digitalWrite(pin,HIGH);
}

/*
|| Set pin LOW as default
*/

void Button::pulldown(void){
  mode=PULLDOWN;
  //digitalWrite(pin,LOW);
}

/*
|| Return the bitWrite(state,CURRENT, of the switch
*/
bool Button::isPressed(void){
  bitWrite(state,PREVIOUS,bitRead(state,CURRENT));
  if (digitalRead(pin) == mode){
    bitWrite(state,CURRENT,false);
  } else {
    bitWrite(state,CURRENT,true);
  }
  if (bitRead(state,CURRENT) != bitRead(state,PREVIOUS)){
    bitWrite(state,CHANGED,true);
  }else{
    bitWrite(state,CHANGED,false);
  }
  return bitRead(state,CURRENT);
}

/*
|| Return true if the button has been pressed
*/
bool Button::wasPressed(void){
  if (bitRead(state,CURRENT)){
    return true;
  } else {
    return false;
  }
}

/*
|| Return true if state has been changed
*/
bool Button::stateChanged(void){
  return bitRead(state,CHANGED);
}

/*
|| Return true if the button is pressed, and was not pressed before
*/
bool Button::uniquePress(void){
  return (isPressed() && stateChanged());
}

////////////////////////

#define buzzerPin 10
#define Led 9
#define Vib 8

const unsigned long wait_interval = 3000; //Maximo 3 segundos entre vibracion y sensor de movimiento para poder considerarlo canasta acertada

Button b1=Button(11,PULLUP);
boolean prox = false;
boolean vibr = false;
unsigned long tiempo1=0;
unsigned long tiempo2=0;
int contador;       //es el marcador, su valor se almacenar'a en la EEPROM
int temp, num;
int cm = 0;
unsigned long wait_time = 3000000; //pasen menos de 6 segundos

byte pines_display[7]={24,32,40,38,36,26,42};
byte tabla7seg[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};

byte digitos[4]={44,30,28,22};  //unidades, decenas, centenas, unidades de millar


void cancion_reset()
{                                 //se ejecuta al pulsar el boton reset
 
  tone(buzzerPin, 523);  //do
  delay(300);
  tone(buzzerPin, 784);  //sol
  delay(300);
  tone(buzzerPin, 659);  //mi
  delay(300);
  noTone(buzzerPin);
}

void cancion()
{                               //se ejecuta al encestar
  tone(buzzerPin, 392);
  delay(150);
  //noTone(12);
  tone(buzzerPin, 523);
  delay(150);
  //noTone(12);
  tone(buzzerPin, 659);
  delay(150);
  //noTone(12);
  tone(buzzerPin, 784);
  delay(300);
  //delay(50);
  tone(buzzerPin, 659);
  delay(150);
  tone(buzzerPin, 784);
  delay(500);
  noTone(buzzerPin);
}

void setup()
{
  pinMode(buzzerPin, OUTPUT);
  pinMode(Led, OUTPUT);
  pinMode(Vib,INPUT);

  for (byte i = 0; i < 7; i++)
    pinMode(pines_display[i], OUTPUT);

  for (byte j = 0; j < 4; j++)
    pinMode(digitos[j], OUTPUT);

  has_encestado(0);


  contador=EEPROM.read(0);

  //Serial.begin(9600); 
}

void muestra7seg(byte val){
  for(byte i=0; i < 7; i++)
    digitalWrite(pines_display[i],bitRead(tabla7seg[val],i));
   
}


void pantalla(){ 
  
  digitalWrite(digitos[0],HIGH);
  digitalWrite(digitos[1],HIGH);
  digitalWrite(digitos[2],HIGH);
  digitalWrite(digitos[3],HIGH);
  
  num = contador;
  int unidades = num %10;
  num = num/10;
  int decenas= num%10;
  num = num / 10;
  int centenas=num % 10;
  num = num / 10;
  int u_mill=num%10;

  muestra7seg(unidades);
  digitalWrite(digitos[0],LOW);
  digitalWrite(digitos[1],HIGH);
  digitalWrite(digitos[2],HIGH);
  digitalWrite(digitos[3],HIGH);
  delay(1);

  
  muestra7seg(decenas);
  digitalWrite(digitos[0],HIGH);
  digitalWrite(digitos[1],LOW);
  digitalWrite(digitos[2],HIGH);
  digitalWrite(digitos[3],HIGH);
  delay(1);

  muestra7seg(centenas);
  digitalWrite(digitos[0],HIGH);
  digitalWrite(digitos[1],HIGH);
  digitalWrite(digitos[2],LOW);
  digitalWrite(digitos[3],HIGH);

   delay(1);

  muestra7seg(u_mill);
  digitalWrite(digitos[0],HIGH);
  digitalWrite(digitos[1],HIGH);
  digitalWrite(digitos[2],HIGH);
  digitalWrite(digitos[3],LOW);

  delay(1);

  digitalWrite(digitos[0],HIGH);
  digitalWrite(digitos[1],HIGH);
  digitalWrite(digitos[2],HIGH);
  digitalWrite(digitos[3],HIGH);

  delay(2);


}


long leeDistancia(int triggerPin, int ecoPin){
  pinMode(triggerPin, OUTPUT);  // Borra el trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH); // activa el trigger 10 us
  delayMicroseconds(5);
  digitalWrite(triggerPin, LOW);
  
  pinMode(ecoPin, INPUT);
  return pulseIn(ecoPin, HIGH); // lee anchura del pulso en us
}



void has_encestado(int mode) {
    // MODE:
    // 0 = FALLO
    // 1 = ACIERTO

    if (mode == 1) {
        contador=contador+1;
        digitalWrite(Led, HIGH);
        cancion();  
        digitalWrite(Led, LOW);
    }
    
    prox=false;
    vibr=false;
    tiempo1=0;
    tiempo2=0;

    delay(1000);
}

void check_prox(){
  float f;
    f = 0.01723 * leeDistancia(12, 13); 
    cm = int(f);
    //Serial.println(prox);
    if(cm <= 15) {
      prox = true;
      if(tiempo1 == 0){
        tiempo1=micros();
      }
    }
}


void check_vibr(){
  //Serial.println(vibr);
   if(digitalRead(Vib) == 1) {
      vibr = true;
        tiempo2=micros();
      
   }
}



void loop() {  
  
  if( b1.uniquePress() ){
    contador=0;
    cancion_reset();
  }

  check_vibr();
  check_prox();

  
  if(prox==1 && vibr==1){
    if((tiempo1 - tiempo2) < wait_time){  
      has_encestado(1);
    }
    else{
      has_encestado(0); 
    }
  }


  pantalla(); 

  EEPROM.update(0,contador);

  delay(3);

}
