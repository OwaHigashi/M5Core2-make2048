#include <M5Core2.h>

// for SD-Updater
#define SDU_ENABLE_GZ
#include <M5StackUpdater.h>

const int CENTERX = 320/2;
const int CENTERY = 230/2;

#define PANELN 16
#define PANELSIZ 48
#define BORDERSIZ 4

#define BGCOLOR 0x9451

const uint16_t PNLCOLOR[] = {0x9cb1,0xef3b,0xef19,0xf58f,0xf4ac,0xf3ec,0xf2e7,0xee6e,0xee6c,0xee4a,0xee47,0xee05};
const uint16_t NUMCOLOR[] = {0,0x7b8c,0x7b8c,0xff9e,0xff9e,0xff9e,0xff9e,0xff9e,0xff9e,0xff9e,0xff9e,0xff9e};

//Arrow Matrix
#define MATDTN  7
#define MATDTSZ  5
#define MATBORDER 1
const struct{int topx; int topy;}MATRIXPOS[] ={{5,70},{5,125},{270,70},{270,125}};
typedef struct{
    struct{int x;int y;}pos[MATDTN * MATDTN];
    u_char mat[MATDTN];
}T_ARROW; 
const u_char ARROWPATTERN[][MATDTN] = {
    {8,0x1c,0x2a,0x49,8,8,0}, /* TOP */
    {0,8,8,0x49,0x2a,0x1c,8}, /* DOWN */
    {8,4,2,0x3f,2,4,8},       /* LEFT */
    {8,0x10,0x20,0x7e,0x20,0x10,8} /* RIGHT */
};

#define GAMELOSE -1
#define GAMEWIN 100

class cls_gamebrd{
private:
  int m_panel[PANELN];
  struct{int x; int y;}m_panelpos[PANELN];
  
  int m_dirArrow;
  T_ARROW m_arrowMatrix[4]; 
  unsigned long m_anmArrowTime;    

public:
  cls_gamebrd();
  void drawInitBoard();
  void drawPanel(int);
  int  appendTwo(void);
  void coveron(void);
  void reverse(void);
  void transparent(void);
  void marge(void);
  void move(int);
  void dispArrow(int, int);
  void changeArrow(int);
  void setArrow(int);
  int getDir(void){return this->m_dirArrow;}
  void animationArrow(void);
  void gameEnd(int);
};

cls_gamebrd::cls_gamebrd(){
  randomSeed(analogRead(0));
  for(int i = 0; i < PANELN; i++){
      this->m_panelpos[i].x = (CENTERX - 2*(PANELSIZ+BORDERSIZ)) + ((i % 4) * (PANELSIZ+BORDERSIZ));
      this->m_panelpos[i].y = (CENTERY - 2*(PANELSIZ+BORDERSIZ)) + ((i / 4) * (PANELSIZ+BORDERSIZ));
  }

  for(int d = 0; d < 4; d++){
    for(int i = 0; i < MATDTN * MATDTN; i++){
      this->m_arrowMatrix[d].pos[i].x = MATRIXPOS[d].topx + (i % MATDTN)*(MATDTSZ+MATBORDER);
      this->m_arrowMatrix[d].pos[i].y = MATRIXPOS[d].topy + (i / MATDTN)*(MATDTSZ+MATBORDER);
    }
  }
}

void cls_gamebrd::drawInitBoard(){
  M5.Lcd.fillScreen(BGCOLOR);
  M5.Lcd.setTextWrap(true);
  
  for(int i = 0; i < PANELN; i++){
    this->m_panel[i] = 0;
    this->drawPanel(i);
  }

  for(int i = 0; i < 4; i++){
    this->dispArrow(i,0);
  }
  this->m_dirArrow = 0;
  this->dispArrow(this->m_dirArrow,1);

  M5.Lcd.fillRect(0,225,320,15,NAVY);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.drawFastVLine(108, 228, 12, WHITE);
  M5.Lcd.drawFastVLine(216, 228, 12, WHITE);
  M5.Lcd.drawCentreString("SEL.V", 50, 228, 1);
  M5.Lcd.drawCentreString("MOVE", 160, 228, 1);
  M5.Lcd.drawCentreString("SEL.H", 270, 228, 1);
}

void cls_gamebrd::drawPanel(int i){
  const char * numtxt[]={"0","2","4","8","16","32","64","128","256","512","1024","2048"};
  int n = this->m_panel[i];
  int fontid = 2; 
  M5.Lcd.fillRect(this->m_panelpos[i].x, this->m_panelpos[i].y, PANELSIZ, PANELSIZ, PNLCOLOR[n]);
  if(n){
    M5.Lcd.setTextColor(NUMCOLOR[n]);
    if(n < 10){fontid=4;}  
    int fontdy = (PANELSIZ - M5.Lcd.fontHeight(fontid))/2;
    M5.Lcd.drawCentreString(numtxt[n],this->m_panelpos[i].x + (PANELSIZ/2), this->m_panelpos[i].y+fontdy,fontid);
  }
}

