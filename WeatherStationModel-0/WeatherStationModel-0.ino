/*                    VERSÃO FINAL DO CODIGO
               VERSÃO 1.0.1      DATA: 12/01/2017
               COMPILADO NA VERSAO ARDUINO: 1.8.1
               __________________________________

               VERSAO DO PROGRAMA ARDUINO: 1.8.1
               PROGRAMA ATUALIZADO EM: 21/01/2017
               HORA-ULTIMO UPDATE: 13:30 p.m.
               __________________________________

                PLACA WIFI ESP8266-07 AT THINKER
                PROGRAMA: MINI ESTACAO CLIMATICA
                CONTÉM SENSORES: BMP-180 E DHT22
               __________________________________

               CONFIGURACAO DA PLACA PARA GRAVACAO
               PLACA: GENERIC ESP8266 MODULE
               FLASH MODE: DIO
               FLASH FREQUENCY: 40MHz
               CPU FREQUENCY: 80MHz
               FLASH SIZE: 1M (512K SPIFFS)
               DEBUG PORT: SERIAL
               DEBUG LEVEL: OTA + UPDATER
               RESET MOTHOD: ck
               UPLOAD SPEED: 115200
               PORTA: PORTA ESP CONECTADA AO COMPUTADOR
               __________________________________
*/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// AGROTECHLINK.COM - ESP8266 - PROGRAM HEADER TEMPLATE - 2017 - JANUARY
//                    TODOS OS DIREITOS RESERVADOS
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*   CADA GPIO POSSUI UMA IDENTIFICACAO ESPECIFICA
     PORTAS UTILIZADAS NAS PLACAS DA MINI ESTACAO CLIMATICA

     ATL3     >--> GPIO-16 + LED0
     ATL4     >--> GPIO-14 + BUZZER
     ATL5     >--> GPIO-12 + SENSOR DHT22 (TEMPERATURA-HUMIDADE)
     ATL7     >--> GPIO-05 + SCL >--> PULLUP INTERNO / SENSOR BMP-180 (PRESSAO)
     ATL8     >--> GPIO-04 + SDA >--> PULLUP INTERNO / SENSOR BMP-180 (PRESSAO)
     /RST     >--> ---[10k]--+3V3  -//- JUMPER COM 0V P/ "RESET"
                   +++[103]---CERAMIC CAPACITOR
     CH-PD    >--> ---[10k]--+3V3 + 103 CERAMIC CAPACITOR
                   +++[103]---CERAMIC CAPACITOR
     GPIO-02  >--> ---[10k]--+3V3
     GPIO-00  >--> ---[10k]--+3V3  -//- JUMPER COM 0V P/ "FLASH"
     GPIO-15  >--> ---[10k]---0V
     RX + TX    >--> CONEXOES PARA GRAVADOR EXTERNO
     PROCEDIMENTO PARA GRAVACAO COM GRAVADOR FTDI
     FTDI-TX  >--> ATL-RX
     FTDI-RX  >--> ATL-TX
     FTDI-3V3 >--> ATL-3V3
     FTDI-0V  >--> ATL-0V

     NUNCA ALIMENTAR ESTE MODULO DIRETAMENTE PELO GRAVADOR
     OU USB DO COMPUTADOR!
