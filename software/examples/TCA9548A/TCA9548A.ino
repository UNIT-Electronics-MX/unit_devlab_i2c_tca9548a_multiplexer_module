
#include <Wire.h>

// ----- DIRECCION DEL TCA9548A ------
//A2=A1=A0=GND -> 0X70
//Ajustar de acuerdo a direccion personalizada
#define MUX_ADDR 0x70

// ----- PINES I2C -------
#define SDA_PIN 6
#define SCL_PIN 7

// ----- Parametros EEPROM (24Cxx)
#define EEPROM_PAGE_SIZE 32
// ---------- Estado detectado ----------
uint8_t  eepromAddrWidth = 2;      // 2 = direccionamiento de 16 bits; 1 = 8 bits (24C16 y menores)
uint32_t eepromSizeBytes = 4096;   // por defecto 32 Kbit (4 KB)
size_t   eepromPageSize  = 32;     // se ajusta tras 'capacity'

// Tabla de dispositivos por canal 
//Guarda la direccion I2C activa para cada uno de los 8 canales 
//Por defecto todos apuntan a ox50 (EEPROM/FRAM estandar)
//Comando target <canal> <addr> para cambiarlos 


uint8_t channelDevice[8] = {
  0x50,
  0x50,
  0x50,
  0x50,
  0x50,
  0x50,
  0x50,
  0x50,
};

bool muxSelectChannel(uint8_t channel){
  if(channel > 7){
    Serial.println("[MUX Config] Error: Canal debe ser 0-7");
    return false;
  }
  //Iniciamos la transmisión en la dirección del multiplexor asignada
  Wire.beginTransmission(MUX_ADDR);
  //Desplazamos 1 para activar el canal especificado
  Wire.write(1 << channel);
  uint8_t err = Wire.endTransmission();

  if(err != 0){

    Serial.printf("[MUX] Error al seleccionar canal %d (err=%d)\n", channel, err);
    Serial.println("¿Está el TCA9548A conectado? ¿Dirección correcta?");
    return false;
  }
  return true;

}

bool muxCloseChannels(){
  //Iniciamos la transmisión en la dirección del multiplexor asignada
  Wire.beginTransmission(MUX_ADDR);

  Wire.write(0x00);
  return Wire.endTransmission() == 0;

}

uint8_t muxReadStatus(){
  if(Wire.requestFrom((int)MUX_ADDR, 1) != 1) return 0xFF;
  return Wire.read();
}

// ============================================================
//  Operaciones genéricas I2C (raw)
// ============================================================

bool rawWrite(uint8_t channel, uint8_t devAddress, uint8_t data){
  //Si no hemos seleccionado canal retornamos falso
  if(!muxSelectChannel(channel)) return false;
  //Iniciamos transmision con la direccion I2C del canal especificado
  Wire.beginTransmission(devAddress);
  //Enviamos byte de datos
  Wire.write(data);
  return Wire.endTransmission() == 0;
}

bool rawRead(uint8_t channel, uint8_t devAddress, uint8_t * buffer, uint8_t len){
  //Si no hemos seleccionado canal retornamos falso
  if(!muxSelectChannel(channel)) return false;
  //Obtenemos la petición con la direccion especificada y la longitud 
  int got = Wire.requestFrom((int) devAddress,(int)len);
  //Si la longitud reqerida es distinta de got retornamos false
  if(got != len) return false;

  //Obtenemos el arreglo leyendo el buffer haciendo superposicion de el puntero
  for(int i = 0; i < len; i++) buffer[i] = Wire.read();
  return true;
}

// ============================================================
//  EEPROM (24Cxx) con paginado
// ============================================================

void eepromWaitReady(uint8_t devAddress, uint32_t timeout = 25) {
  uint32_t t0 = millis();
  while (millis() - t0 < timeout) {
    Wire.beginTransmission(devAddress);
    if (Wire.endTransmission() == 0) return;
    delay(1);
  }
}

bool eepromSetPointer(uint8_t channel,uint16_t mem) {
  
  //if (!muxSelectChannel(channel)) return false;

  uint8_t dev = channelDevice[channel];
  Serial.printf("Canal %d configurado con la direccion 0x%02X",channel , dev);
  Wire.beginTransmission(dev);
  if (eepromAddrWidth == 2) {
    Serial.println("FRAM O EEPROM BIEN CONFIGURADO");
    Wire.write(mem >> 8);
    Wire.write(mem & 0xFF);
  } else {
    Serial.println("FRAM O EEPROM MAL CONFIGURADO");
    Wire.write((uint8_t)(mem & 0xFF));
  }
  return (Wire.endTransmission(false) == 0);
}


