/*

ESP8266 default olarak 192.168.4.1 alır. 
Kendiniz IP atamak isterseniz setup kısmına "softAPConfig (local_ip, gateway, subnet)" komut satırını düzenleyip ekleyebilirsiniz.

ONUR YONTAR 2021




*/


#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

const char WiFiPassword[] = "1234567"; // wifi şifresini buradan değiştirebilirsiniz.
const char AP_NameChar[] = "GARAJ_KAPISI_AP" ; // Wifi adının buradan değiştirebilirsiniz. 
 
WiFiServer server(80);
 
String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = "<!DOCTYPE html><html><head><title>Garaj</title></head><body><div id='main'><h2>KAPI KONTROL</h2>";
String html_2 = "<form id='F1' action='KAPIAC'><input class='button' type='submit' style='text-align:center;font-size:200%;' value='KAPI AC' ></form><br>";
String html_3 = "<form id='F2' action='KAPIKAPAT'><input class='button' type='submit' style='text-align:center;font-size:200%;'  value='KAPI KAPAT' ></form><br>";
String html_4 = "<form id='F3' action='LEDAC'><input class='button' type='submit' style='text-align:center;font-size:200%;'  value='ISIK AC' ></form><br>";
String html_5 = "<form id='F4' action='LEDKAPAT'><input class='button' type='submit' style='text-align:center;font-size:200%;'  value='ISIK KAPAT' ></form><br>";
String html_6 = "<form id='F5' action='DUR'><input class='button' type='submit' style='text-align:center;font-size:300%;background-color:red;'  value='DUR' ></form><br>";
String html_7 = "</div></body></html>";
 
String request = "";


#define LEDR D5 // Projektör Rölesi
#define AROLE D6 // Kapıyı Aç röle
#define KROLE D7 // Kapıyı Kapatan Röle
#define BTNA D3 // Manuel Açma Buton 
#define BTNK D4 // Manuel Kapatma Buton
#define SNRA D1 // Kapı Açılma Sınır Anahtarı
#define SNRK D2 // Kapı Kapanma Sınır Anahtarı
#define LDR D0 // LDR Modülünün Sinyal Girişi

#define SURE 40000
  int MAXSURE;
  String ADURUM="0";
  String KDURUM="0";
  unsigned long eskiZaman = 0;
  unsigned long yeniZaman;
  




  

void setup() {
  Serial.begin(9600);
  
   boolean conn = WiFi.softAP(AP_NameChar, WiFiPassword, 13);
   server.begin();
  pinMode(LEDR, OUTPUT);
  pinMode(AROLE, OUTPUT);
  pinMode(KROLE, OUTPUT);
  pinMode(BTNA, INPUT_PULLUP);
  pinMode(BTNK, INPUT_PULLUP);
  pinMode(SNRA, INPUT_PULLUP);
  pinMode(SNRK, INPUT_PULLUP);
  pinMode(LDR, INPUT_PULLUP);
  digitalWrite(LEDR, HIGH);
  digitalWrite(AROLE, HIGH);
  digitalWrite(KROLE, HIGH);

   ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOT: FS kullandıysanız FS.end()komutu ile dosyayı kapatmalısınız
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Hata[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Yetki Hatası");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Baslatma Basarisiz");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Baglanti Basarisiz");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Veri Alinamadi");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Sonlandirma Basarisiz");
    }
  });
  ArduinoOTA.begin();//OTA'yi başlatıyor
  Serial.println("Sistem Hazir");
  Serial.print("IP Adresiniz: ");
  Serial.println(WiFi.localIP());
  

  
}