int cls_gamebrd::appendTwo(void){
  int lst[PANELN];
  int count = 0;

  for(int i = 0; i < PANELN; i++){
    if(this->m_panel[i] > 10){
      return (i + GAMEWIN);
    }
    if(!this->m_panel[i]){
      lst[count++] = i;
    }
  }
  if(!count){return GAMELOSE;}
  int n = lst[random(count)];
  this->m_panel[n] = 1;
  this->drawPanel(n);
  return 0;
}

void cls_gamebrd::coveron(void){
  int next[PANELN];
  for(int i = 0; i < PANELN; i++){next[i] = 0;}
  for(int y = 0; y < 4; y++){
    int count = 0;
    for(int x = 0; x < 4; x++){
      if(this->m_panel[y*4+x]){
        next[y*4+count] = this->m_panel[y*4+x];
        count++;
      }
    }
  }
  for(int i = 0; i < PANELN; i++){this->m_panel[i] = next[i];}
}

void cls_gamebrd::reverse(void){
  int next[PANELN];
  for(int y = 0; y < 4; y++){
    for(int x = 0; x < 4; x++){
      next[y*4+x] = this->m_panel[y*4+3-x];
    }
  }
  for(int i = 0; i < PANELN; i++){this->m_panel[i] = next[i];}
}

void cls_gamebrd::transparent(void){
  int next[PANELN];
  for(int y = 0; y < 4; y++){
    for(int x = 0; x < 4; x++){
      next[x*4+y] = this->m_panel[y*4+x];
    }
  }
  for(int i = 0; i < PANELN; i++){this->m_panel[i] = next[i];}
}

void cls_gamebrd::marge(void){
  this->coveron();
  for(int y = 0; y < 4; y++){
    for(int x = 0; x < 3; x++){
      if((this->m_panel[y*4+x]) && (this->m_panel[y*4+x] == this->m_panel[y*4+x+1])){
        this->m_panel[y*4+x]++;
        this->m_panel[y*4+x+1] = 0;
      }
    }
  }
  this->coveron();
}

#define MOVE_UP 0
#define MOVE_DOWN 1
#define MOVE_LEFT 2
#define MOVE_RIGHT 3
void cls_gamebrd::move(int d){
  switch(d){
    case MOVE_UP:
      this->transparent();
      break;
    case MOVE_DOWN:
      this->transparent();
      this->reverse();
      break;
    case MOVE_RIGHT:
      this->reverse();
      break;
  }

  this->marge();

  switch(d){
    case MOVE_UP:
      this->transparent();
      break;
    case MOVE_DOWN:
      this->reverse();
      this->transparent();
      break;
    case MOVE_RIGHT:
      this->reverse();
      break;
  }
  for(int i=0; i<PANELN; i++){
    this->drawPanel(i);
  }
}

const uint16_t ARROWFRCOLOR[] ={LIGHTGREY,YELLOW};
const uint16_t ARROWBGCOLOR[] ={DARKGREY,RED};
void cls_gamebrd::dispArrow(int d, int sts){
  T_ARROW * pArrowMatrix = &(this->m_arrowMatrix[d]);

  if(!sts){
    for(int i = 0; i < MATDTN; i++){
      pArrowMatrix->mat[i] = ARROWPATTERN[d][i];
    }
    this->m_anmArrowTime = 0;
  }
    
  for(int y = 0; y < MATDTN; y++){
    u_char c = 1;
    for(int x = 0; x < MATDTN; x++){
      uint16_t color = ARROWBGCOLOR[sts];
      if((pArrowMatrix->mat[y] & c)){color = ARROWFRCOLOR[sts];}
      M5.Lcd.fillRect(pArrowMatrix->pos[y*MATDTN+x].x, pArrowMatrix->pos[y*MATDTN+x].y, MATDTSZ, MATDTSZ, color);
      c <<= 1;
    }
  }  
}

void cls_gamebrd::changeArrow(int d){
 this->dispArrow(this->m_dirArrow,0);
  if(d == 1){
    if(this->m_dirArrow > 1){this->m_dirArrow-=2;}else{this->m_dirArrow ^= 1;}
  }else if(d == 2){
    if(this->m_dirArrow > 1){this->m_dirArrow ^= 1;}else{this->m_dirArrow+=2;}
  }
 this->dispArrow(this->m_dirArrow, 1);
 this->m_anmArrowTime = millis();
}

void cls_gamebrd::setArrow(int d){
  this->dispArrow(this->m_dirArrow,0);
  this->m_dirArrow = d; 
  this->dispArrow(this->m_dirArrow, 1);
  this->m_anmArrowTime = millis();
}