/**
 * @brief Escribe a memoria EEPROM
 * 
 * Esta función configura los registros necesarios para
 * comenzar la lectura del sensor vía I2C.
 * 
 * @param channel Canal interno del multiplexor
 * @param devAddress Direccion de memoria del dispositivo conectado a el canal especificado
 * @param mem Direccion de memoria del dispositivo conectado a el canal especificado
 * @return true si la inicialización fue exitosa
 * @return false si ocurrió un error
 */
bool eepromWritePaged(uint8_t channel, uint16_t devAddress, const uint8_t* data, size_t len) {
  if (!muxSelectChannel(channel)){
    muxSelectChannel(channel);
    channelDevice[channel] = devAddress;
  }

  size_t off = 0;
  while (off < len) {
    size_t pageSpace = EEPROM_PAGE_SIZE - ((devAddress + off) % EEPROM_PAGE_SIZE);
    size_t chunk = min(len - off, pageSpace);
    if (eepromAddrWidth == 1) {
      size_t blockSpace = 256 - ((devAddress + off) & 0xFF);  // no cruzar bloque de 256 B
      if (chunk > blockSpace) chunk = blockSpace;
    }
    uint8_t dev = channelDevice[channel];
    Wire.beginTransmission(dev);
      if (eepromAddrWidth == 2) {
        Wire.write((uint8_t)((devAddress + off) >> 8));
        Wire.write((uint8_t)((devAddress + off) & 0xFF));
      } else {
        Wire.write((uint8_t)((devAddress + off) & 0xFF));
      }
      Wire.write(data + off, chunk);
      if (Wire.endTransmission() != 0) return false;
      eepromWaitReady(devAddress);
      off += chunk;
  }
  
  return true;
}

/**
 * @brief Escribe a memoria EEPROM
 * 
 * Esta función configura los registros necesarios para
 * comenzar la lectura del senso if (Wire.requestFrom((int)devAddr, (int)req) != (int)req) return false;
    for (size_t i = 0; i < req; i++) buf[got++] = Wire.read();r vía I2C.
 * 
 * @param channel Canal interno del multiplexor
 * @param devAddress Direccion de memoria del dispositivo conectado a el canal especificado
 * @param mem Direccion de memoria del dispositivo conectado a el canal especificado
 * @return true si la inicialización fue exitosa
 * @return false si ocurrió un error
 */
 /*
bool eepromReadSeq(uint8_t channel, uint8_t devAddress, uint16_t mem, uint8_t* buffer, size_t len){
  if (!muxSelectChannel(channel)) return false;

  size_t got = 0;
  while (got < len){
    //Apuntar a la direccion interna de la EEPROM
    Wire.beginTransmission(devAddress);
    Wire.write((mem + got) >> 8);
    Wire.write((mem + got) & 0xFF);
    if (Wire.endTransmission(false) != 0) return false;  // false = no STOP, Repeated-START
    // Leer chunk
    size_t req = min((size_t)32, len - got);
    if (Wire.requestFrom((int)devAddress, (int)req) != (int)req) return false;
    for (size_t i = 0; i < req; i++) buffer[got++] = Wire.read();
  }
  return true;
}
*/

bool eepromReadSeq(uint8_t channel, uint8_t devAddress, uint16_t mem, uint8_t *buf, size_t len) {
    if (!muxSelectChannel(channel)){
    muxSelectChannel(channel);
    channelDevice[channel] = devAddress;
    }
  size_t got = 0;
  while (got < len) {
    if (!eepromSetPointer(channel,mem + got)) return false;
    size_t req = min((size_t)32, len - got);
    if (eepromAddrWidth == 1) {
      size_t blockRemaining = 256 - ((mem + got) & 0xFF);
      if (req > blockRemaining) req = blockRemaining;
    }
    uint8_t dev = channelDevice[channel];
    if (Wire.requestFrom((int)dev, (int)req) != (int)req) return false;
    for (size_t i = 0; i < req; i++) buf[got++] = Wire.read();
  }
  return true;
}
// ============================================================
//  FRAMWrite (24Cxx) con paginado
// ============================================================