*/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// LIVRARIAS EXTERNAS PARA FUNCIONAMENTO DOS SENSORES, CONEXÃO E DADOS
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#include     <FS.h>                  // BIBLIOTECA WiFi DO ESP8266. DEVE SER SEMPRE A PRIMEIRA BIBLIOTECA MENCIONADA NO #INLUDE!!!!!!!
#include     <ESP8266WiFi.h>         // BIBLIOTECA WiFi DO ESP8266
#include     <DNSServer.h>           // BIBLIOTECA WiFi DO ESP8266
#include     <ESP8266WebServer.h>    // BIBLIOTECA WiFi DO ESP8266
#include     "WiFiManager.h"         // BIBLIOTECA WiFi-MANANGER DO ESP8266
#include     <TimeLib.h>             // BIBLIOTECA DE DATA E HORA DO ESP8266
#include     <WiFiUdp.h>             // BIBLIOTECA DE DATA E HORA DO ESP8266
#include     <SFE_BMP180.h>          // SENSOR BMP-180 (PRESSAO) 
#include     <Wire.h>                // NECESSÁRIO PARA COMUNICACAO I2C (PRESSAO)
#include     "DHT.h"                 // SENSOR DHT22 OU DHT11 (TEMPERATURA-HUMIDADE)   
#include     <MySQL_Connection.h>    // CONEXAO COM BANCO DE DADOS
#include     <MySQL_Cursor.h>        // CONEXAO COM BANCO DE DADOS
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// AGROTECHLINK MINI ESTACAO CLIMATICA - PINOUTS - DEFINES - DESCRICOES
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define      LED_BUILTIN   2         // LED_BUILTIN (LED NATIVO DO ESP8266)
#define      ATL3         16         // GPIO-16 + LED0
#define      ATL4         14         // GPIO-14 + BUZZER
#define      ATL5         12         // GPIO-12 + SENSOR DHT22 (TEMPERATURA-HUMIDADE)
#define      ATL7          5         // GPIO-05 + SCL >--> PULLUP INTERNO / SENSOR BMP-180 (PRESSAO)
#define      ATL8          4         // GPIO-04 + SDA >--> PULLUP INTERNO / SENSOR BMP-180 (PRESSAO)
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// SENSOR PINS SETTINGS
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#define      DHTPIN     ATL5         // SENSOR DHT22 (TEMPERATURA-HUMIDADE)
#define      DHTTYPE    DHT22        // ESPECIFICACAO DO SENSOR UTILIZADO
/*#define      DHTTYPE    DHT11*/    // DESMARCAR ESTE DEFINE CASO UTILIZAR O DHT11 E SUBLINHAR DHT22!!!
#define      ALTITUDE   4.5          // ALTITUDE DA LOCALIZACAO DO ESP8266: JARAGUÁ DO SUL - SC (EM METROS - USAR PONTO AO INVES DA VIRGULA. Ex.:(23.3))
WiFiUDP      Udp;                    // DEFINICAO DA BIBLIOTECA UDP DATA E HORA
DHT          dht (DHTPIN, DHTTYPE);  // ENDERECAMENTO DO SENSOR DHT22
SFE_BMP180   pressao;                // DEFINICAO DO SENSOR BMP-180
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// DEFINICAO DAS VARIAVEIS GLOBAIS
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static const char   ntpServerName[] = "a.st1.ntp.br";    // DEFINICAO DO SERVIDOR NTP DE HORA BRASILEIRO
const int           timeZone = -2, NTP_PACKET_SIZE = 48; // DEFINIDO COMO -2 DEVIDO AO HORÁRIO DE VERÃO. DEPOIS DO DIA 19/02 MUDAR PARA -3!
unsigned int        localPort = 8888;                    // PORTAL LOCAL PARA OS PACOTES UDP
byte                packetBuffer[NTP_PACKET_SIZE];       // BUFFER PARA OS PACOTES DE DATA E HORA
double              baseline, P_bmp, T_bmp;              // VARIAVEIS PARA O SENSOR BMP-180
float               T_dht, U_dht;                        // VARIAVEIS PARA O SENSOR DHT22 OU DHT11
int                 nCon, contNcon, fMysql;              // VARIAVEIS PARA WiFi MANANGER E MySQL (CONEXAO COM A INTERNET E MySQL EM CASO DE ERROS)
String              tempo = "hh:MM:ss | dd/mm/aaaa";     // STRING DA DATA E HORA PARA O MYSQL
String              hora, minuto, segundo, dia, mes, ano;// VARIAVEIS DA DATA E HORA MYSQL
char                STempo[30];                          // VARIAVEL DA DATA E HORA MYSQL
unsigned long       previousMillis = 0;                  // CONTADOR DE TEMPO PARA SUBIR OS DADOS NO MySQL
const long          intervalo = 1200000;                 // INTERVALO DE 20 MINUTOS PARA SUBIR OS DADOS NO MySQL
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// CONFIGURACOES DE ACESSO AO BANCO DE DADOS
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
IPAddress   server_addr (186, 202, 127, 122);   // IP DO MySQL SERVER - SITE AGROTECHLINK.COM
char        user[] = "agrotech_u_intel";        // USUARIO DO BANCO DE DADOS
char        password[] = "OlvAgrotechlink1357"; // SENHA DO USUARIO