void cls_gamebrd::animationArrow(void){
    if( millis() - this->m_anmArrowTime < 250 ){return;}
    
    T_ARROW * pArrowMatrix = &(this->m_arrowMatrix[this->m_dirArrow]);
    u_char newpattern[MATDTN];    
    u_char c;    
    int i;

    for(i = 0; i<MATDTN; i++){newpattern[i] = pArrowMatrix->mat[i];}
    
    switch(this->m_dirArrow){
    case 0:
        c = newpattern[0];
        for(i = 0; i< MATDTN-1; i++){newpattern[i] = newpattern[i+1];}
        newpattern[MATDTN-1] = c;
        break;
    case 1:
        c = newpattern[MATDTN-1];
        for(i = 0; i< MATDTN-1; i++){newpattern[MATDTN-i-1] = newpattern[MATDTN-i-2];}
        newpattern[0] = c;
        break;
    case 2:
        for(i = 0; i< MATDTN; i++){
            c = newpattern[i];
            newpattern[i] &= 0xFE;
            newpattern[i]>>= 1;
            if(c & 1){newpattern[i]|=0x80;}
        }
        break;
    case 3:
        for(i = 0; i< MATDTN; i++){
            c = newpattern[i];
            newpattern[i] &= 0x7f;
            newpattern[i]<<= 1;
            if(c & 0x80){newpattern[i]|=1;}
        }
        break;
    }
    for(i=0; i<MATDTN; i++){pArrowMatrix->mat[i]=newpattern[i];}
    this->dispArrow(this->m_dirArrow, 1);
    this->m_anmArrowTime = millis();
}

void cls_gamebrd::gameEnd(int sts){
  int i,y = 10;
  int color[] = {BLACK,RED};
  const char *msg[]={"GAME OVER","GAME CLEAR!"};
  if(sts >= GAMEWIN){
    sts -= GAMEWIN;
    for(i = 0; i < 5; i++){
      this->m_panel[sts] = 0;
      this->drawPanel(sts);
      delay(200);
      this->m_panel[sts] = 11;
      this->drawPanel(sts);
      delay(200);
    }
    sts = 1;    
  }else{
    sts = 0;
  }
  while(y <= 120){
    M5.Lcd.fillRect(0,120-y,320,y*2,color[sts]);
    y += 10;
    delay(200);        
  }
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.drawCentreString(msg[sts], 160, 100, 4);
  M5.Lcd.drawCentreString("New Game -> Press[Centre]Button.", 160, 180, 2);
}

class cls_gyro{
private:
  unsigned long m_baset;
  int m_dispdir;
  int m_nowdir;
public:
  cls_gyro();
  int senseGyro();
};

cls_gyro::cls_gyro(){
  this->m_baset = 0;
  this->m_dispdir = 0;
  this->m_nowdir = 0;
}

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
 
int cls_gyro::senseGyro(void){
  int dir = 0; 
  M5.IMU.getAccelData(&accX, &accY, &accZ); 
  int ax = (int)(1000* accX);
  int ay = (int)(1000* accY);

  #define THRE 100
  #define THREUP 40
  #define THREDOWN 200
  if((ax > THRE) && (ay > -THREUP*1.5) && (ay < THREDOWN*1.5)){dir = 3;}
  else if((ax < -THRE) && (ay > -THREUP*1.5) && (ay < THREDOWN*1.5)){dir = 4;}
  if((ay < -THREUP) && (ax > -THRE) && (ax < THRE)){dir = 1;}
  else if((ay > THREDOWN) && (ax > -THRE) && (ax < THRE)){dir = 2;}

  unsigned long now = millis();
  
  if(dir == this->m_nowdir){
    if(now - this->m_baset > 180){
        this->m_dispdir = this->m_nowdir;
    }
  }else{
    this->m_nowdir = dir;
    this->m_baset = now;   
  }

  return this->m_dispdir;
}

void setup() {
  M5.begin();
  // for SD-Updater
  checkSDUpdater( SD, MENU_BIN, 5000 );
  M5.IMU.Init();
}

void loop() {
  M5.update(); // ここで状態更新

  int gamests = 0;
  int gyrosts = 0;

  cls_gamebrd * gamebrd = new cls_gamebrd;
  cls_gyro * gamegyro = new cls_gyro;

  gamebrd->drawInitBoard();
  gamebrd->appendTwo();
  gamebrd->appendTwo();

  while(!gamests){
    M5.update(); // 毎ループ先頭で更新
    gyrosts = gamegyro->senseGyro();
    if(gyrosts){
      gyrosts--;
      if(gamebrd->getDir() != gyrosts){
        gamebrd->setArrow(gyrosts);
      }
    }

    if(M5.BtnA.wasPressed()){
        gamebrd->changeArrow(1);
    }
    if(M5.BtnC.wasPressed()){
        gamebrd->changeArrow(2);
    }
    if(M5.BtnB.wasPressed()){
        gamebrd->move(gamebrd->getDir());
        gamests = gamebrd->appendTwo();    
    }
    gamebrd->animationArrow();
  }

  gamebrd->gameEnd(gamests);

  while(!M5.BtnB.wasPressed()){
      M5.update();
  }
  
  delete gamegyro;
  delete gamebrd;
}