/**
 * @brief Escribe a memoria EEPROM
 * 
 * Esta función configura los registros necesarios para
 * comenzar la lectura del senso if (Wire.requestFrom((int)devAddr, (int)req) != (int)req) return false;
    for (size_t i = 0; i < req; i++) buf[got++] = Wire.read();r vía I2C.
 * 
 * @param channel Canal interno del multiplexor
 * @param devAddress Direccion de memoria del dispositivo conectado a el canal especificado
 * @param mem Direccion de memoria del dispositivo conectado a el canal especificado
 * @return true si la inicialización fue exitosa
 * @return false si ocurrió un error
 */

 bool framWrite(uint8_t channel, uint8_t devAddress, uint16_t mem, const uint8_t* data, size_t len){
    if (!muxSelectChannel(channel)) return false;
    Wire.beginTransmission(devAddress);
    Wire.write(mem >> 8);
    Wire.write(mem & 0xFF);
    Wire.write(data,len);
    return Wire.endTransmission() == 0;
 }

 /**
 * @brief Escribe a memoria EEPROM
 * 
 * Esta función configura los registros necesarios para
 * comenzar la lectura del senso if (Wire.requestFrom((int)devAddr, (int)req) != (int)req) return false;
    for (size_t i = 0; i < req; i++) buf[got++] = Wire.read();r vía I2C.
 * 
 * @param channel Canal interno del multiplexor
 * @param devAddress Direccion de memoria del dispositivo conectado a el canal especificado
 * @param mem Direccion de memoria del dispositivo conectado a el canal especificado
 * @return true si la inicialización fue exitosa
 * @return false si ocurrió un error
 */

bool framRead(uint8_t channel, uint8_t devAddress, uint16_t mem, uint8_t* buffer, size_t len){
    if (!muxSelectChannel(channel)) return false;
    Wire.beginTransmission(devAddress);
    Wire.write(mem >> 8);
    Wire.write(mem & 0xFF);
    if (Wire.endTransmission(false) != 0) return false;  // false = no STOP, Repeated-START
    size_t got = 0;
    while(got < len){
      size_t req = min((size_t)32, len - got);
      if (Wire.requestFrom((int)devAddress, (int)req) != (int)req) return false;
      for (size_t i = 0; i < req; i++) buffer[got++] = Wire.read();
    }
  return true;
}

// ============================================================
//  Display Utilities
// ============================================================

void dumpHex(const uint8_t* buffer, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (i && i % 16 == 0) Serial.println();
    Serial.printf("%02X ", buffer[i]);
  }
  Serial.println();
}

void dumpAscii(const uint8_t* buffer, size_t n) {
  for (size_t i = 0; i < n; i++)
    Serial.print((buffer[i] >= 32 && buffer[i] < 127) ? (char)buffer[i] : '.');
  Serial.println();
}

// ============================================================
//  Comandos del terminal
// ============================================================

void cmdScan(){
  Serial.println("\n[MUX SCAN] Buscando dispositivos en cada canal...");
  bool muxFound = false;

  //Verificar que el mux responde
  Wire.beginTransmission(MUX_ADDR);
  if(Wire.endTransmission() == 0){
    Serial.printf(" [OK] TCA9548A detectado en 0x%02X\n\n", MUX_ADDR);
    muxFound = true;
  }else{
    Serial.printf(" [ERROR] TCA9548A NO encontrado en 0x%02X\n\n",MUX_ADDR);
    Serial.println(" Revisa cableado VCC/GND/SDA/SCL y pines A0-A2");
    return;
  }

  for(uint8_t ch = 0; ch < 8; ch++){
    //Activar canal
    Wire.beginTransmission(MUX_ADDR);
    Wire.write(1 << ch);
    if(Wire.endTransmission() != 0) continue;

    bool found = false;

    for(uint8_t addr = 1; addr < 127; addr ++){
      if(addr == MUX_ADDR) continue;
      Wire.beginTransmission(addr);
      if(Wire.endTransmission() == 0){
        if(!found) Serial.printf(" Canal %d: \n",ch);
        found = true;
        Serial.printf("  -> 0x%02X",addr);
        //Identify Common Devices
        if(addr >= 0x50 && addr <= 0x57) Serial.print(" Posible (EEPROM/FRAM)");
        if(addr >= 0x68 && addr <= 0x6B) Serial.print(" Posible (IMU/RTC)");
        if(addr >= 0x71 && addr <= 0x77) Serial.print(" Posible (BMP280/BM3280 o Multiplexor)");
        if(addr >= 0x18 && addr <= 0x1F) Serial.print(" Posible Sensor)");
        Serial.println();
      }
      delay(2);
    }
    if(!found) Serial.printf("Canal %d: (vacío o mal configurado)\n",ch);

    //Close all channels
    muxCloseChannels();
    

  }
  Serial.println("\n [MUX SCAN] Completo. Canales Cerrados");
}

