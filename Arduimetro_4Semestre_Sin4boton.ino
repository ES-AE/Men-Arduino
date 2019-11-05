//#include <LiquidCrystal.h>
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#include <Filters.h>//Librería del Sensor ZMPT101B
#include <Wire.h> // " " " "
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
//Variables del sensor ACS712
float Sensibilidadcc = 0.063; // Modelo 30A [A/A]
float Sensibilidadca = 0.055; // Modelo 30A [A/A]
int NumMuestras = 100; //muestras del conversorADC
const int PulsadorPin = 52;
int Pulsador = 0;
//
//Variables del sensor ZMPT101B
int Sensor = 0; //A1
float testFrequency = 50;                     // Frecuencia (Hz)
float windowLength = 40.0 / testFrequency;   // promedio de la señal
float intercept = -0.04; // to be adjusted based on calibration testing
float slope = 0.0405; // to be adjusted based on calibration testing
float volts; // Voltage

unsigned long periodo = 1000;
unsigned long tiempoAnterior = 0;
//
/* Variable para recoger la lectura de los puertos analogicos*/
float lectura;


/* Variables donde se guarda el estado de los botones. Pulsado = 1, sin pulsar =0 */
int estado_boton_1 = 0, estado_boton_2 = 0, estado_boton_3 = 0;

/* Variables para los 3 botones*/
int boton_1 = 2;
int boton_2 = 3;
int boton_3 = 4;

/* Variables para calculas las medidas*/
float voltaje = 0.0;
float corriente = 0.0;
float ohmios = 0.0;

/* Valor real de la resistencia de 22ohm para medir amperaje*/
float R = 19;

float mediaTotal;
int muestra;

/* Texto que se va a deslizar de derecha a izquierda por la pantalla*/
String texto = "Encendiendo arduimetro...";

#define COLUMNAS 16
#define FILAS 2
#define VELOCIDAD 250

void setup() {
  //lcd.begin(COLUMNAS, FILAS);
  lcd.init();
  lcd.backlight();
  pinMode(boton_1, INPUT);
  pinMode(boton_2, INPUT);
  pinMode(boton_3, INPUT);

  /* Mostrar texto en movimiento */

  /* Bucle para ir recorriendo las columnas */
  for (int i = COLUMNAS - 1; i >= 0; i--)
  {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("BIENVENIDO");
    lcd.setCursor(i, 1);

    /* Escribir el mensaje */
    lcd.print(texto);

    delay(VELOCIDAD);
  }

  /* Bucle para hacer desaparecer el texto por la izquierda */
  for (int i = 1; i < texto.length(); i++)
  {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("BIENVENIDO");

    lcd.setCursor(0, 1);

    /* Eliminar letra a letra empezando por la izquierda */
    lcd.print(texto.substring(i));

    delay(VELOCIDAD);
  }

  /* Mostrar el menu principal para seleccionar una opcion*/
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Seleccione boton");
  lcd.setCursor(0, 1);
  lcd.print("1.V 2.A 3.O");
  Serial.begin(9600);

}
//Método ACS712
void MostrarMedicion(String magnitud, float valor, String unidad)
{
  Serial.print(magnitud);
  Serial.print(valor, 3);
  Serial.println(unidad);
}
//

