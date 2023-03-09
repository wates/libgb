int latchPin = 11;
int clockPin = 12;
int serPin = 10;
int readPin = A5;
int writePin = 13;
int dataPin = 2;

int num_bank = 0;
void setup() {
  Serial.begin(38400);
  Serial.write('\r');
  Serial.write('\n');
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(serPin, OUTPUT);
  pinMode(writePin, OUTPUT);
  pinMode(readPin, OUTPUT);
  for(int i=0;i<8;i++){
    pinMode(dataPin+i, INPUT);
  }
  digitalWrite(readPin, LOW);
  digitalWrite(writePin, HIGH);
  Serial.print("// ");
uint8_t checksum = 0;
for (uint16_t address = 0x0134; address <= 0x014C; address++) {
    uint8_t dat=read(address);
    checksum = checksum - dat - 1;
      Serial.print(dat);
      Serial.write(',');
}
uint8_t acc=read(0x14d);
  Serial.print("// calc_sum=");
  Serial.print(checksum,DEC);
  Serial.print(" checksum=");
  Serial.print(acc,DEC);
  if(checksum != acc){
    delay(10000000);
  }

  uint32_t size=read(0x0148);
  num_bank = 2 << size;
  Serial.print("\r\n// num_bank=");
  Serial.print(num_bank,DEC);
  Serial.print(" type=");
  Serial.print(read(0x0147),HEX);
  Serial.write('\r');
  Serial.write('\n');
}

void printBuf(uint16_t addr,int bank,uint8_t *buf){
  Serial.print("// addr=");
  Serial.print(addr&0xff00,HEX);
  Serial.print(", bank=");
  Serial.print(bank,HEX);
  Serial.write('\r');
  Serial.write('\n');
  // if(addr&0x3f00)return;
  for(int i=0;i<256;i++){
    Serial.print(buf[i],DEC);
    Serial.write(',');
  }
  Serial.write('\r');
  Serial.write('\n');
}

void loop() {
  uint8_t buf[256];
  for(uint16_t a=0;a<0x4000;a++){
    buf[a&0xff]=read(a);
    if((a&0xff)==0xff){
      printBuf(a,0,buf);
    }
  }
  for(uint8_t b=1;b<num_bank;b++){
    setBank(b);
    for(uint16_t x=0x4000;x<0x8000;x++){
      buf[x&0xff]=read(x);
      if((x&0xff)==0xff){
        printBuf(x,b,buf);
      }
    }
  }
  delay(1000000);
}

void setBank(uint8_t bank){
  // for(int i=0x20;i<0x40;i++)
    write(0x2100,bank);
}

uint8_t read(uint16_t addr){
  address(addr);
  uint8_t data=0;
  for(uint8_t i=0;i<8;i++){
    uint8_t r = digitalRead(dataPin+i);
    data |= r==HIGH?(1<<(7-i)):0;
  }
  return data;
}

void write(uint16_t addr,uint8_t data){
  digitalWrite(readPin, HIGH);
  digitalWrite(writePin, LOW);
  for(uint8_t i=0;i<8;i++){
    pinMode(dataPin+i, OUTPUT);
  }
  delay(5);
  address(addr);
  delay(5);
  for(uint8_t i=0;i<8;i++){
    if((data>>i)&1){
      digitalWrite(dataPin+(7-i),HIGH);
    }else{
      digitalWrite(dataPin+(7-i),LOW);
    }
  }
  digitalWrite(readPin, LOW);
  digitalWrite(writePin, HIGH);
  delay(5);
  for(int i=0;i<8;i++){
    pinMode(dataPin+i, INPUT);
  }
  delay(5);
}

void address(uint16_t addr) {
  digitalWrite(latchPin, LOW);
  shiftOut(serPin, clockPin, MSBFIRST, addr >> 8);
  shiftOut(serPin, clockPin, MSBFIRST, addr & 0xff);
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(50);
}
