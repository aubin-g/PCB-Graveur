// Ports du 74HC595
const int dataPin = 11;   // DS
const int latchPin = 12;  // ST_CP
const int clockPin = 10;  // SH_CP

// Stepper Moteur
#include <Stepper.h>

const int stepPin = 2;// Déclaration des broches de contrôle du moteur
const int dirPin = 3;

// Déclaration de la résolution du moteur
const int stepsPerRevolution = 200;

// Création de l'objet Stepper
Stepper myStepper(stepsPerRevolution, stepPin, dirPin);

const int digitPins[4] = {4, 3, 2, 5};  // Pins des cathodes des digits

// Définition des boutons
const int button0 = 6;   // Bouton pour 0 degrés\0 mm 
const int button36 = 7;  // Bouton pour 36 degrés\0.6 mm
const int button180 = 8; // Bouton pour 180 degrés\3 mm
const int button_1 = A0; // Bouton pour -1 degrés
const int button1 = A1; // Bouton pour +1 degrés

const byte segmentDigits[12] =  {
  0b01011111, // 0
  0b01000010, // 1
  0b10011011, // 2
  0b11010011, // 3
  0b11000110, // 4
  0b11010101, // 5
  0b11011101, // 6
  0b01000111, // 7
  0b11011111, // 8
  0b11010111, // 9
  0b10000000, // - (moins)
  0b00000000  // rien
};

const byte decimalPointMask = 0b00100000;

void shiftOutData(byte data) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}

void displayDigit(int digit, int value, bool decimalPoint = false) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], HIGH);
  }

  byte data = segmentDigits[value];
  if (decimalPoint) {
    data |= decimalPointMask; 
  }

  shiftOutData(data);
  digitalWrite(digitPins[digit], LOW);
  delay(5);
}

//Afficher une float à un entier et deux décimales
void displayNumber(float number) {
  int integer = (int)number; 
  int firstDecimal = (int)((number - integer) * 10);
  int secondDecimal = round((number * 100) - (integer * 100 + firstDecimal * 10));
  if (secondDecimal >= 10){
    secondDecimal = 0;
    firstDecimal++;
  }
  if (firstDecimal >= 10){
    firstDecimal = 0;
    integer++;
  }


  if (round(number * 100) == 0) {
    displayDigit(0, 11, false);  
    displayDigit(1, 0, true);   
    displayDigit(2, 0, false);   
    displayDigit(3, 0, false);   
  } else if (number < 10) {
    displayDigit(0, 10, false); // Affiche un moins
    displayDigit(1, integer, true);  // Affiche la partie entière avec le point décimal
    displayDigit(2, firstDecimal, false); // Affiche la première décimale
    displayDigit(3, secondDecimal, false); // Affiche la seconde décimale
  }
}

void moveServo(int _angle, int newAngle) {
  for (int position = _angle; position <= newAngle; position ++){ // on crée une variable position qui prend des valeurs entre 0 à 180 degrés
    monServo.write(position);  // le bras du servomoteur prend la position de la variable position
    float elevation = (_angle / 360.0) * 6.0;
    displayNumber(elevation);
    delay(40);
  }

  for (int position = _angle; position >= newAngle; position --){ // cette fois la variable position passe de 180 à 0°
    monServo.write(position);  // le bras du servomoteur prend la position de la variable position
    float elevation = (_angle / 360.0) * 6.0;
    displayNumber(elevation);
    delay(40);
  }
  
  
}

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }

  // Déclaration de la vitesse du moteur
  myStepper.setSpeed(100); // Vitesse en tours par minute (TPM)

  // Déclaration de la direction du moteur
  pinMode(dirPin, OUTPUT);

  // Configuration des boutons avec résistance de tirage interne activée
  pinMode(button0, INPUT_PULLUP);
  pinMode(button36, INPUT_PULLUP);
  pinMode(button180, INPUT_PULLUP);
  pinMode(button_1, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);

  Serial.begin(9600);
}

int angle = 0;

void loop() {

  if (digitalRead(button0) == LOW) {
    moveServo(angle, 0);
    delay(200);
    angle = 0;
  }
  if (digitalRead(button36) == LOW) {
    moveServo(angle, 36);
    delay(200); 
    angle = 36;
  }
  if (digitalRead(button180) == LOW) {
    moveServo(angle, 180);
    delay(200);
    angle = 180;
  }
  if (digitalRead(button_1) == LOW) {
    if (angle - 1>=0){
      angle = angle - 1;
      monServo.write(angle);
      delay(200);
    }
  }
  if (digitalRead(button1) == LOW) {
    if (angle + 1<=3){
      angle = angle + 1;
      monServo.write(angle);
      delay(200);
    }
  }

  // Calcul et affichage de l'élévation
  float elevation = (angle / 360.0) * 6.0;
  displayNumber(elevation);
}