void cmdSend(uint8_t channel, uint8_t address, uint8_t data){
  Serial.printf("[SEND] Canal %d -> dispositivo 0x%02X -> dato 0x%02x\n",channel,address, data);
  //Step 1 : Select channel on mux
  Serial.printf("1 . Escribiendo 0x%02x al mux (0x%02X) para abrir canal %d\n",(1 << channel), MUX_ADDR, channel);

  //Step 2 : Send data to the device
  Serial.printf("2 . Enviando 0x%02x al dispositivo 0x%02X \n",data, address);

  if(rawWrite(channel, address, data))
    Serial.println(" [OK] Dato enviado");
  else
    Serial.println("[Error] Dispositivo no respondio");

}

void cmdSetChannel(uint8_t channel, uint8_t address){
  if(channel > 7) { Serial.println("[Error] Canal debe ser 0 - 7"); return;}

  //Verify device answer us 
  Serial.printf("[Target] Verificando 0x%02X en canal %d... \n",address, channel);
  if(!muxSelectChannel(channel)) return;
  
  Wire.beginTransmission(address);
  bool found = (Wire.endTransmission() == 0);
  
  channelDevice[channel] = address;  

  if(found)
    Serial.printf("[CANAL OK] Canal %d -> 0x%02X respondio correctamente\n", channel, address); 
  else
    Serial.printf("[CANAL ERROR] Canal %d -> 0x%02X guardado pero dispositivo no respondio correctamente\n");
}

void cmdRead(uint8_t channel, uint16_t mem, int len){
  if(len <= 0 || len > 256){Serial.println("len: 1 - 128"); return;}
  uint8_t buffer[256];
  uint8_t dev = channelDevice[channel];

  Serial.printf("[FRAM] Canal %d | dispositivo 0x%02X | mem 0x%04X | %d bytes\n",channel,dev,mem,buffer,len);
  //if(framRead(channel, dev, mem, buffer, len)){
  if(eepromReadSeq(channel,dev,mem,buffer,len)){
    Serial.printf("HEX:   "); dumpHex(buffer,len);
    Serial.printf("ASCII:  "); dumpAscii(buffer,len);
  }else{
    Serial.println("[Error] Lectura no valida");
  }

}

void cmdWrite(uint8_t channel, uint16_t devAddress,const String& text){
  
  if (eepromWritePaged(channel, devAddress, (const uint8_t*)text.c_str(), text.length()))
    Serial.printf("Escrito en 0x%04X\n", devAddress);
  else
    Serial.println("Error de escritura (WP alto, dirección o bus).");
}
void printHelp(){
  Serial.println("\n ----- Comandos disponibles ----------------------------------------");
  Serial.println("  scan                               Escanea todos los canales del mux");
  Serial.println("  mux <ch>                                Activa canal 0-7 manualmente");
  Serial.println("  close                                       Cierra todos los canales");
  Serial.println("  status                            Lee el registro de control del mux");
  Serial.println("");
  Serial.println("  setchannel <ch> <addr>      Asigna dirección de dispositivo al canal");
  //Serial.println("  send <ch> <addr> <data>     Asigna dirección de dispositivo al canal");
  Serial.println("  read <ch> <addr> <len>     Asigna dirección de dispositivo al canal");
  Serial.println("  write <ch> <addr> <texto>     Asigna dirección de dispositivo al canal");
}
// ============================================================
//  Parser
// ============================================================

String input;