void loop() {
  /* Si se pulsa el boton 1, poner a 1 el estado*/
  if (digitalRead(boton_1) == HIGH)
  {
    lcd.clear();
    estado_boton_1 = 1;
    estado_boton_2 = 0;
    estado_boton_3 = 0;
  }

  /* Si se pulsa el boton 2, poner a 1 el estado*/
  else if (digitalRead(boton_2) == HIGH)
  {
    lcd.clear();
    estado_boton_1 = 0;
    estado_boton_2 = 1;
    estado_boton_3 = 0;
  }

  /* Si se pulsa el boton 3, poner a 1 el estado*/
  else if (digitalRead(boton_3 ) == HIGH)
  {
    lcd.clear();
    estado_boton_1 = 0;
    estado_boton_2 = 0;
    estado_boton_3 = 1;
  }
 
  /* Si se ha pulsado el boton 1 llamar a la funcion que calcula el voltaje AC y mostrarlo*/
  if (estado_boton_1 == 1 && estado_boton_2 == 0 && estado_boton_3 == 0)
  {
     RunningStatistics inputStats;
    inputStats.setWindowSecs(windowLength);
    while (true) {
      Sensor = analogRead(A1);  //Leer pin Analógico
      inputStats.input(Sensor);
      if ((unsigned long)(millis() - tiempoAnterior) >= periodo) {
        volts = intercept + slope * inputStats.sigma(); //offset y amplitud
        volts = volts * (40.3231);                      //calibración
     Serial.print("\tVoltage: ");
        Serial.println(volts);
        lcd.setCursor(0, 0);
        lcd.print("Voltaje RMS:");
        lcd.setCursor(0, 1);
        lcd.print(volts);
        tiempoAnterior = millis();         
    }
   }
  }
  /* Si se ha pulsado el boton 2 llamar a la funcion que calcula el amperaje y mostrarlo en mA y A */
 if (estado_boton_1 == 0 && estado_boton_2 == 1 && estado_boton_3 == 0)
  {
    Pulsador = digitalRead(PulsadorPin);
    delay(200);
    if (Pulsador == LOW)//IF para un pulsador con retención para elegir entre CC y CA
    {
      float voltaje;
      float corrienteSuma = 0;
      for (int i = 0; i < NumMuestras; i++)
      {
        voltaje = analogRead(A0) * 5.0 / 1023.0;
        corrienteSuma += (voltaje - 2.4955) / Sensibilidadcc;
      }
      float corriente = (corrienteSuma / NumMuestras);

      MostrarMedicion("Intensidad: ", corriente, " A ,");
      MostrarMedicion("SalidaSensor: ", voltaje, " V");

      if ( (corriente < 0.100) && (corriente > -0.100) )
      {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Sin lectura");
        lcd.setCursor(6, 1);
        lcd.print("CC");
        delay(1000);
      }
      else
      {
        lcd.clear();
        lcd.setCursor(14, 0);
        lcd.print("CC");

        lcd.setCursor(0, 1);
        lcd.print("I= ");
        lcd.print(corriente, 3);
        lcd.print(" A");
        delay(1000);
      }
    }

    else //Pulsador
    {
      float voltaje;
      float get_corriente();
      float corriente = get_corriente(); //obtenemos la corriente pico
      float corrienteRMS = corriente * 0.707; //Intensidad RMS = Ipico/(2^1/2)
      float potencia = corrienteRMS * 223.2; // P=IV watts

      MostrarMedicion("Intensidad: ", corriente, "A ,");
      MostrarMedicion("Irms: ", corrienteRMS, "A ,");
      MostrarMedicion("Potencia: ", potencia, "W");



      if ( (corriente < 0.100) && (corriente > -0.100) )
      {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Sin lectura");
        lcd.setCursor(6, 1);
        lcd.print("CA");
        delay(1000);
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Irms= ");
        lcd.print(corrienteRMS, 3);
        lcd.print(" A");
        lcd.setCursor(14, 0);
        lcd.print("CA");

        lcd.setCursor(0, 1);
        lcd.print("Prms= ");
        lcd.print(potencia, 3);
        lcd.print(" W");
        delay(1000);
      }
    }
  }

  /* Si se ha pulsado el boton 3 llamar a la funcion que calcula el valor de la resistencia y mostrarlo en mV y V */
  else if (estado_boton_1 == 0 && estado_boton_2 == 0 && estado_boton_3 == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Resistencia:");
    lcd.setCursor(0, 1);
    lectura_ohmios();
  }
}

/*Funcion para la lectura de los voltios
  Recibe la unidad en la que se va a medir y los decimales con los que mostrar el resultado
*/
void lectura_voltios(float unidad, int decimales)
{
  lectura = analogRead(A0);

  /* Si no se esta leyendo nada */
  if (lectura == 0) {
    lcd.setCursor(0, 1);
    lcd.print("0   mV     0.00V");
  }
  //Si se está leyendo
  else {
    voltaje = ((5 * float(lectura)) / 1024);
    lcd.print(float(voltaje) * 3 * unidad, decimales);
  }
  delay(200);
}

/* Funcion para la lectura de los amperios
   Recibe la unidad en la que se va a medir y los decimales con los que mostrar el resultado
*/
void lectura_amperios(float unidad, int decimales)
{
  lectura = analogRead(A3);
  for (int i = 1; i <= 20; i++) {
    corriente = (float) lectura * (5 / 1023.0) / R + corriente;
  }
  corriente = (float) corriente / 20;
  lcd.print(corriente * unidad, decimales);
  delay(200);
}

/* Funcion para la lectura de los ohmios */
void lectura_ohmios()
{
  lectura = analogRead(A5);

  /* Si no se esta leyendo nada */
  if (lectura == 0) {
    lcd.print("0           Ohms ");
  }
  ohmios = float(lectura * 5 / 1024);
  muestra = float(100000 * (5 - ohmios) / ohmios);
  mediaTotal = muestra;

  /* Si la medida esta por debajo de 1000 la muestra en Ohms */
  if (mediaTotal < 1000) {
    lcd.print(mediaTotal * 1, 0);
    lcd.setCursor(12, 1);
    lcd.print("Ohms");
  }
  /* Si la medida esta por encima de 1000 la muestra en KOhms */
  else {
    lcd.print(mediaTotal * 0.001, 2);
    lcd.setCursor(11, 1);
    lcd.print("KOhms");
  }
  delay(200);
}
//Método ACS712
float get_corriente()

{
  float voltaje;
  float corriente = 0;
  long tiempo = millis();
  float Imax = 0;
  float Imin = 0;
  while (millis() - tiempo < 500) //realizamos mediciones durante 0.5 segundos
  {
    voltaje = analogRead(A0) * (5.0 / 1023.0);//lectura del sensor
    corriente = 0.9 * corriente + 0.1 * ((voltaje - 2.4955) / Sensibilidadca); //Ecuación  para obtener la corriente
    if (corriente > Imax)Imax = corriente;
    if (corriente < Imin)Imin = corriente;
  }
  return ((Imax - Imin) / 2);
}
//
void VoltiosAC(){
 
}
