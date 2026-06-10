#include <RTClib.h>
#include <LedControl.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>

//note utilisé par le buzzer
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_B4  494
#define NOTE_C5  523

//PIN
#define buzzer 8
#define pinArduinoRaccordementSignalSW  5
#define pinArduinoRaccordementSignalCLK 3
#define pinArduinoRaccordementSignalDT  4
#define PIN_TRIG 7
#define PIN_ECHO 6

//dimension écran
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//informations EEPROM
const byte LongueurSon[2] = {32,33};
const byte AdresseNotes[2] = {0,AdresseNotes[0]+sizeof(int)*LongueurSon[0]+1+sizeof(byte)*LongueurSon[0]+1};
const byte AdresseTemps[2] = {AdresseNotes[0]+sizeof(int)*LongueurSon[0]+1,AdresseNotes[1]+sizeof(int)*LongueurSon[0]+1};

//initialisation de différent truc
LedControl matriceled = LedControl(11, 13, 10, 4);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//variables
byte menu=0;
int etatPrecedentLigneCLK = digitalRead(pinArduinoRaccordementSignalCLK);
int etatPrecedentLigneDT  = digitalRead(pinArduinoRaccordementSignalDT);
int compteur = 6;
byte intensite=4;
int secondeMinuteur=0;
byte minuteurActiv=0;
int stageTime[3]={25,5,15};
int time=stageTime[0]*60;
String stageName[3]={"Pomodoro","Short break","Long break"};
byte activite=0;
boolean pause=true;
byte stage=0;
unsigned long lastSecond= millis();