WiFiClient client;
MySQL_Connection conn((Client *)&client);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// FORMATACAO DA BIBLIOTECA DE DATA E HORA
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
time_t      getNtpTime();
void        relogioDisplay();
void        printDigits(int digits);
void        sendNTPpacket(IPAddress &address);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// BIBLIOTECAS DE DATA E HORA
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
time_t prevDisplay = 0;
time_t getNtpTime()
{
  IPAddress ntpServerIP;
  while (Udp.parsePacket() > 0) ;
  WiFi.hostByName(ntpServerName, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  } return 0;
}
void dataHora()
{
  hora = String(hour());
  minuto = String(minute());
  segundo = String(second());
  dia = day();
  mes = month();
  ano = year();
  tempo.replace("hh", hora);
  tempo.replace("MM", minuto);
  tempo.replace("ss", segundo);
  tempo.replace("dd", dia);
  tempo.replace("mm", mes);
  tempo.replace("aaaa", ano);
  tempo.toCharArray(STempo, 30);
}
void sendNTPpacket(IPAddress &address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// MODO CONFIGURAÇÃO DO WiFi MANANGER
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void configModoCallback (WiFiManager *myWiFiManager) {
  Serial.println(">--> Iniciando modo de configuracao.");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// FAZER O LED PISCAR
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void LedATLblinks(unsigned M) {
  for (short j = 0; j < M; j++) {
    digitalWrite(ATL3, HIGH);                             delay(250);
    digitalWrite(ATL3, LOW);                              delay(250);
  }
  digitalWrite(ATL3, LOW);
  digitalWrite(LED_BUILTIN, LOW);                         delay(250);
  digitalWrite(LED_BUILTIN, HIGH);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// PRESSAO E TEMPERATURA DO BMP180 - ESPECIAL
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
float getPressure() {
  char status;
  status = pressao.startTemperature();
  if (status != 0)  {
    delay(status);
    status = pressao.getTemperature(T_bmp);
    if (status != 0)    {
      status = pressao.startPressure(3);
      if (status != 0)      {
        delay(status);
        status = pressao.getPressure(P_bmp, T_bmp);
        if (status != 0) {
          return (P_bmp, T_bmp);
        }
        else Serial.print("\n>--> Erro ajustando - P - ESPECIAL!");
      }
      else Serial.print("\n>--> Erro na leitura - P - ESPECIAL!");
    }
    else Serial.print("\n>--> Erro ajustando temperatura BMP - ESPECIAL!");
  }
  else Serial.print("\n>--> Erro na leitura da temperatura BMP - ESPECIAL!");
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// GET DADOS DE TEMPERATURA E PRESSAO DO BMP-180
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void GetATLbmpPT() { // SENSOR BMP180
  getPressure();
  if (isnan(P_bmp)) {
    delay(500);
    getPressure();
    if (isnan(P_bmp)) {
      Serial.println("\n>--> Falha na leitura - p - do sensor BMP180!");
      return;
    }
  }
  if (isnan(T_bmp)) {
    delay(500);
    getPressure();
    if (isnan(T_bmp)) {
      Serial.println("\n>--> Falha na leitura - T - do sensor BMP180!");
      return;
    }
  }
  Serial.print("\n| Pressao atmosferica......BMP: ");
  Serial.print(P_bmp, 2);
  Serial.print(" hPa");
  Serial.println(" - - - |");
  Serial.print("| Temperatura ambiente.....BMP: ");
  Serial.print(T_bmp, 2);
  Serial.print(" *C");
  Serial.println("  - - - - |");
  Serial.println("| - - - - - - - - - - - - - - - - - - - - - - - - |");
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// GET DADOS DE TEMPERATURA E HUMIDADE DO DHT22 OU DHT11
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void GetATLdhtTU() {
  U_dht = dht.readHumidity();
  T_dht = dht.readTemperature();
  if (isnan(U_dht) || isnan(T_dht)) {
    delay(2000);
    // WHEN DHT22 IS USED SET THIS TO 2000, OTHERWISE SET TO 5000
    U_dht = dht.readHumidity();
    T_dht = dht.readTemperature();
    if (isnan(U_dht) || isnan(T_dht)) {
      Serial.println("\n>--> Falha na leitura do Sensor DHT!");
      T_dht = 0; // PARA ASSEGURAR SEJA REGISTRADO 0 NO BD DO MySQL
      U_dht = 0; // PARA ASSEGURAR SEJA REGISTRADO 0 NO BD DO MySQL
    }
  }
  Serial.print("\n| Temperatura ambiente.....DHT: ");
  Serial.print(T_dht);
  Serial.print(" *C");
  Serial.println("  - - - - |");
  Serial.print("| Umidade relativa do ar...DHT: ");
  Serial.print(U_dht);
  Serial.print(" %UR");
  Serial.println(" - - - - |");
  Serial.println("\n| - - - - - - - - - - - - - - - - - - - - - - - - |");
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);         // INICIALIZA O LED_BUILTIN NATIVO DO ESP8266
  digitalWrite(LED_BUILTIN, HIGH);      // DESLIGA O LED_BUILTIN NATIVO DO ESP8266
  pinMode(ATL3, OUTPUT);      digitalWrite(ATL3, LOW);  // GPIO-16 + LED0
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModoCallback);
  if (!wifiManager.autoConnect("Agrotechlink", "agrotechlink")) {
    Serial.println("Falha na conexao.");
    contNcon = (nCon + 1);
    delay(5000);
    ESP.restart();
    delay(2000);
    if (contNcon == 3) {
      wifiManager.resetSettings();
      // CASO OCORRA FALHAS EXEPCIONAIS NA CONEXAO O ESP8266 RESETA E INICIA NOVAMENTE
      delay(5000);
      ESP.reset();
      delay(2000);
    }
  }
  Serial.println("| Internet conectada. Wi-fi client em rede :)");
  delay(500);
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  Serial.println("| - - - - - - - - - - - - - - - - - - - - - - - - |");
  Serial.println("| AGROTECHLINK - RADIO BOARD TEST - ESP8266 - - - |");
  Serial.println("| Inicio verificacao de sensores: BMP-180 & DHT22 |");
  Serial.println("| Informacoes do ESP8266 - - - - - - - - - - - -  |");
  Serial.print("| MAC NUMBER: " + WiFi.macAddress());
  Serial.println(" - - - - - - - - - |");
  Serial.println("| INICIANDO UDP - - - - - - - - - - - - - - - - - |");
  Serial.println("| SINCRONIZANDO COM SERVIDOR NTP / TEMPO - - - -  |");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  Serial.println("| AGROTECHLINK - TODOS OS DIREITOS SAO RESERVADOS |");
  Serial.println("| - - - - - - - - - - - - - - - - - - - - - - - - |");
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  Serial.println("| Conectando ao banco de dados da Agrotechlink... |\n");
  while (conn.connect(server_addr, 3306, user, password) != true) {
    delay(500);
    Serial.print ( "." );
  }
  delay(500);

  Serial.println("| - - - - - - - - - - - - - - - - - - - - - - - - |");
  Serial.println("| Conexao com o banco de dados bem sucedida! (:   |\n");

  LedATLblinks(3);           // LED. 3 VEZES = PRIMEIRA CONEXAO COM BD OK!
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  dht.begin();               // INICIANDO SENSOR DE TEMPERATURA DHT22
  pressao.begin();           // INICIANDO SENSOR DE PRESSAO BMP-180
  getPressure();             // SETANDO CONFIGURACOES ESPECIAIS DO BMP-180
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// FIM DO SETUP E CONFIGURACOES. INICIO DO LOOP.
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void loop() {
  GetATLbmpPT();             // BMP-180
  GetATLdhtTU();             // DHT22
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  unsigned long currentMillis = millis();    // LOGICA DE TEMPO PARA SUBIDA DE DADOS A CADA 20 MINUTOS NO DB
  if (currentMillis - previousMillis >= intervalo) {
    if (conn.connected()) {
      char ST_dht[6], SU_dht[6], ST_bmp[6], SP_bmp[8], query[100];
      // CONVERTENDO DADOS DOS SENSORES PARA STRING
      dtostrf(T_dht, 2, 2, ST_dht);
      dtostrf(U_dht, 2, 2, SU_dht);
      dtostrf(T_bmp, 2, 2, ST_bmp);
      dtostrf(P_bmp, 4, 2, SP_bmp);
      dataHora();              //REGISTRANDO A HORA
      /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
      char INSERT_SQL[] = "INSERT INTO agrotech_intel.teste VALUES (NULL, %s, %s, %s, %s, '%s');";
      sprintf(query, INSERT_SQL, ST_dht, SU_dht, ST_bmp, SP_bmp, STempo);
      // CONCATENANDO A STRING INSERT_SQL PARA GRAVACAO NO BANCO DE DADOS
      Serial.println(query);
      Serial.println(STempo);
      MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
      delay(200);
      Serial.println("\n| Executando querry no banco de dados - - - - - - |");
      cur_mem->execute(query);
      delay(200);
      Serial.println("| Querry INSERT: - - - - - - - - - - - - - - - -  |");
      Serial.println(query);
      Serial.println("| Limpando dados de requisicao da memoria - - - - |");
      delete cur_mem;
      delay(250);
      Serial.println("| Execucao de querry bem sucedida! - - - - - - -  |");
      fMysql = 0;

      LedATLblinks(2);           // LED. 2 VEZES = DADOS INSERIDOS NO BD!

    } else {
      conn.close();
      Serial.println("| Reconectando ao banco de dados... - - - - - - - |");
      if (conn.connect(server_addr, 3306, user, password)) {
        delay(500);
        Serial.println("| Reconectando com sucesso! (: - - - - - - - - - - |");
        fMysql++;
        if (fMysql == 3) {
          Serial.println("| Maximo de reconexoes atingidas: MySQL. Resetando |");
          delay(5000);
          ESP.reset();
          delay(2000);
        }
      }
    }
    previousMillis = currentMillis;
  }
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
// MAIN FUNCTION END - FINAL
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
