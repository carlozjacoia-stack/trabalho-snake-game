#include <LedControl.h>

typedef struct Snake Snake;
struct Snake{
  int head[2];     
  int body[40][2]; 
  int len;         
  int dir[2];      
};

typedef struct Apple Apple;
struct Apple{
  int rPos; 
  int cPos; 
};

//led Matrix
const int DIN =12;
const int CS =11;
const int CLK = 10;
LedControl lc = LedControl(DIN, CLK, CS,1);

const int varXPin = A3;//X do Joystick
const int varYPin = A4;//Y do Joystick

byte pic[8] = {0,0,0,0,0,0,0,0};

Snake snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};
Apple apple = {(int)random(0,8),(int)random(0,8)};


float oldTime = 0;
float timer = 0;
float updateRate = 3;

int tempoReinicio = 1000;
int i,j;//Counters

void setup() {
  Serial.begin(9600);
   lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);

  pinMode(varXPin, INPUT);
  pinMode(varYPin, INPUT);
}

void loop() {
    float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  int xVal = analogRead(varXPin);
  int yVal = analogRead(varYPin);
  
  if(xVal<100 && snake.dir[1]==0){
    snake.dir[0] = 0;
    snake.dir[1] = -1;
  }else if(xVal >920 && snake.dir[1]==0){
    snake.dir[0] = 0;
    snake.dir[1] = 1;
  }else if(yVal<100 && snake.dir[0]==0){
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  }else if(yVal >920 && snake.dir[0]==0){
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  }
  
  //Update
  if(timer > 1000/updateRate){
    timer = 0;
    Update();
  }
  
  //Render
  Render();
}


float calculateDeltaTime(){
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void reset(){
  for(int j=0;j<8;j++){
    pic[j] = 0;
  }
}
void gameOver(){

  // Acende todos os LEDs
  for(int i = 0; i < 8; i++){
    lc.setRow(0, i, B11111111);
  }
  
  // Mantém aceso pelo tempo definido
  delay(tempoReinicio);

  lc.clearDisplay(0);

  // Reinicia a cobra
  snake.head[0] = 1;
snake.head[1] = 5;

snake.body[0][0] = 0;
snake.body[0][1] = 5;

snake.body[1][0] = 1;
snake.body[1][1] = 5;

snake.len = 2;

snake.dir[0] = 1;
snake.dir[1] = 0;

  updateRate = 3;
  
  gerarMaca();

  // Reinicia os temporizadores
  timer = 0;
  oldTime = millis();

  reset();
}

void Update(){
  reset();
  int newHead[2] = {
    snake.head[0] + snake.dir[0],
    snake.head[1] + snake.dir[1]
  };

  // Verifica parede
  if(newHead[0] < 0 || newHead[0] > 7 ||
     newHead[1] < 0 || newHead[1] > 7){
    gameOver();
    return;
  }

  bool comeu = (newHead[0] == apple.rPos &&
                newHead[1] == apple.cPos);

  // ✅ Verifica colisão com o corpo ANTES de modificar o array
  // Se não comeu, o rabo vai sair — então ignora a posição [0]
  int inicioVerificacao = comeu ? 0 : 1;
  for(int j = inicioVerificacao; j < snake.len; j++){
    if(snake.body[j][0] == newHead[0] &&
       snake.body[j][1] == newHead[1]){
      gameOver();
      return;
    }
  }

  if(!comeu){
    removeFirst();
  } else {
    if(snake.len < 40) snake.len++;
    updateRate += 0.1;
    gerarMaca();
  }

  // Agora sim insere a nova cabeça
  snake.body[snake.len-1][0] = newHead[0];
  snake.body[snake.len-1][1] = newHead[1];
  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];

  // Renderiza body e maçã no buffer
  for(int j = 0; j < snake.len; j++){
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[apple.rPos] |= 128 >> apple.cPos;
}

void Render(){
  
   for(i=0;i<8;i++){
    lc.setRow(0,i,pic[i]);
   }
}

void removeFirst(){
  for(j=1;j<snake.len;j++){
    snake.body[j-1][0] = snake.body[j][0];
    snake.body[j-1][1] = snake.body[j][1];
  }
}

void gerarMaca() {
  bool dentroDaCobra;

  do {
    dentroDaCobra = false;

    apple.rPos = random(0,8);
    apple.cPos = random(0,8);

    for(int i = 0; i < snake.len; i++) {
      if(snake.body[i][0] == apple.rPos &&
         snake.body[i][1] == apple.cPos) {
        dentroDaCobra = true;
        break;
      }
    }

  } while(dentroDaCobra);
}