void handleCommand(String line){

  line.trim();

  if (line.length() == 0) return;

  if(line == "scan") {cmdScan(); return;}
  if(line == "help") {printHelp(); return;}

  if(line.startsWith("setchannel")){
   int sp1 = line.indexOf(' ');
   int sp2 = line.indexOf(' ', sp1 + 1);
   if (sp1 < 0 || sp2 < 0) { 
    Serial.println("Uso: canal <0-7> <0xXX>"); 
    return; 
   }
   String chStr = line.substring(sp1 + 1,sp2);
   String addrStr = line.substring(sp2 + 1);
   uint8_t channel = chStr.toInt();

   if(channel > 7){
    Serial.println("[Error] Canal debe ser 0-7");
    return;
   }  

   uint8_t address = addrStr.startsWith("0x") ? (uint8_t)strtoul(addrStr.c_str(),nullptr,16) : (uint8_t) addrStr.toInt();

   cmdSetChannel(channel,address);
   return;
  }

  //send <ch> <addr> <byte>
  if(line.startsWith("send")){
    int sp1 = line.indexOf(' ');
    int sp2 = line.indexOf(' ', sp1 + 1);
    int sp3 = line.indexOf(' ', sp2 + 1);
    if(sp1 < 0 || sp2 < 0 || sp3 < 0){
      Serial.println("Uso: send <channel> <address: 0xXX> <data: OxAA"); 
    }
    String channelStr = line.substring(sp1 + 1,sp2);
    String addrStr = line.substring(sp2 + 1, sp3);
    String dataStr = line.substring(sp3 + 1);

    uint8_t channel = channelStr.toInt();
    if(channel > 7){
      Serial.println("[Error] Canal debe ser 0-7");
      return;
    }  

    uint8_t address = addrStr.startsWith("0x") ? (uint8_t)strtoul(addrStr.c_str(),nullptr,16) : (uint8_t) addrStr.toInt();

    uint8_t data = dataStr.startsWith("0x") ? (uint8_t)strtoul(dataStr.c_str(),nullptr,16) : (uint8_t) dataStr.toInt();
    cmdSend(channel,address,data);
    return;
  }

  //read <ch> <addr> <size>
  if(line.startsWith("read")){
    int sp1 = line.indexOf(' ');
    int sp2 = line.indexOf(' ', sp1 + 1);
    int sp3 = line.indexOf(' ', sp2 + 1);
    if(sp1 < 0 || sp2 < 0 || sp3 < 0){
      Serial.println("Uso: fread <channel> <address: 0xXX> <size: bytes"); 
    }
    String channelStr = line.substring(sp1 + 1,sp2);
    String addrStr = line.substring(sp2 + 1,sp3);
    String sizeStr = line.substring(sp3 + 1);

    uint8_t channel = channelStr.toInt();
    if(channel > 7){
      Serial.println("[Error] Canal debe ser 0-7");
      return;
    } 
    uint8_t address = addrStr.startsWith("0x") ? (uint8_t)strtoul(addrStr.c_str(),nullptr,16) : (uint8_t) addrStr.toInt();
    
    int size = sizeStr.toInt();

    cmdRead(channel,address,size);

  }

    //read <ch> <addr> <data>
  if(line.startsWith("write")){
    int sp1 = line.indexOf(' ');
    int sp2 = line.indexOf(' ', sp1 + 1);
    int sp3 = line.indexOf(' ', sp2 + 1);
    if(sp1 < 0 || sp2 < 0 || sp3 < 0){
      Serial.println("Uso: write <channel> <address: 0xXX> <texto"); 
    }
    String channelStr = line.substring(sp1 + 1,sp2);
    String addrStr = line.substring(sp2 + 1,sp3);
    String textoStr = line.substring(sp3 + 1);

    uint8_t channel = channelStr.toInt();
    if(channel > 7){
      Serial.println("[Error] Canal debe ser 0-7");
      return;
    } 
    uint8_t address = addrStr.startsWith("0x") ? (uint8_t)strtoul(addrStr.c_str(),nullptr,16) : (uint8_t) addrStr.toInt();
    

    cmdWrite(channel, address, textoStr);

  }
  
}

void setup() {
  // put your setup code here,= to run once:
  Serial.begin(115200);
  delay(300);

  #if defined(ESP8266) || defined(ESP32)
  Wire.begin(SDA_PIN, SCL_PIN);
#else
  Wire.begin();
#endif
  Wire.setClock(100000);  // 100 kHz - standard mode, más robusto para pruebas
  delay(500);
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║  TCA9548A Multiplexor I2C Demo       ║");
  Serial.println("║  Mux: 0x70                           ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println("Escribe 'help' para ver los comandos.");
  Serial.println("Escribe 'scan' para detectar qué hay conectado.");

 // Verificar que el mux responde al iniciar
  Wire.beginTransmission(MUX_ADDR);
  if (Wire.endTransmission() == 0)
    Serial.printf("\n✓ TCA9548A encontrado en 0x%02X\n", MUX_ADDR);
  else
    Serial.printf("\n✗ TCA9548A NO encontrado en 0x%02X - revisa cableado\n", MUX_ADDR); 
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length()) { handleCommand(input); input = ""; }
    } else {
      input += c;
    }
  }
}