void loop() {
  ArduinoOTA.handle();
  websw();
  yeniZaman = millis();
  
  if (digitalRead(BTNA) == LOW && digitalRead(BTNK) == HIGH ){
    ADURUM= "1";
    KDURUM= "0";
    digitalWrite(AROLE, LOW);
    digitalWrite(KROLE, HIGH);
    
     Serial.println("1");
     if(digitalRead(LDR) == HIGH){
      digitalWrite(LEDR, LOW);
     }
     
     eskiZaman = yeniZaman;
     while((digitalRead(SNRA) == !LOW) && (ADURUM != "0") && (!((yeniZaman - eskiZaman) > (SURE))) && (digitalRead(BTNK) == !LOW)  ){
     yeniZaman = millis();
     websw();
     delay(50);
     Serial.println("While içinde");
      }
     Serial.println("While çıkıldı.");
     ADURUM= "0";
     digitalWrite(AROLE, HIGH);
     delay(1000);
  }

  else if (digitalRead(BTNK) == LOW && digitalRead(BTNA) == HIGH && digitalRead(SNRK) !=LOW){
    ADURUM= "0";
    KDURUM= "1";
    digitalWrite(AROLE, HIGH);
    digitalWrite(KROLE, LOW);
    Serial.println("3");
    eskiZaman = yeniZaman;
     while((digitalRead(SNRK) == !LOW) && (KDURUM != "0") && (!((yeniZaman - eskiZaman) > (SURE)))&& (digitalRead(BTNA) == !LOW)  ){
     yeniZaman = millis();
     websw();
     delay(50);
     Serial.println("While içinde");
      }
     Serial.println("While çıkıldı.");
     KDURUM= "0";
     digitalWrite(KROLE, HIGH); 
     delay(1000);
    
  }

    if (digitalRead(SNRA) == LOW ){
      digitalWrite(AROLE, HIGH);
      ADURUM = "0";
      delay(100);
      Serial.println("4");
    }

    else if (digitalRead(SNRK) == LOW ){
      digitalWrite(KROLE, HIGH);
      KDURUM = "0";
      eskiZaman = yeniZaman;
      delay(100);
      Serial.println("5");
    }


          if (digitalRead(SNRK) == LOW && digitalRead(BTNK) == LOW ){
            if(digitalRead(LEDR) == LOW){
          digitalWrite(LEDR, HIGH);
          Serial.println("6");
            }
            else if (digitalRead(LEDR) == HIGH){
          digitalWrite(LEDR, LOW);
          Serial.println("7");
            }
          }
        

       if (ADURUM == "1"){
        digitalWrite(AROLE, LOW);
        digitalWrite(KROLE, HIGH);
        Serial.println("8");
         if(digitalRead(LDR) == HIGH)
            digitalWrite(LEDR, LOW);
        eskiZaman = yeniZaman;
         while((digitalRead(SNRA) == !LOW) && (ADURUM != "0") && (!((yeniZaman - eskiZaman) > (SURE)))&& (digitalRead(BTNK) == !LOW)  ){
         yeniZaman = millis();
         websw();
         delay(50);
         
         Serial.println("While içinde");
          }
         Serial.println("While çıkıldı.");
         ADURUM= "0";
         digitalWrite(AROLE, HIGH);
         delay(1000);
       }
       else if (KDURUM == "1"){
        digitalWrite(AROLE, HIGH);
        digitalWrite(KROLE, LOW);
        Serial.println("9");
        eskiZaman = yeniZaman;
         while((digitalRead(SNRK) == !LOW) && (KDURUM != "0") && (!((yeniZaman - eskiZaman) > (SURE)))&& (digitalRead(BTNA) == !LOW)  ){
         yeniZaman = millis();
         websw();
         delay(50);
         Serial.println("While içinde");
          }
         Serial.println("While çıkıldı.");
         KDURUM= "0";
         digitalWrite(KROLE, HIGH);
         delay(1000);
       }
        if (ADURUM == "0"){
        digitalWrite(AROLE, HIGH);
        Serial.println("10");
       }
        if (KDURUM == "0"){
        digitalWrite(KROLE, HIGH);
        Serial.println("11");
       }

            
       if (yeniZaman - eskiZaman >300000){
      digitalWrite(LEDR, HIGH);
      }
      delay(100);
}

void websw(){
  WiFiClient client = server.available();
    if (!client)  {  return;  }
 
    // Read the first line of the request
    request = client.readStringUntil('\r');
 
    if       ( request.indexOf("KAPIAC") > 0 )  { ADURUM="1";  }
    else if  ( request.indexOf("KAPIKAPAT") > 0 ) { KDURUM="1";   }
    else if  ( request.indexOf("LEDAC") > 0 ) { digitalWrite(LEDR, LOW);   }
    else if  ( request.indexOf("LEDKAPAT") > 0 ) { digitalWrite(LEDR, HIGH);   }
    else if  ( request.indexOf("DUR") > 0 ) { ADURUM="0",KDURUM="0" ;   }  
    client.flush();
 
    client.print( header );
    client.print( html_1 );
    client.print( html_2 );
    client.print( html_3 );
    client.print( html_4);
    client.print( html_5);
    client.print( html_6);
    client.print( html_7);
 
    delay(5);
}
  
