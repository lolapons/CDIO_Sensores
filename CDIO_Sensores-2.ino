#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>

// Comentar/Descomentar para ver mensajes de depuracion en monitor serie y/o respuesta del HTTP server
#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP_RESPONSE

// Comentar/Descomentar para conexion Fuera/Dentro de UPV
//#define WiFi_CONNECTION_UPV

// Selecciona que servidor REST quieres utilizar entre ThingSpeak y Dweet
// cambio enlace por mio https://thingspeak.com/channels/360979
#define REST_SERVER_THINGSPEAK //Selecciona tu canal para ver los datos en la web (https://thingspeak.com/channels/1629093)
//#define REST_SERVER_DWEET //Selecciona tu canal para ver los datos en la web (http://dweet.io/follow/PruebaGTI)

///////////////////////////////////////////////////////
/////////////// WiFi Definitions /////////////////////
//////////////////////////////////////////////////////

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "RU-Gandia";
  const char WiFiPSK[] = "104@Gandia-";
#endif



///////////////////////////////////////////////////////
/////////////// SERVER Definitions /////////////////////
//////////////////////////////////////////////////////

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#else
  const char Server_Host[] = "dweet.io";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

///////////////////////////////////////////////////////
/////////////// HTTP REST Connection ////////////////
//////////////////////////////////////////////////////

#ifdef REST_SERVER_THINGSPEAK 
  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="L7744G5QW9RREPR1"; // Escribe la clave de tu canal ThingSpeak
#else 
  const char Rest_Host[] = "dweet.io";
  String MyWriteAPIKey="cdiocurso2021g05"; // Escribe la clave de tu canal Dweet
#endif

#define NUM_FIELDS_TO_SEND 4 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

/////////////////////////////////////////////////////
/////////////// Pin Definitions ////////////////
//////////////////////////////////////////////////////

const int LED_PIN = 5; // Thing's onboard, green LED

/////////////////////////////////////////////////////
/////////////// WiFi Connection ////////////////
//////////////////////////////////////////////////////

void connectWiFi()
{
  byte ledStatus = LOW;

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

/////////////////////////////////////////////////////
/////////////// HTTP POST  ThingSpeak////////////////
//////////////////////////////////////////////////////

void HTTPPost(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak mediante el metodo HTTP POST
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar numFields al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
       
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
   
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}

////////////////////////////////////////////////////
/////////////// HTTP GET  ////////////////
//////////////////////////////////////////////////////

void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }


          
           
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}

//    El circuito de humedad se conecta al puerto 1 (adc1)
//   El circuito de salinidad se conecta al puerto 3 (adc3)
//  El circuito de temperatura se conecta al puerto 0 (adc0)
// El circuito de Luz se conecta al puerto 2 (adc2)

Adafruit_ADS1115 ads1115; // construct an ads1115 at address 0x48

//variables Humedad
const int AirValue  = 30600;  // Medimos valor en seco
const int WaterValue = 16000;  // Medimos valor en agua
// variables Salinidad
const int SaltValue =22000  ; // 26072
const int WithoutSaltValue = 18000;  // 20595
int power_pin = 5; // GPIO 5 se utiliza para alimentar la sonda de salinidad
// variables de Luz
int power_pinL = 0; // GPIO 0 se utiliza para alimentar la garrapata 
bool hayLuz = true;
//variables deep sleep
const int sleepTimeS = 30;
int cont=0;

void setup() {

  Serial.begin(9600);
  Serial.println("Inicializando...");
  ads1115.begin(); //Initialize ads1115
  Serial.println("Ajustando la ganancia...");
  ads1115.setGain(GAIN_ONE);
  pinMode(power_pin, OUTPUT);
  pinMode(power_pinL, OUTPUT);

  Serial.println("Tomando medidas de los AIN1, AIN3 y AIN0");
  Serial.println("Rango del ADC: +/- 4.096V (1 bit=2mV)"); 

  //////////////////////////////////////////////
  //////////////////////////////////////////////

  Serial.println("ESP8266 in normal mode");
   Serial.println("Conectar el cable");
  delay(10000);
  Serial.println("Entrando en el loop");

  
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif
 }

//-----------------------------------------------------------
//-----------------------------------------------------------