//dessin des chiffres dans la matrices led
byte digits[10][8] = {
  {0b00111100, 0b01100110, 0b01101110, 0b01110110, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  {0b00011000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b01111110, 0b00000000},
  {0b00111100, 0b01100110, 0b00000110, 0b00001100, 0b00110000, 0b01100000, 0b01111110, 0b00000000},
  {0b00111100, 0b01100110, 0b00000110, 0b00011100, 0b00000110, 0b01100110, 0b00111100, 0b00000000},
  {0b00001100, 0b00011100, 0b00101100, 0b01001100, 0b01111110, 0b00001100, 0b00011110, 0b00000000},
  {0b01111110, 0b01100000, 0b01111100, 0b00000110, 0b00000110, 0b01100110, 0b00111100, 0b00000000},
  {0b00111100, 0b01100110, 0b01100000, 0b01111100, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  {0b01111110, 0b01100110, 0b00000110, 0b00001100, 0b00011000, 0b00011000, 0b00011000, 0b00000000},
  {0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  {0b00111100, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b01100110, 0b00111100, 0b00000000} 
};


void changementSurLigneCLK() {
  int etatActuelDeLaLigneCLK = digitalRead(pinArduinoRaccordementSignalCLK);
  int etatActuelDeLaLigneDT  = digitalRead(pinArduinoRaccordementSignalDT);

  if((etatActuelDeLaLigneCLK == HIGH) && (etatActuelDeLaLigneDT == LOW) && (etatPrecedentLigneCLK == LOW) && (etatPrecedentLigneDT == HIGH)) compteur++;
  if((etatActuelDeLaLigneCLK == HIGH) && (etatActuelDeLaLigneDT == HIGH) && (etatPrecedentLigneCLK == LOW) && (etatPrecedentLigneDT == LOW)) compteur--;

  etatPrecedentLigneCLK = etatActuelDeLaLigneCLK;
  etatPrecedentLigneDT = etatActuelDeLaLigneDT;
}

/*void initialisationDesSonsDansEEPROM() {
  //son 1
  int notes1[LongueurSon[0]] = {
    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5,
    NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4,
    NOTE_E4, NOTE_G4, NOTE_B4, NOTE_C5,
    NOTE_C5, NOTE_B4, NOTE_G4, NOTE_C4,
    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5,
    NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4,
    NOTE_E4, NOTE_G4, NOTE_B4, NOTE_C5,
    NOTE_C5, NOTE_B4, NOTE_G4, NOTE_C4
  };

  byte temps1[LongueurSon[0]] = {
    30, 30, 30, 60,
    30, 30, 30, 60,
    30, 30, 30, 60,
    30, 30, 30, 60,    
    30, 30, 30, 60,
    30, 30, 30, 60,
    30, 30, 30, 60,
    30, 30, 30, 100
  };

  //son 2
  int notes2[LongueurSon[0]] = {
    NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_E4, NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_B3,
    NOTE_D4, NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_D4,
    NOTE_B3, NOTE_CS4, NOTE_D4, NOTE_E4, NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_E4,
    NOTE_FS4, NOTE_E4, NOTE_D4, NOTE_B3, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_CS4, NOTE_D4
  };

  byte temps2[LongueurSon[0]] = {
    20, 20, 20, 20, 20, 20, 20, 40,
    20, 20, 20, 20, 20, 20, 20, 40,
    20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 40, 20, 20, 20, 20, 60
  };

  //ecriture dans l'EEPROM
  for (int i = 0; i < LongueurSon[0]; i++) {
    EEPROM.put(AdresseNotes[0] + i*sizeof(int), notes1[i]);
    EEPROM.put(AdresseTemps[0] + i*sizeof(byte), temps1[i]);
  }
  for (int i = 0; i < LongueurSon[0]; i++) {
    EEPROM.put(AdresseNotes[1] + i*sizeof(int), notes2[i]);
    EEPROM.put(AdresseTemps[1] + i*sizeof(byte), temps2[i]);
  }
}*/

void setup() {
  Serial.begin(115200);

  //setup matrices led
  for (int i = 0; i < 4; i++) {
    matriceled.shutdown(i, false);
    matriceled.setIntensity(i, intensite);
    matriceled.clearDisplay(i);
  }

  //setup écran Oled
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println(F("SSD1306 allocation failed"));
  for(;;);
  }
  

  //setup buzzer
  pinMode(buzzer,OUTPUT);

  //setup encodeur numérique
  pinMode(pinArduinoRaccordementSignalSW, INPUT_PULLUP);
  pinMode(pinArduinoRaccordementSignalCLK, INPUT);
  pinMode(pinArduinoRaccordementSignalDT, INPUT);
  etatPrecedentLigneCLK = digitalRead(pinArduinoRaccordementSignalCLK);
  etatPrecedentLigneDT  = digitalRead(pinArduinoRaccordementSignalDT);
  attachInterrupt(digitalPinToInterrupt(pinArduinoRaccordementSignalCLK), changementSurLigneCLK, CHANGE);   // CHANGE => détecte tout changement d'état


  //initialisationDesSonsDansEEPROM();
  
  delay(2000);
}

void affichage_chiffre(byte matrice, byte chiffre) {
  for (byte i = 0; i < 8; i++) {
    matriceled.setRow(matrice, i, digits[chiffre][i]); 
  }
}

/*void lancementMinuteur(){
  byte minutes=0,heures=0,secondes=0;
  
  //initilaisation écran pour l'heure
  display.setTextSize(2);
  display.setTextColor(WHITE);

  //réglage des heures
  delay(200);
  compteur=0;
  while(digitalRead(pinArduinoRaccordementSignalSW) == HIGH){
    if(compteur<0)compteur=23;
    if(compteur>23)compteur=0;
    heures=compteur;
    //initialisation écran
    display.clearDisplay();
    //affichage Titre de section
    display.setCursor(5, 5 );
    display.print("MINUTEUR");
    //affichage de l'heure
    display.setCursor(5, 30);
    display.print(heures/10);
    display.print(heures%10);
    display.print(":");
    display.print(minutes/10);
    display.print(minutes%10);
    display.print(":");
    display.print(secondes/10);
    display.print(secondes%10);
    //affichage curseur
    display.fillRect(5,55,25,4,WHITE);
    //actualisation écran
    display.display();
  }
  
  //réglage des minutes
  delay(200);
  compteur=0;
  while(digitalRead(pinArduinoRaccordementSignalSW) == HIGH){
    if(compteur<0)compteur=59;
    if(compteur>59)compteur=0;
    minutes=compteur;
    //initialisation écran
    display.clearDisplay();
    //affichage Titre de section
    display.setCursor(5, 5 );
    display.print("MINUTEUR");
    //affichage de l'heure
    display.setCursor(5, 30);
    display.print(heures/10);
    display.print(heures%10);
    display.print(":");
    display.print(minutes/10);
    display.print(minutes%10);
    display.print(":");
    display.print(secondes/10);
    display.print(secondes%10);
    //affichage curseur
    display.fillRect(40,55,25,4,WHITE);
    //actualisation écran
    display.display();
  }

  //réglage des secondes
  delay(200);
  compteur=0;
  while(digitalRead(pinArduinoRaccordementSignalSW) == HIGH){
    if(compteur<0)compteur=59;
    if(compteur>59)compteur=0;
    secondes=compteur;
    //initialisation écran
    display.clearDisplay();
    //affichage Titre de section
    display.setCursor(5, 5 );
    display.print("MINUTEUR");
    //affichage de l'heure
    display.setCursor(5, 30);
    display.print(heures/10);
    display.print(heures%10);
    display.print(":");
    display.print(minutes/10);
    display.print(minutes%10);
    display.print(":");
    display.print(secondes/10);
    display.print(secondes%10);
    //affichage curseur
    display.fillRect(75,55,25,4,WHITE);
    //actualisation écran
    display.display();
  }
  compteur=abs(menu-6);
}
*/

void affichageTime(){
  boolean points;
  if(millis()-lastSecond>=1000 && !pause){
    time--;
    points=!points;
    lastSecond=millis();
  }
  int minutes=(time/60);
  int secondes=(time%60);
  //affichage des minutes
  affichage_chiffre(3, minutes/10);
  affichage_chiffre(2, minutes%10);
  //affichage des secondes
  affichage_chiffre(1,secondes/10);
  affichage_chiffre(0,secondes%10);
  //affichage des deux points
  if(points){
    matriceled.setLed(1, 2, 0, true);
    matriceled.setLed(1, 5, 0, true);
    matriceled.setLed(2, 2, 7, false);
    matriceled.setLed(2, 5, 7, false);
  }
  else{
    matriceled.setLed(2, 2, 7, true);
    matriceled.setLed(2, 5, 7, true);
    matriceled.setLed(1, 2, 0, false);
    matriceled.setLed(1, 5, 0, false);
  }
}

void fctMenu(){

  //initialisation écran
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  if(compteur<0)compteur=6;
  if(compteur>6)compteur=0;
  menu=abs(compteur-6);
  
  //affichage indication de réglages
  display.setCursor(5,0);
  display.print(stageName[stage]);

  //affichage menu
  //page 1
  if(menu<5){
    display.setCursor(5, 10);
    if(pause)display.print("play"); else display.print("stop");
    display.setCursor(5, 20);
    display.print("period");
    display.setCursor(5, 30);
    display.print("pomodoro ");
    display.print(stageTime[0]);
    display.setCursor(5, 40);
    display.print("short break ");
    display.print(stageTime[1]);
    display.setCursor(5, 50);
    display.print("long break ");
    display.print(stageTime[2]);
  }
  //page 2
  else{
    display.setCursor(5, 10);
    display.print("intensite");
    display.setCursor(5, 20);
    display.print("minuteur");
  }

  //affichage curseur
  display.fillRect(0,10+((menu%5)*10),3,10,WHITE);
  display.display();

  //interaction menu
  if(digitalRead(pinArduinoRaccordementSignalSW) == LOW){
    switch(menu){
      case 0:
        pause=!pause;
        break;
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
      case 4:
        break ;
      case 5:
        break ;
      case 6:
        break ;
    }
    delay(300);
  }
}

/*void sonnerie(){
  //réveil
  if(0){
    for (byte j = 0; j < LongueurSon[0]; j++) {
      int notes;
      byte temps;
      EEPROM.get(AdresseNotes[1] + j*sizeof(int),notes);
      EEPROM.get(AdresseTemps[1] + j*sizeof(byte),temps);
      tone(buzzer, notes);
      delay(temps * 10);
      noTone(buzzer);
      if(digitalRead(pinArduinoRaccordementSignalSW) == LOW) j=LongueurSon[0];
      digitalWrite(PIN_TRIG, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_TRIG, LOW);
      int duration = pulseIn(PIN_ECHO, HIGH);
    }
  }
  //fin minuteur
  int tempsActu=1;
  if(tempsActu>=secondeMinuteur && minuteurActiv){
    minuteurActiv=0;
    tone(buzzer, 250);
    delay(500);
    noTone(buzzer);
    delay(250);
    tone(buzzer, 300);
    delay(500);
    noTone(buzzer);
  }
}
*/

void loop() {
  affichageTime();
  if(time<=0 && stage<2){
    stage++;
    time=stageTime[stage]*60;
    //sonnerie()
  }
  fctMenu();
}