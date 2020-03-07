#define ARDUINOJSON_ENABLE_PROGMEM 0

#include "ArduinoJson.h"    
#include <WiFi.h>
#include "DHT.h"              // DHT22 Sensör Kütüphanesi
#include <Wire.h>
#include <SFE_BMP180.h>       //BMP180 Basınç Sensörü Kütüphansi

const char* ssid     = "ssid buraya";
const char* password = "parola buraya";
String CityID = "750662"; //Beykoz, Turkey
String APIKEY = "api key buraya";

#define RAKIM 25.0
#define LED_PIN 2			  // Dahili LED Pini
#define DHTPIN 33             // DHT22'nin bağlı olduğu pin
#define DHTTYPE DHT22        
DHT dht(DHTPIN, DHTTYPE);    

float temperature = 0;
float humidity = 0;
float pressure = 0;
int weatherID = 0;

SFE_BMP180 bmp180;

WiFiClient client;
char* servername ="api.openweathermap.org";  // Hava durumunu alacağımız sunucu
String result;

int  donguler = 0;
String weatherDescription ="";
String weatherLocation = "";
float Temperature;

void setup() 
{
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600); //Ekranla iletişime geçmek için seri bağlantıyı başlatıyoruz

  initSensor(); //Sensörleri başlatıyoruz

  connectToWifi(); //Wifi ye bağlanıyoruz
  getWeatherData();              //Hava durumunu alıp ekrana gönderiyoruz
  printWeatherIcon(weatherID);
}

void loop() { 
 
 delay(2000); // 2 Saniye bekliyoruz

 if(donguler == 450) //Her 15 dakika da bir hava durumunu alıyoruz (15x60=900/2=450)
 {
  getWeatherData(); //Hava durumunu alıyoruz ve resmini ekrana gönderip döngü sayısını sıfırlıyoruz
  printWeatherIcon(weatherID);
  donguler = 0;   
 }
 
 getTemperature();              // Sıcaklığı sensörden alıp ekrana gönderiyoruz
 sendTemperatureToNextion();    
 
 getHumidity();                 // Nemi sensörden alıp ekrana gönderiyoruz
 sendHumidityToNextion();
 
 getPressure();                 // Basıncı sensörden alıp ekrana gönderiyoruz
 sendPressureToNextion();

 donguler++;					//Dongu sayısınına 1 ekleyip ledi yakıp tekrarlıyoruz
 
 blinkLED();
}

void connectToWifi() //Wifi ye bağlanma kodu
{
  WiFi.enableSTA(true);
  
  delay(2000);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
}

void initSensor() //Sensörleri başlatma kodu
{
  Serial.begin(9600);
  dht.begin();
  if (!bmp180.begin()) { //Eğer basınç sensörü bulunamazsa burda duruyoruz
  Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
  while (1) {}
  }
}

void blinkLED() 
{
  digitalWrite(LED_PIN,HIGH);      //Ledi 0.1 saniyeliğine yakıp söndürüyoruz
  delay(100);                      
  digitalWrite(LED_PIN,LOW);       
}                                  

float getTemperature()
{
  temperature = dht.readTemperature();
}

float getHumidity()
{
  humidity = dht.readHumidity();
}

float getPressure()
{
  pressure = readPressure();
}

float readPressure()
{
  char status;
  double T,P,p0,a;

  status = bmp180.startTemperature();            //Basınç değerini almak için sensördeki sıcaklık sensörünü başlatıyoruz
  if (status != 0)                               
  {
    delay(status);
    status = bmp180.getTemperature(T);            //Basıncı ölçmek için sıcaklık değerini alıyoruz
    if (status != 0)
    { 
      status = bmp180.startPressure(3);           //Basınç değerini almak için basınç sensörünü başlatıyoruz
      if (status != 0)
      {
        delay(status);
        status = bmp180.getPressure(P,T);         //Basınç değerini alıyoruz
        if (status != 0)
        {
          p0 = bmp180.sealevel(P,RAKIM);       
          return p0;
        }
      }
    }
  }
}

void getWeatherData() // Hava durumunu alma kodu
{
  String result ="";
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(servername, httpPort)) {
        return;
    }
      // Sunucuya göndereceğimiz istek için URI yapıyoruz
    String url = "/data/2.5/forecast?id="+CityID+"&units=metric&cnt=1&APPID="+APIKEY;
    
       // Bu kod isteği sunucuya gönderecek
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + servername + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            client.stop();
            return;
        }
    }

    // Sunucunun cevabındaki tüm satırları okuyoruz
    while(client.available()) {
        result = client.readStringUntil('\r');
    }

result.replace('[', ' ');
result.replace(']', ' ');

char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';

StaticJsonBuffer<1024> json_buf;
JsonObject &root = json_buf.parseObject(jsonArray);
if (!root.success())
{
  Serial.println("parseObject() failed");
}

String location = root["city"]["name"];
String temperature = root["list"]["main"]["temp"];
String weather = root["list"]["weather"]["main"];
String description = root["list"]["weather"]["description"];
String idString = root["list"]["weather"]["id"];
String timeS = root["list"]["dt_txt"];