void loop() {
  int16_t adc1;
  int16_t humedad;
  int16_t adc3;
  int16_t salinidad;
  int16_t adc0;
  double temperatura;
  int16_t adc2;
  int16_t Luz;
  //--------------------------------------------  
  // HUMEDAD
  //---------------------------------------------
  adc1 = ads1115.readADC_SingleEnded(1);
  humedad = 100*AirValue/(AirValue-WaterValue)-adc1*100/(AirValue-WaterValue);
  //--------------------------------------------  
  // SALINIDAD
  //---------------------------------------------
  digitalWrite( power_pin, HIGH ); 
  //--->  Esperar 100ms
  delay(100);
  //--->  Muestrear la tensión del sensor de salinidad
  adc3 =ads1115.readADC_SingleEnded(3);                                
  //--->  Poner power_pin a nivel bajo
  digitalWrite( power_pin, LOW );
  //--> salinidad = 100*WithoutSaltValue / (WithoutSaltValue-SaltValue) -adc3*100/(WithoutSaltValue-SaltValue);
  salinidad = 10000*WithoutSaltValue / (WithoutSaltValue-SaltValue) - adc3*10000/(WithoutSaltValue-SaltValue);
  salinidad = salinidad/100;
  if(salinidad<=10){
    salinidad = 0;
  }
  else{
    if(salinidad>=100){
      salinidad = 100;
    }
    else{
      salinidad = salinidad;
    }
  }
  //salinidad = 100*SaltValue/(WithoutSaltValue-SaltValue)-adc3*100/(WithoutSaltValue-SaltValue); //1)
  //salinidad = salinidad/100;
  //--------------------------------------------  
  // TEMPERATURA
  //---------------------------------------------
  adc0 = ads1115.readADC_SingleEnded(0);
  float m = 37*pow(10,-3);
  float b = 0.79;
  double Vo = (adc0 * 4.096 / 32767);
  temperatura = ((Vo - b) /m);

  //------------------------------------------
  // LUZ
  //------------------------------------------
  digitalWrite(power_pinL, HIGH);
  delay(100);
  adc2 = ads1115.readADC_SingleEnded(2);
  Luz = adc2;
 
  digitalWrite(power_pinL,LOW);
  //--------------------------------------------
  // SLEEP
  //-------------------------------------------
  Serial.print("cont = ");
  Serial.println(cont, DEC);
  if (cont<20){
    cont++;
    delay(1000);
  }
  else{
    Serial.println("ESP8266 in sleep mode");
    ESP.deepSleep(sleepTimeS);

  }
  //--------------------------------------
  //  OUTPUTS
  // ---------------------------------------------------------------------
  // ---------------------------------------------------------------------
  delay(3500);
  Serial.println(" ");
  Serial.println("/////////////////////////");
  Serial.println("Nuevas medidas: ");
  // ---------------------------------------------------------------------
  // Escribimos la humedad
  // ---------------------------------------------------------------------
  //Serial.print("AIN1: ");
  //Serial.println(adc1);
  Serial.print("Humedad (%): ");
  Serial.print(humedad);
  Serial.println("%");
  // ---------------------------------------------------------------------
  // Escribimos la salinidad
  // ---------------------------------------------------------------------
  //Serial.print("AIN3: ");
  Serial.println(adc3);
  Serial.print("Salinidad (%): ");
  Serial.print(salinidad);
  Serial.println("%");
  // ---------------------------------------------------------------------
  // Escribimos la temperatura
  // ---------------------------------------------------------------------
  //Serial.println(adc0);
  Serial.print("Temperatura(º): ");
  Serial.print(temperatura);
  Serial.println("º");
  //---------------------------------------------------------------------
  // Escribimos Luz
  //---------------------------------------------------------------------
  //Serial.print("AIN2: ");
  //Serial.println(adc2);
  Serial.print("Estado de Luz: ");
  if(Luz < 120){ // 2600  20000
    Serial.println("Oscuro");
  }
  else if(Luz > 1500){ // -2400  10000
    Serial.println("Soleado");
  }
  else{
    Serial.println("Nublado");
  }
  Serial.println("/////////////////////////");
  //---------------------------------------------------------------------
  //---------------------------------------------------------------------

   
    String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    
    data[ 1 ] = String(humedad); //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Random1 = " );
        Serial.println( data[ 1 ] );
    #endif

    data[ 2 ] = String(salinidad); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Random2 = " );
        Serial.println( data[ 2 ] );
    #endif

  data[ 3 ] = String(temperatura); //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Random1 = " );
        Serial.println( data[ 3 ] );
    #endif

  data[ 4 ] = String(Luz); //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Random1 = " );
        Serial.println( data[ 4 ] );
    #endif


    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    HTTPPost( data, NUM_FIELDS_TO_SEND );
    //HTTPGet( data, NUM_FIELDS_TO_SEND );
  
  
}
