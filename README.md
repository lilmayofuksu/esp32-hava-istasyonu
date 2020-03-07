# esp32-hava-istasyonu
ESP32 ve Nextion ekran ile yapılan bir hava istasyonu

Gereken Malzemeler:
ESP32
DHT22 Sıcaklık/Nem sensörü ve BMP180 Basınç sensörü,
Nextion 3.2 İnç Ekran,
Kablolar ve prototip için breadboard.

Kurulumu:

Nextion ekranın RX kablosunu ESP32 nin TX0 pinine bağlayın.

BMP180 in SDA pinini D22 pinine, SCL pinini de D21 pinine bağlayın.

DHT22 nin data pinini D33 pinine bağlayın.

Koda Wifi bilgilerinizi, API keyinizi ve City ID nizi girerek ESP32 ye yükleyin.

Nextion Ekran kodunu SD Kartla veya Arduino/UART(USB TTL) adaptörü ile yükleyin. Eğer Arduino ile yükleyecekseniz ekranın
TX kablosunu Arduino da TX e, RX Kablosunu da RX e bağlayın.
Arduino yu USBTTL olarak kullanmak için gereken kod:

```
void setup(){
pinMode(0,INPUT);   
pinMode(1,INPUT); 
} 
void loop(){ 
}  
```

Ekranı programladıktan sonra ekranın RX pinini ESP32 nin TX0 pinine bağlayıp kullanmaya başlayabilirsiniz.
Sensörlerin güç pinlerini bağlamayı unutmayın.
Bu kodu eğer kullanmak için bir sebep bulup ve hata gördüyseniz, Issue veya Pull Request açabilirsiniz.
