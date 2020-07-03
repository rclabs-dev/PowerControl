#include <FloatToString.h>
#include <LiquidCrystal.h>
#include <Wire.h> 
#include <RtcDS3231.h> 

// RTC library by Makuna

RtcDS3231<TwoWire> rtc(Wire); 

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 23, en = 25, d4 = 30, d5 = 31, d6 = 32, d7 = 33;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

String version = "1.0";

// Posição da chave manual ou automático
const int ctrlManualAuto = 36;
  
// Há AC da concessionária
const int inputAc = 37;

// Há AC do Gerador
const int inputGen = 38; 

// Aciona/libera Afogador
const int startChokeCtrl = 39;

// Liga desliga o gerador
const int startEngine = 40;

// Ativa modo nobreak
const int switchToNobreak = 41;

// Ativa modo energia comercial
const int switchToAc = 42;

// Buzzy
const int buzzer = 43;

// Tempo de comutacao para o gerador em milisegundos
long timeStartGen = 5000;

// Temp de comutacao para a energia AC em milisegundo
long timeStartAc = 5000;

// Estado inicial da presença de energia ac e do gerador
String acState = "Off";
String genState = "On";

// Buffer para lib de conversão flot para string
char floatBuffer[10];

// Flag para saida de loop
boolean flagOut = false;

// Contador para uso geral
int count;

void setup() {
  // Define o comportadamento das GPIOs
  pinMode(ctrlManualAuto, INPUT_PULLUP);
  pinMode(inputAc, INPUT_PULLUP); 
  pinMode(inputGen, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(startChokeCtrl, OUTPUT);
  pinMode(startEngine, OUTPUT);
  pinMode(switchToNobreak, OUTPUT);
  pinMode(switchToAc, OUTPUT);

  // Define estado inical das GPIOs
  digitalWrite(startChokeCtrl, LOW);
  digitalWrite(startEngine, LOW);
  digitalWrite(switchToNobreak, LOW);
  digitalWrite(switchToAc, LOW);

  beepBuzzer("ok");
  
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 2);
  // Print a message to the LCD.
  lcd.print(" Power Line Switch");
  lcd.setCursor(0, 1);
  lcd.print("    Version " + version);
  delay(1000);
  lcd.clear();

  // Inicializa Real Time Clock
  rtc.Begin();

  // Obtém a data da compilação
  RtcDateTime data_compilacao = RtcDateTime(__DATE__, __TIME__);

  if (!rtc.IsDateTimeValid()) {
    printLCD("ATENCAO! Verifique", "a bateria do relogio");
    beepBuzzer("wrong");
    delay(1000);
    //rtc.SetDateTime(data_compilacao);
  }
}
  
void loop() {
  RtcDateTime now = rtc.GetDateTime();
  RtcTemperature temp = rtc.GetTemperature();
  checkOutputs();  
  if (digitalRead(ctrlManualAuto)){ // Em automático             
       printLCD(formatDate(now,"dmy") + " " + formatTime(now,"h:m:s") + " " + floatToString(floatBuffer, temp.AsFloatDegC(),1), "AUTO AC:" + acState + " GER:" + genState + " ");
       count = timeStartGen / 1000;
       if (!digitalRead(inputAc)) {
       while (!digitalRead(inputAc)) {
          printLCD("SEM ENERGIA, LIGANDO", "O NOBREAK EM  " + String(count) + "s");
          delay(1000);
          count = count - 1;
          if (count == -1) {
             while (!digitalRead(inputAc)) {
                count = 1;
                printLCD("LIGANDO O GERADOR,", "TENTATIVA #" + String(count) + "s");
             }
          }
       }
       printLCD("A ENERGIA AC VOLTOU!", "");
       delay(3000);
       }
  } else { // Em manual
     printLCD(formatDate(now,"dmy") + " " +  formatTime(now,"h:m:s") + " " + floatToString(floatBuffer, temp.AsFloatDegC(),1), "MAN AC:" + acState + " GER:" + genState);
  }
}

// Funções para formatação dos dados
String formatDate(const RtcDateTime& dt, String format) 
{
  String d = dt.Day() < 10 ? "0" + String(dt.Day()) : String(dt.Day()) ; 
  String m = dt.Month() < 10 ? "0" + String(dt.Month()) : String(dt.Month()) ;
  String y = String(dt.Year()).substring(2) ;
  format.replace("d",d);
  format.replace("m",m);
  format.replace("y",y);
  return format;
}
 
String formatTime(const RtcDateTime& dt, String format) {
  String h = dt.Hour() < 10 ? "0" + String(dt.Hour()) : String(dt.Hour()) ;
  String m = dt.Minute() < 10 ? "0" + String(dt.Minute()) : String(dt.Minute()) ;
  String s = dt.Second() < 10 ? "0" + String(dt.Second()) : String(dt.Second()) ;
  format.replace("h",h);
  format.replace("m",m);
  format.replace("s",s);
  return format;
}

void checkOutputs() {
  if (digitalRead(inputAc)) {
    acState = "On";
  } else {
    acState = "Off";
  }
  
  if (digitalRead(inputGen)) {
    genState = "On";
  } else {
    genState = "Off";
  }
}

// Escreve nos LCDs
void printLCD(String l1, String l2){ 
  lcd.clear();
  lcd.print(l1);
  lcd.setCursor(0, 1);
  lcd.print(l2);
  delay(500);
}

// Toca buzzer
void beepBuzzer(String tone) {
  if (tone == "ok") {
     digitalWrite(buzzer, LOW);
     delay(100);
     digitalWrite(buzzer, HIGH);
     delay(100);
     digitalWrite(buzzer, LOW);
     delay(100);
     digitalWrite(buzzer, HIGH);
  }
  if (tone == "wrong") {
     digitalWrite(buzzer, LOW);
     delay(500);
     digitalWrite(buzzer, HIGH);
     delay(100);
     digitalWrite(buzzer, LOW);
     delay(500);
     digitalWrite(buzzer, HIGH);
     delay(100);
     digitalWrite(buzzer, LOW);
     delay(500);
     digitalWrite(buzzer, HIGH);

  } 
}