weatherID = idString.toInt();
Serial.print("\nWeatherID: ");
Serial.print(weatherID);
endNextionCommand(); 

}

void showConnectingIcon()
{
  Serial.println();
  String command = "weatherIcon.pic=3";
  Serial.print(command);
  endNextionCommand();
}

void sendHumidityToNextion()
{
  String command = "humidity.txt=\""+String(humidity,1)+"\"";
  Serial.print(command);
  endNextionCommand();
}

void sendTemperatureToNextion()
{
  String command = "temperature.txt=\""+String(temperature,1)+"\"";
  Serial.print(command);
  endNextionCommand();
}

void sendPressureToNextion()
{
  String command = "pressure.txt=\""+String(pressure,1)+"\"";
  Serial.print(command);
  endNextionCommand();
}

void endNextionCommand()
{
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void printWeatherIcon(int id)
{
 switch(id)
 {
  case 800: drawClearWeather(); break;
  case 801: drawFewClouds(); break;
  case 802: drawFewClouds(); break;
  case 803: drawCloud(); break;
  case 804: drawCloud(); break;
  
  case 200: drawThunderstorm(); break;
  case 201: drawThunderstorm(); break;
  case 202: drawThunderstorm(); break;
  case 210: drawThunderstorm(); break;
  case 211: drawThunderstorm(); break;
  case 212: drawThunderstorm(); break;
  case 221: drawThunderstorm(); break;
  case 230: drawThunderstorm(); break;
  case 231: drawThunderstorm(); break;
  case 232: drawThunderstorm(); break;

  case 300: drawLightRain(); break;
  case 301: drawLightRain(); break;
  case 302: drawLightRain(); break;
  case 310: drawLightRain(); break;
  case 311: drawLightRain(); break;
  case 312: drawLightRain(); break;
  case 313: drawLightRain(); break;
  case 314: drawLightRain(); break;
  case 321: drawLightRain(); break;

  case 500: drawLightRainWithSunOrMoon(); break;
  case 501: drawLightRainWithSunOrMoon(); break;
  case 502: drawLightRainWithSunOrMoon(); break;
  case 503: drawLightRainWithSunOrMoon(); break;
  case 504: drawLightRainWithSunOrMoon(); break;
  case 511: drawLightRain(); break;
  case 520: drawModerateRain(); break;
  case 521: drawModerateRain(); break;
  case 522: drawHeavyRain(); break;
  case 531: drawHeavyRain(); break;

  case 600: drawLightSnowfall(); break;
  case 601: drawModerateSnowfall(); break;
  case 602: drawHeavySnowfall(); break;
  case 611: drawLightSnowfall(); break;
  case 612: drawLightSnowfall(); break;
  case 615: drawLightSnowfall(); break;
  case 616: drawLightSnowfall(); break;
  case 620: drawLightSnowfall(); break;
  case 621: drawModerateSnowfall(); break;
  case 622: drawHeavySnowfall(); break;

  case 701: drawFog(); break;
  case 711: drawFog(); break;
  case 721: drawFog(); break;
  case 731: drawFog(); break;
  case 741: drawFog(); break;
  case 751: drawFog(); break;
  case 761: drawFog(); break;
  case 762: drawFog(); break;
  case 771: drawFog(); break;
  case 781: drawFog(); break;

  default:break; 
 }
}

void drawFog()
{
  String command = "weatherIcon.pic=13";
  Serial.print(command);
  endNextionCommand();
}

void drawHeavySnowfall()
{
  String command = "weatherIcon.pic=8";
  Serial.print(command);
  endNextionCommand();
}

void drawModerateSnowfall()
{
  String command = "weatherIcon.pic=8";
  Serial.print(command);
  endNextionCommand();
}

void drawLightSnowfall()
{
  String command = "weatherIcon.pic=11";
  Serial.print(command);
  endNextionCommand();
}

void drawHeavyRain()
{
  String command = "weatherIcon.pic=10";
  Serial.print(command);
  endNextionCommand();
}

void drawModerateRain()
{
  String command = "weatherIcon.pic=6";
  Serial.print(command);
  endNextionCommand();
}

void drawLightRain()
{
  String command = "weatherIcon.pic=6";
  Serial.print(command);
  endNextionCommand();
}

void drawLightRainWithSunOrMoon()
{
  String command = "weatherIcon.pic=7";
  Serial.print(command);
  endNextionCommand(); 
}
void drawThunderstorm()
{
  String command = "weatherIcon.pic=3";
  Serial.print(command);
  endNextionCommand();
}

void drawClearWeather()
{
  String command = "weatherIcon.pic=4";
  Serial.print(command);
  endNextionCommand();
}

void drawCloud()
{
  String command = "weatherIcon.pic=9";
  Serial.print(command);
  endNextionCommand();
}

void drawFewClouds()
{
  String command = "weatherIcon.pic=5";
  Serial.print(command);
  endNextionCommand(); 
}
