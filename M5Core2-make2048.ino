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
  bool canMerge(void);
  void coveron(void);
  void reverse(void);
  void transparent(void);
  void marge(void);
  bool move(int);
  void dispArrow(int, int);
  void changeArrow(int);
  void setArrow(int);
  int getDir(void){return this->m_dirArrow;}
  void animationArrow(void);
  void drawPanelAt(int val, int px, int py);
  void animateMove(int d, int prev[]);
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

bool cls_gamebrd::canMerge(void){
  for(int y = 0; y < 4; y++){
    for(int x = 0; x < 4; x++){
      int v = this->m_panel[y*4+x];
      if(!v) return true;
      if(x < 3 && v == this->m_panel[y*4+x+1]) return true;
      if(y < 3 && v == this->m_panel[(y+1)*4+x]) return true;
    }
  }
  return false;
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
  if(!count){
    if(!this->canMerge()) return GAMELOSE;
    return 0;
  }
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
bool cls_gamebrd::move(int d){
  int prev[PANELN];
  for(int i = 0; i < PANELN; i++){ prev[i] = this->m_panel[i]; }

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

  bool changed = false;
  for(int i = 0; i < PANELN; i++){
    if(prev[i] != this->m_panel[i]){ changed = true; break; }
  }
  if(changed){
    this->animateMove(d, prev);
  }
  return changed;
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

void cls_gamebrd::drawPanelAt(int val, int px, int py){
  const char * numtxt[]={"0","2","4","8","16","32","64","128","256","512","1024","2048"};
  if(val < 0 || val > 11) return;
  M5.Lcd.fillRect(px, py, PANELSIZ, PANELSIZ, PNLCOLOR[val]);
  if(val){
    int fontid = 2;
    M5.Lcd.setTextColor(NUMCOLOR[val]);
    if(val < 10){fontid=4;}
    int fontdy = (PANELSIZ - M5.Lcd.fontHeight(fontid))/2;
    M5.Lcd.drawCentreString(numtxt[val], px + (PANELSIZ/2), py+fontdy, fontid);
  }
}

#define ANM_FRAMES 4
#define ANM_STEP ((PANELSIZ+BORDERSIZ)/ANM_FRAMES)  // 13px per frame
#define ANM_DELAY 25
void cls_gamebrd::animateMove(int d, int prev[]){
  int dx = 0, dy = 0;
  switch(d){
    case MOVE_UP:    dy = -1; break;
    case MOVE_DOWN:  dy =  1; break;
    case MOVE_LEFT:  dx = -1; break;
    case MOVE_RIGHT: dx =  1; break;
  }

  // Board area bounds (expanded to cover slide overflow)
  int bx = this->m_panelpos[0].x;
  int by = this->m_panelpos[0].y;
  int bw = 4*(PANELSIZ+BORDERSIZ);
  int bh = 4*(PANELSIZ+BORDERSIZ);
  int cx = bx, cy = by, cw = bw, ch = bh;
  if(dx < 0){ cx -= PANELSIZ; cw += PANELSIZ; }
  if(dx > 0){ cw += PANELSIZ; }
  if(dy < 0){ cy -= PANELSIZ; ch += PANELSIZ; }
  if(dy > 0){ ch += PANELSIZ; }

  for(int f = 1; f <= ANM_FRAMES; f++){
    // Clear expanded area
    M5.Lcd.fillRect(cx, cy, cw, ch, BGCOLOR);

    // Draw each old tile at offset position
    int offset = f * ANM_STEP;
    for(int i = 0; i < PANELN; i++){
      if(prev[i]){
        int px = this->m_panelpos[i].x + dx * offset;
        int py = this->m_panelpos[i].y + dy * offset;
        // Clip to board area
        if(px >= bx - PANELSIZ && px < bx + bw && py >= by - PANELSIZ && py < by + bh){
          this->drawPanelAt(prev[i], px, py);
        }
      }
    }
    delay(ANM_DELAY);
  }

  // Draw final state
  for(int i = 0; i < PANELN; i++){
    this->drawPanel(i);
  }
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

// Flick detection
bool touch_active = false;
int touch_sx = 0, touch_sy = 0;
int touch_ex = 0, touch_ey = 0;  // last known position while touching
#define FLICK_THRESHOLD 50
 
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
        if(gamebrd->move(gamebrd->getDir())){
            gamests = gamebrd->appendTwo();
        }
    }

    // Flick detection on screen (y < 225 to avoid button area)
    if(M5.Touch.points > 0 && M5.Touch.point[0].y < 225){
      if(!touch_active){
        touch_active = true;
        touch_sx = M5.Touch.point[0].x;
        touch_sy = M5.Touch.point[0].y;
      }
      // Always track last known position while touching
      touch_ex = M5.Touch.point[0].x;
      touch_ey = M5.Touch.point[0].y;
    } else if(touch_active){
      touch_active = false;
      int fdx = touch_ex - touch_sx;
      int fdy = touch_ey - touch_sy;
      int absx = fdx < 0 ? -fdx : fdx;
      int absy = fdy < 0 ? -fdy : fdy;
      if(absx > FLICK_THRESHOLD || absy > FLICK_THRESHOLD){
        int flickDir;
        if(absx > absy){
          flickDir = (fdx > 0) ? MOVE_RIGHT : MOVE_LEFT;
        } else {
          flickDir = (fdy > 0) ? MOVE_DOWN : MOVE_UP;
        }
        gamebrd->setArrow(flickDir);
        if(gamebrd->move(flickDir)){
          gamests = gamebrd->appendTwo();
        }
      }
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
