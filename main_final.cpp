#include "iGraphics.h"
#include "iSound.h"
#include <windows.h>
#include<stdio.h>
#include<stdio.h>

#define NAME_MAX 13
char playerName[NAME_MAX + 1] = "";
int  nameLen   = 0;         

Image shipFrames[10], spacebg,namebg;
Sprite ship;
double bullet_x[1000], bullet_y[1000];
int totalbullet = 0;
double asteroid_x[1000], asteroid_y[1000];
int totalasteroid = 0;
#define STAR_COUNT 100
int star_x[STAR_COUNT], star_y[STAR_COUNT];
int ship_x = 300, ship_y = 50;
int beethovenChannel = -1;
static bool lbwritten=false;  
enum GameState { MENU, PLAY, GAMEOVER, SETTINGS, SOUND_MENU, HELP_MENU, pMENU,HOME, LEVEL_MENU, ENTER_NAME,LEADERBOARD,WIN_GAME};
GameState current_state = HOME;
GameState previous_state = MENU;

//enemy things
typedef struct enemy{
    double x,y;
    bool alive;
    double bullet_x[50],bullet_y[50];
    int total_bullets;
}enemy;

enemy enemies[2000];
int total_enemies=0;
bool enemy_mode=false;


//level adding
int level=1;
int score=0;
int stage_values[5]={200,400,600,800,1000};
float asteroidspeed=5;
int stage=1;


//glowup animation
bool glow_level[6]={false};
bool glow_main[4]={false};
bool glowSettings[2]={false};
bool glow_sound[2]={false};
bool glow_help=false;
bool glow_lead=false;
bool glow_hmenu_back=false;

//blasting animations
void add_blast(double x, double y);
void update_blast();
void load_blast_resources();
void updateblastframes();
void blasting_sound(){
    iInitializeSound();
    iPlaySound("assets/sounds/explosionsound.wav",0);
}

//to add life of the ship
int life=4;
bool recently_hit=false;
int hit_timer=0;
void update_hittimer(){
    if(recently_hit){
        hit_timer--;
        if(hit_timer<=0){
            recently_hit=false;
            hit_timer=0;
        }
    }
}
int leaderboard(const char *name, int score);
void apply_damage();

//drawing functions
void draw_option();
void draw_option_pMENU();
void draw_gameover_options();
void draw_LEVEL_MENU();
void stage_update();
void generate_asteroid(){
    if (current_state != PLAY || enemy_mode) return;
    if(totalasteroid<1000){
        asteroid_x[totalasteroid]=rand()%(700-64);
        asteroid_y[totalasteroid]=800;
        totalasteroid++;
    }
}
void move_asteroid(){
    if (current_state != PLAY) return;
    for(int i=0;i<totalasteroid;i++){
        asteroid_y[i]-=asteroidspeed;
        if(asteroid_y[i]<0){
            for(int j=i;j<totalasteroid-1;j++){
                asteroid_x[j]=asteroid_x[j+1];
                asteroid_y[j]=asteroid_y[j+1];
            }
            --totalasteroid;
            --i;
        }
    }
}
void generate_bullet(){
    if (current_state != PLAY) return;
    iInitializeSound();
    iPlaySound("assets/sounds/bulletsound.wav",0);
    if(totalbullet<1000){
        bullet_x[totalbullet]=ship.x+40;
        bullet_y[totalbullet]=ship.y+64;
        totalbullet++;
    }
}
void move_bullet(){
    if (current_state != PLAY) return;
    for(int i=0;i<totalbullet;i++){
        bullet_y[i]+=5;
        if(bullet_y[i]>800){
            for(int j=i;j<totalbullet-1;j++){
                bullet_x[j]=bullet_x[j+1];
                bullet_y[j]=bullet_y[j+1];
            }
            --totalbullet;
            --i;
        }
    }
}
void collision_checker(){
    if (current_state != PLAY) return;
    for(int i=0;i<totalbullet;i++){
        for(int j=0;j<totalasteroid;j++){
            if(bullet_x[i]>asteroid_x[j] && bullet_x[i]<asteroid_x[j]+64 && bullet_y[i]>asteroid_y[j] && bullet_y[i]<asteroid_y[j]+64){
                add_blast(asteroid_x[j],asteroid_y[j]);
                for(int k=i;k<totalbullet-1;k++){
                bullet_x[k]=bullet_x[k+1];
                bullet_y[k]=bullet_y[k+1];
                }
            --totalbullet;
            --i;
            for(int k=j;k<totalasteroid-1;k++){
                asteroid_x[k]=asteroid_x[k+1];
                asteroid_y[k]=asteroid_y[k+1];
            }
            asteroid_x[totalasteroid]=-1000;
            asteroid_y[totalasteroid]=-1000;
            --totalasteroid;
            score+=10;
            break;
            }
        }
        for(int j=0;j<total_enemies;j++){
                if(!enemies[j].alive) continue;
                if (bullet_x[i]>enemies[j].x && bullet_x[i]<enemies[j].x+64 && bullet_y[i]>enemies[j].y && bullet_y[i]<enemies[j].y+64) {
                add_blast(enemies[j].x,enemies[j].y);
                enemies[j].alive=false;
                score+=10;
                //removing bullets
                for(int k=i;k<totalbullet-1;k++){
                    bullet_x[k]=bullet_x[k+1];
                    bullet_y[k]=bullet_y[k+1];
                }
                totalbullet--;
                i--;
                break;
            }
        }

    }
    if(!recently_hit){

        bool collision_detected=false;

        for(int j=0;j<totalasteroid;j++){
        if(((asteroid_x[j]>ship.x && asteroid_x[j]<ship.x+80) || (asteroid_x[j]+64>ship.x && asteroid_x[j]+64<ship.x+80)) && 
            asteroid_y[j]<ship.y+64){
            add_blast(asteroid_x[j],asteroid_y[j]);
            //removing asteroids when collision happens
            for(int k=j;k<totalasteroid;k++){
                asteroid_x[k]=asteroid_x[k+1];
                asteroid_y[k]=asteroid_y[k+1];
            }
            asteroid_x[totalasteroid]=-1000;
            asteroid_y[totalasteroid]=-1000;
            totalasteroid--;
            collision_detected=true;
            apply_damage();
            return;
            }

        }

        for(int j=0;j<total_enemies;j++){
        if(((enemies[j].x>ship.x && enemies[j].x<ship.x+80) || (enemies[j].x+64>ship.x && enemies[j].x+64<ship.x+80)) 
            && enemies[j].y<ship.y+64){
            add_blast(enemies[j].x,enemies[j].y);
            //removes collisioned enemies
            enemies[j].alive=false;
            enemies[j].x=-1000;
            enemies[j].y=-1000;
            collision_detected=true;
            apply_damage();
            return;
            }
        }
        for(int i=0;i<total_enemies;i++){
            if(!enemies[i].alive) continue;
            for(int j=0;j<enemies[i].total_bullets;j++){
                if (enemies[i].bullet_x[j]>=0 && enemies[i].bullet_x[j]>ship.x && enemies[i].bullet_x[j]<ship.x+80 && enemies[i].bullet_y[j]>=0 && enemies[i].bullet_y[j]>ship.y && enemies[i].bullet_y[j]<ship.y+64){
                //removing bullets when collision
                double cox=enemies[i].bullet_x[j],coy=enemies[i].bullet_y[j];
                for(int k=j;k<enemies[i].total_bullets;k++){
                    enemies[i].bullet_x[k]=enemies[i].bullet_x[k+1];
                    enemies[i].bullet_y[k]=enemies[i].bullet_y[k+1];
                }
                enemies[i].bullet_x[enemies[i].total_bullets]=-1000;
                enemies[i].bullet_y[enemies[i].total_bullets]=-1000;
                enemies[i].total_bullets--;
                j--;
                add_blast(cox-5,coy);
                collision_detected=true;
                apply_damage();
                return;
                }
            }
            if(collision_detected) break;
        }

    }

}

void apply_damage(){
    if(!recently_hit){
        life--;
        recently_hit=true;
        hit_timer=50;
        if(life<=0){
            iInitializeSound();
            iPlaySound("assets/sounds/gameoversound.wav",0);
            current_state=GAMEOVER;
        }
    }
}

void generate_enemy(){
    if(current_state!=PLAY || !enemy_mode) return;
    if(total_enemies<2000){
        enemies[total_enemies].x=rand()%(700-64);
        enemies[total_enemies].y=800;
        enemies[total_enemies].total_bullets=0;
        enemies[total_enemies].alive=true;
        total_enemies++;
    }
}

void move_enemies(){
    if(current_state!=PLAY || !enemy_mode) return;
    for(int i=0;i<total_enemies;i++){
        if(!enemies[i].alive) continue;
        enemies[i].y-=asteroidspeed;

        //generate bullet
        if (rand()%100<3 && enemies[i].total_bullets<50){
            enemies[i].bullet_x[enemies[i].total_bullets]=enemies[i].x+32;
            enemies[i].bullet_y[enemies[i].total_bullets]=enemies[i].y+5;
            enemies[i].total_bullets++;
        }

        //move bullet
        for(int j=0;j<enemies[i].total_bullets;j++){
            enemies[i].bullet_y[j]-=(5+asteroidspeed);

            //remove bullets from screen if goes beyong limit/range
            if(enemies[i].bullet_y[j]<0){
                for(int k=j;k<enemies[i].total_bullets;k++){
                    enemies[i].bullet_x[k]=enemies[i].bullet_x[k+1];
                    enemies[i].bullet_y[k]=enemies[i].bullet_y[k+1];
                }
                enemies[i].total_bullets--;
                j--;
            }
        }
        if(enemies[i].y<0) {
        enemies[i].alive=false;
        enemies[i].x=-1000;
        enemies[i].y=-1000;
        }
    }
}

//for blasting animation

#define BLAST_FRAMES 24
Image blastframes[BLAST_FRAMES];

void load_blast_resources(){
    for(int i=0;i<BLAST_FRAMES;i++){
        char path[100];
        sprintf(path,"assets/images/blasts/tile%d.png",i);
        iLoadImage(&blastframes[i],path);
    }
}

//blast structure

typedef struct{
    double x,y;
    int frame;
    bool active;
}BLAST;

BLAST blasts[100];

void add_blast(double x, double y){
    for(int i=0;i<100;i++){
        if(!blasts[i].active){
            blasts[i].x=x;
            blasts[i].y=y;
            blasts[i].frame=0;
            blasts[i].active=true;
            blasting_sound();
            break;
        }
    }
}

void update_blast(){
    for(int i=0;i<100;i++){
        if(blasts[i].active){
            blasts[i].frame++;
            if(blasts[i].frame>=BLAST_FRAMES){
                blasts[i].active=false;
                blasts[i].frame=0;
            }
        }
    }
}

void updateblastframes(){
    for(int i=0;i<100;i++){
        if(blasts[i].active){
            blasts[i].frame++;
            if(blasts[i].frame>=BLAST_FRAMES){
                blasts[i].active=false;
                blasts[i].frame=0;
            }
        }
    }
}

void reset(){
    score=0;
    totalbullet=0;
    totalasteroid=0;
    ship_x=300;
    ship_y=50;
    iSetSpritePosition(&ship, ship_x, ship_y);
    total_enemies=0;
    enemy_mode=false;
    stage=1;
    level=1;
    life=4;
    recently_hit=false;
    hit_timer=0;

    for(int i=0;i<100;i++) {
        enemies[i].alive=false;
        enemies[i].total_bullets=0;
        for(int j=0;j<50;j++){
        enemies[i].bullet_x[j]=-100;
        enemies[i].bullet_y[j]=-100;
        }
    }

    for(int i=0;i<100;i++){
        blasts[i].active = false;
        blasts[i].frame = 0;
    }

    current_state=PLAY;
    lbwritten=false;

}

void glow(){
    iSetColor(100,0,0);
}

void iAnim(){
    if (current_state != PLAY) return;
    iAnimateSprite(&ship);
}
void loadresources(){
    iInitSprite(&ship,-1);
   for(int i=0;i<STAR_COUNT;i++){
        star_x[i]=rand()%700;
        star_y[i]=rand()%800;
    }
    //iLoadImage(&spacebg, "assets/images/spacebg1-1.jpg");
    iLoadFramesFromFolder(shipFrames,"assets/images/flyship");
    iChangeSpriteFrames(&ship,shipFrames,1);
    iSetSpritePosition(&ship,ship_x,ship_y);
    iLoadImage(&spacebg, "assets/images/spacebg2.1.jpg");
    iLoadImage(&namebg,"assets/images/name.jpg");
    load_blast_resources();
}

void iDraw() {
    
    iClear();
   // iSetColor(255,255,0);
  // printf("STATE: %d\n", current_state); 
  //  iShowImage(0, 0,"assets/images/spacebg1.1.jpg");

    switch (current_state) {
        case HOME:
            iShowImage(0,0,"assets/images/homebg.jpg");
             iSetColor(255, 255, 255);
    iTextBold(250, 100, "Press SPACE to enter", GLUT_BITMAP_HELVETICA_18);
            break;
        case MENU:
     //   iShowImage(0, 0, "spacebg1.1.jpg");
     iShowImage(0, 0,"assets/images/spacebg1.1.jpg");
                draw_option();
                break;
        case pMENU:
     iShowImage(0, 0,"assets/images/spacebg1.1.jpg");

            draw_option_pMENU();
                break;
        
      //play state
      case ENTER_NAME:
        {
    iShowLoadedImage(0, 0, &namebg);

    
    int barX = 142, barY = 262;
    int barWidth = 420;
    int name_box_center_x = barX + barWidth / 2;

    int char_width = 12; 
    int text_width = nameLen * char_width;

    int name_start_x = name_box_center_x - text_width / 2;

    iSetColor(173, 216, 230);
    iTextBold(name_start_x, barY + 38, playerName, GLUT_BITMAP_TIMES_ROMAN_24);
    break;
}
      case PLAY:
           iShowLoadedImage(0, 0, &spacebg);
            iSetColor(255,255,255);
    for(int i=0;i<STAR_COUNT;i++){
        iFilledCircle(star_x[i],star_y[i],2,1000);
    }
    if(!recently_hit || (hit_timer/5)%2 == 0)
    iShowSprite(&ship);
    iSetColor(255,0,0);
    for(int i=0;i<totalbullet;i++){
        iFilledRectangle(bullet_x[i],bullet_y[i],4,10);
    }
    
    for(int i=0;i<totalasteroid;i++){
        iShowImage(asteroid_x[i],asteroid_y[i],"assets/images/Asteroid-A-10-50.png");
    }
    if(enemy_mode) {
        for (int i=0;i<total_enemies;i++) {
        if (!enemies[i].alive) continue;
        iShowImage(enemies[i].x, enemies[i].y, "assets/images/enemy_ship.png");
        for (int j=0;j<enemies[i].total_bullets;j++){
            iSetColor(255,255,0);
            iFilledRectangle(enemies[i].bullet_x[j], enemies[i].bullet_y[j],4,8);
        }
      }
    }
    char score_text[50];
    sprintf(score_text,"SCORE : %d",score);
    char life_text[20];
    sprintf(life_text,"LIFE : ");
    for(int i=0;i<life;i++){
        iSetColor(0,255,0);
        iFilledRectangle(660-i*20,770,15,15);
        iSetColor(255,255,255);
        iRectangle(660-i*20,770,15,15);
    }
    iSetColor(255,255,255);
    iTextBold(10,770,score_text,GLUT_BITMAP_TIMES_ROMAN_24);
    stage_update();
    if(score>=1500) {
        iInitializeSound();
        iPlaySound("assets/sounds/gamewinsound.wav",0);
        current_state=WIN_GAME; // edited here 
    }

    for(int i=0;i<100;i++){
        if(blasts[i].active){
            if(blasts[i].frame < BLAST_FRAMES)
            iShowLoadedImage(blasts[i].x, blasts[i].y, &blastframes[blasts[i].frame]);
        }
    }

    break;

    //help menu 

      case HELP_MENU:
    iClear();
    
    iShowImage(0, 0, "assets/images/space_help_scroll.jpg");

    iSetColor(173,216,230);
    if(glow_hmenu_back) glow();
    iFilledRectangle(50,600,50,50);
    iSetColor(0,51,150);
    iTextBold(60,620,"BACK");
    // Title
    iSetColor(255,0,0);
    iTextBold(210, 610, "Space Shooter – Help & Controls");

    // Objective
    iSetColor(0, 0, 255);
    iText(170, 580, "Objective:", GLUT_BITMAP_HELVETICA_12);

    iSetColor(0, 0, 0);
    iText(180, 560, "• Pilot your spaceship and shoot down", GLUT_BITMAP_HELVETICA_12);
    iText(200, 545, "  incoming asteroids.", GLUT_BITMAP_HELVETICA_12);
    iText(180, 525, "• Each asteroid gives you +10 points", GLUT_BITMAP_HELVETICA_12);
    iText(200, 510, "  when destroyed.", GLUT_BITMAP_HELVETICA_12);
    iText(180, 490, "• Reach 100 points to clear the level.", GLUT_BITMAP_HELVETICA_12);

    // Controls
    iSetColor(0, 0, 255);
    iText(170, 465, "Controls:", GLUT_BITMAP_HELVETICA_12);

    iSetColor(0, 0, 0);
    iText(180, 445, "• A / Left Arrow     - Move Left", GLUT_BITMAP_HELVETICA_12);
    iText(180, 425, "• D / Right Arrow    - Move Right", GLUT_BITMAP_HELVETICA_12);
    iText(180, 405, "• SPACE or Click     - Shoot bullet", GLUT_BITMAP_HELVETICA_12);
    iText(180, 385, "• ESC                - Pause the game", GLUT_BITMAP_HELVETICA_12);
    iText(180, 365, "• M                  - Return to Menu", GLUT_BITMAP_HELVETICA_12);

    // Gameplay
    iSetColor(0, 0, 255);
    iText(170, 340, "Gameplay:", GLUT_BITMAP_HELVETICA_12);

    iSetColor(0, 0, 0);
    iText(180, 320, "• Asteroids fall from the top.", GLUT_BITMAP_HELVETICA_12);
    iText(180, 300, "• Shoot them or dodge them.", GLUT_BITMAP_HELVETICA_12);
    iText(180, 280, "• If an asteroid hits your ship,", GLUT_BITMAP_HELVETICA_12);
    iText(200, 265, "  it's game over.", GLUT_BITMAP_HELVETICA_12);
    iText(180, 245, "• Speed increases every 100 points.", GLUT_BITMAP_HELVETICA_12);

    // Footer
    iText(180, 220, "Press 'M' to return to the Main Menu.", GLUT_BITMAP_HELVETICA_12);
    break;


       // case MENU:
         //   draw_option();
           // break;

        case SETTINGS:
            iClear();
            iShowImage(0, 0,"assets/images/spacebg1.1.jpg");
            iSetColor(0, 0, 0);
            if(glowSettings[0]) glow();
            iFilledRectangle(250, 460, 250, 30);
            iSetColor(250, 250, 250);
            iText(340, 470, "SOUND",GLUT_BITMAP_HELVETICA_18);
            iSetColor(0, 0, 0);
            if(glowSettings[1]) glow();
            iFilledRectangle(250, 335, 250, 30);
            iSetColor(250, 250, 250);
            iText(350, 345, "BACK",GLUT_BITMAP_HELVETICA_18);
            break;

        case SOUND_MENU:
            iClear();
            iShowImage(0, 0,"assets/images/spacebg1.1.jpg");
            iSetColor(0, 0, 0);
            if(glow_sound[0]) glow();
            iFilledRectangle(250, 460, 250, 30);
            iSetColor(250, 250, 250);
            iText(355, 470, "ON",GLUT_BITMAP_HELVETICA_18);
            iSetColor(0, 0, 0);
            if(glow_sound[1]) glow();
            iFilledRectangle(250, 335, 250, 30);
            iSetColor(250, 250, 250);
            iText(355, 345, "OFF",GLUT_BITMAP_HELVETICA_18);
            break;
        case GAMEOVER:
            iClear();
             iShowImage(0, 0,"assets/images/spacebg3.1.jpg");
            draw_gameover_options();
            //leaderboard(playerName,score);
            if (!lbwritten && playerName[0]) {           
        leaderboard(playerName, score);          
        lbwritten=true;
         }
            break;
        case LEVEL_MENU:
            iClear();
            iShowImage(0, 0,"assets/images/spacebg1.1.jpg");
            draw_LEVEL_MENU();
            break;
        case LEADERBOARD:{
            iClear();
            iShowImage(0,0,"assets/images/leaderboard.png");
            iSetColor(0,255,255);
            iTextBold(140,70,"PRESS 'B' TO RETURN TO MAIN MENU",GLUT_BITMAP_TIMES_ROMAN_24);
            int i=1;
            int y=560-65;
            int score;
            char name[33];
            FILE *fp=fopen("leaderboard.txt","r");
            
            if(fp==NULL)
            {
                perror("leaderboard is problematic");
                return ;
            }
            for(i=1;i<=10&&fscanf(fp, " %33[^:]:%d", name, &score) == 2;i++,y=y-38)
            {
                iText(170, y, name, GLUT_BITMAP_TIMES_ROMAN_24);
                char buf[16];
                sprintf(buf,"%d",score);
                iText(460,y,buf, GLUT_BITMAP_TIMES_ROMAN_24);
            }
            fclose(fp);
            break;}
        case WIN_GAME:
        {   
           iClear();
            iShowImage(0,0,"assets/images/win_game.png");
            iSetColor(0,128,128);
            iTextBold(190,190,"PRESS W TO RETURN TO MENU", GLUT_BITMAP_TIMES_ROMAN_24);
            if (!lbwritten && playerName[0]) {           
        leaderboard(playerName, score);          
        lbwritten=true;}
           // reset();
            break;
        }
        default:
            break;
    }
}

int speed[5]={5,7,9,11,13};

void stage_update(){
    if(score >= 100) enemy_mode=true;
    for(int i=0;i<5;i++){
        if(score<stage_values[i]){
            stage=i+1;
            break;
        }
    }
    asteroidspeed=speed[level-1]+1.5*(stage-1);
}

void mouse_click(){
    iInitializeSound();
    iPlaySound("assets/sounds/clicksound.wav",0);
    return;
}

void draw_LEVEL_MENU(){
        iSetColor(255,0,0);
        iText(280,650,"SELECT A LEVEL",GLUT_BITMAP_TIMES_ROMAN_24);

        //level texts

        iSetColor(0,0,0);
        if(glow_level[0]) glow();
        iFilledRectangle(250,585,250,30);
        iSetColor(250,250,250);
        iText(345,595,"LEVEL 1",GLUT_BITMAP_HELVETICA_18);

        iSetColor(0,0,0);
        if(glow_level[1]) glow();
        iFilledRectangle(250,500,250,30);
        iSetColor(250,250,250);
        iText(345,510,"LEVEL 2",GLUT_BITMAP_HELVETICA_18);

        iSetColor(0,0,0);
        if(glow_level[2]) glow();
        iFilledRectangle(250,415,250,30);
        iSetColor(250,250,250);
        iText(345,425,"LEVEL 3",GLUT_BITMAP_HELVETICA_18);

        iSetColor(0,0,0);
        if(glow_level[3]) glow();
        iFilledRectangle(250,330,250,30);
        iSetColor(250,250,250);
        iText(345,340,"LEVEL 4",GLUT_BITMAP_HELVETICA_18);

        iSetColor(0,0,0);
        if(glow_level[4]) glow();
        iFilledRectangle(250,245,250,30);
        iSetColor(250,250,250);
        iText(345,255,"LEVEL 5",GLUT_BITMAP_HELVETICA_18);

        iSetColor(0,0,0);
        if(glow_level[5]) glow();
        iFilledRectangle(250,160,250,30);
        iSetColor(250,250,250);
        iText(350,170,"BACK",GLUT_BITMAP_HELVETICA_18);

        return;
}

void draw_option() {
    iSetColor(0,0,0);
    if(glow_lead) glow();
    iFilledRectangle(50,600,100,50);
    iSetColor(255,255,255);
    iTextBold(57,620,"LEADERBOARD");
    iSetColor(0,0,0);
    if(glow_main[0]) glow();
    iFilledRectangle(250, 585, 250, 30);
    iSetColor(250, 250, 250);
    iText(350, 595, "PLAY",GLUT_BITMAP_HELVETICA_18);

    iSetColor(0,0,0);
    if(glow_main[1]) glow();
    iFilledRectangle(250, 460, 250, 30);
    iSetColor(250, 250, 250);
    iText(350, 470, "HELP",GLUT_BITMAP_HELVETICA_18);

    iSetColor(0,0,0);
    if(glow_main[2]) glow();
    iFilledRectangle(250, 335, 250, 30);
    iSetColor(250, 250, 250);
    iText(330, 345, "SETTINGS",GLUT_BITMAP_HELVETICA_18);

    iSetColor(0,0,0);
    if(glow_main[3]) glow();
    iFilledRectangle(250, 210, 250, 30);
    iSetColor(250, 250, 250);
    iText(350, 220, "QUIT",GLUT_BITMAP_HELVETICA_18);
}
void draw_option_pMENU() {
    iSetColor(0, 0, 0);
    if(glow_main[0]) glow();
    iFilledRectangle(250, 585, 250, 30);
    iSetColor(250, 250, 250);
    iText(325, 595, "CONTINUE",GLUT_BITMAP_HELVETICA_18);

    iSetColor(0, 0, 0);
    if(glow_main[1]) glow();
    iFilledRectangle(250, 460, 250, 30);
    iSetColor(250, 250, 250);
    iText(350, 470, "HELP",GLUT_BITMAP_HELVETICA_18);

    iSetColor(0, 0, 0);
    if(glow_main[2]) glow();
    iFilledRectangle(250, 335, 250, 30);
    iSetColor(250, 250, 250);
    iText(330, 345, "SETTINGS",GLUT_BITMAP_HELVETICA_18);

    iSetColor(0, 0, 0);
    if(glow_main[3]) glow();
    iFilledRectangle(250, 210, 250, 30);
    iSetColor(250, 250, 250);
    iText(340, 220, "RETURN",GLUT_BITMAP_HELVETICA_18);
}




void draw_gameover_options(){
       
   // iClear();
          //  iShowImage(0, 0,"assets/images/spacebg3.1.jpg");

    // Red translucent overlay
   

    // Main GAME OVER text
    iSetColor(255, 255, 255);

   /* if(score>=1500){
        //iClear();
            iShowImage(0,0,"assets/images/win_game.png");
            iSetColor(0,128,128);
            iTextBold(190,190,"PRESS W TO RETURN TO MENU", GLUT_BITMAP_TIMES_ROMAN_24);
            
    }*/
   // else{
    iTextBold(273, 520, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
    
    // Show final score
    char msg[50];
    sprintf(msg, "Final Score: %d", score);
    iText(275, 460, msg,GLUT_BITMAP_TIMES_ROMAN_24);

    // Instructions to restart or go back
    iText(100, 400, "Press 'R' to Restart or 'M' to return to Main Menu",GLUT_BITMAP_TIMES_ROMAN_24);
   // }
    return;

}
        

void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (current_state == MENU) {
            if (mx > 250 && mx < 500 && my > 585 && my < 615)
                {
                mouse_click();
                current_state = LEVEL_MENU;
                previous_state = MENU;}
            else if (mx > 250 && mx < 500 && my > 460 && my < 490)
                {
                mouse_click();
                current_state = HELP_MENU;
                previous_state = MENU;}
            else if (mx > 250 && mx < 500 && my > 335 && my < 365){
                mouse_click();
                current_state = SETTINGS;
                previous_state = MENU;}
            else if (mx > 250 && mx < 500 && my > 210 && my < 240)
                {mouse_click();
                exit(0);}
                
    
            else if(mx>50&&mx<150&&my>600 && my<650)
                {   
                    mouse_click();
                    current_state=LEADERBOARD;
                    previous_state=MENU;
                }
    
        }
        else if(current_state == HELP_MENU)
        {
            if(mx>50 && mx<110 && my>600&& my<650)
            {
               if(previous_state==MENU)
                {
                    mouse_click();
                    current_state=MENU;
                previous_state=HELP_MENU;
                }
                if(previous_state==pMENU)
                {
                    mouse_click();
                    current_state=pMENU;
                    previous_state=HELP_MENU;
                }

            }
        }
        
        else if (current_state == pMENU) {
            if (mx > 250 && mx < 500 && my > 585 && my < 615)
                {mouse_click();
                current_state = PLAY;
                previous_state = pMENU;}
            else if (mx > 250 && mx < 500 && my > 460 && my < 490)
                {mouse_click();
                current_state = HELP_MENU;
                previous_state = pMENU;}
            else if (mx > 250 && mx < 500 && my > 335 && my < 365)
                {mouse_click();
                current_state = SETTINGS;
                previous_state = pMENU;}
            else if (mx > 250 && mx < 500 && my > 210 && my < 240)
                {   mouse_click();
                    score=0;
                     totalbullet=0;
                      totalasteroid=0;
                     ship_x=300; ship_y=50;
                     iSetSpritePosition(&ship, ship_x, ship_y);
                     current_state=MENU;
                     previous_state = pMENU;
                }
        }
        else if (current_state == SETTINGS) {
            if (mx > 250 && mx < 500 && my > 460 && my < 490)
                {mouse_click();
                current_state = SOUND_MENU;}
            else if (mx > 250 && mx < 500 && my > 335 && my < 365)
               // current_state = MENU;
                {   mouse_click();
                    if(previous_state==MENU || previous_state==pMENU || previous_state==PLAY){
                        current_state=previous_state;
                    }
                    else current_state=MENU;
                }
        }
        else if (current_state == SOUND_MENU) {
            if (mx > 250 && mx < 500 && my > 460 && my < 490)
                {mouse_click();
                iSetVolume(beethovenChannel, 100);}
            else if (mx > 250 && mx < 500 && my > 335 && my < 365)
               { mouse_click();
                iSetVolume(beethovenChannel, 0);}
            //current_state = MENU;
            current_state=previous_state;
        }
        else if(current_state== PLAY){
            generate_bullet();
        }
        else if(current_state==LEVEL_MENU){
            if(mx>250 && mx<500 && my>585 && my<615){
                mouse_click();
                level=1; score=0; asteroidspeed=5; stage=1;

                current_state=PLAY;
            }
            else if(mx>250 && mx<500 && my>500 && my<530){
                mouse_click();
                level=2; score=0; asteroidspeed=7; stage=1;
                current_state=PLAY;
            }
            else if(mx>250 && mx<500 && my>415 && my<445){
                mouse_click();
                level=3; score=0; asteroidspeed=9; stage=1;
                current_state=PLAY;
            }
            else if(mx>250 && mx<500 && my>330 && my<360){
                mouse_click();
                level=4; score=0; asteroidspeed=11; stage=1;
                current_state=PLAY;
            }
            else if(mx>250 && mx<500 && my>245 && my<275){
                mouse_click();
                level=5; score=0; asteroidspeed=13; stage=1;
                current_state=PLAY;
            }
            else if(mx>250 && mx<500 && my>160 && my<190){
                mouse_click();
                current_state=MENU;
            }
        }
    }
}

void iMouseMove(int mx, int my) {
        for(int i=0;i<6;i++) glow_level[i]=false;
        for(int i=0;i<4;i++) glow_main[i]=false;
        for(int i=0;i<2;i++) glowSettings[i]=false;
        for(int i=0;i<2;i++) glow_sound[i]=false;
        glow_hmenu_back=false;
        glow_lead=false;

    if(current_state==LEVEL_MENU){
        if(mx>250 && mx<500) {
            if(my>585 && my<615) glow_level[0]=true;
            else if(my>500 && my<530) glow_level[1]=true;
            else if(my>415 && my<445) glow_level[2]=true;
            else if(my>330 && my<360) glow_level[3]=true;
            else if(my>245 && my<275) glow_level[4]=true;
            else if(my>160 && my<190) glow_level[5]=true;
        }
    }
    else if(current_state==MENU || current_state==pMENU){
        if(mx>250 && mx<500){
            if(my>585 && my<615) glow_main[0]=true;
            else if(my>460 && my<490) glow_main[1]=true;
            else if(my>335 && my<365) glow_main[2]=true;
            else if(my>210 && my<240) glow_main[3]=true;
        }
        else if(mx>50 && mx<150 && my>600 && my<650) 
        glow_lead=true;
    }
    else if(current_state==SETTINGS){
        if(mx>250 && mx<500){
            if(my>460 && my<490) glowSettings[0]=true;
            else if(my>335 && my<365) glowSettings[1]=true;
        }
    }
    else if(current_state==SOUND_MENU){
        if(mx>250 && mx<500){
            if(my>460 && my<490) glow_sound[0]=true;
            else if(my>335 && my<365) glow_sound[1]=true;
        }
    }
    else if(current_state == HELP_MENU && mx > 50 && mx < 100 && my > 600 && my < 650) 
    glow_hmenu_back=true;

}
void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my) {}
void move_stars(){
    for(int i=0;i<STAR_COUNT;i++){
        star_y[i]-=1;
        if(star_y[i]<0){
            star_y[i]=800;
            star_x[i]=rand()%700;
        }
    }
}

void iKeyboard(unsigned char key){
    if(key=='c'&&current_state==PLAY)
    {
        score=1490;
    }
    if(key=='b'&&current_state==PLAY) current_state=MENU;
    if(key=='a' && ship.x>0&&current_state==PLAY){
        ship.x-=10;
    }
    if(key=='d' && ship.x+80<700&&current_state==PLAY){
        ship.x+=10;
    }
    if(key=='r'&&(current_state==PLAY||current_state==GAMEOVER)) reset();
    if(key=='p'&&current_state== PLAY)
    {
        current_state=pMENU;
    }
    if(key==27) exit(0);
    if(key == ' '&&current_state==PLAY) generate_bullet();
    if(key=='m'&& (current_state==GAMEOVER|| current_state== HELP_MENU))
    {
        reset();
        current_state=MENU;
    }
    if(key==' '&& (current_state==HOME ))
        {   
            mouse_click();
            current_state=ENTER_NAME;
        }
     if (current_state == ENTER_NAME)
    {
        
        if ((key == '\r' || key == '\n')&&nameLen>0)
        {
            
                   
                
                current_state=MENU;
                return ;
        } 

        
        if (key == 8 && nameLen > 0) {
            playerName[--nameLen] = '\0';
            return;
        }

        
        if (key >= 32 && key <= 126 && nameLen < NAME_MAX) {
            playerName[nameLen++] = key;
            playerName[nameLen]   = '\0';
            return;
        }
        if(key==' ')
        {
            playerName[nameLen++]=' ';
            playerName[nameLen]= '\0';
        }

        return;   
    }
    if(current_state==LEADERBOARD&& key=='b')
    {
        current_state=MENU;
        previous_state=LEADERBOARD;
    }
    if(current_state==WIN_GAME && key=='w')
    {   
        reset();
        current_state=MENU;
    }
}


void iSpecialKeyboard(unsigned char key){
    if(key==GLUT_KEY_LEFT && ship.x>0)
    {   
         ship.x-=10;
         iSetSpritePosition(&ship, ship.x, ship.y);
    }
    if(key==GLUT_KEY_RIGHT && ship.x+80<700)
    {   
        ship.x+=10;
         iSetSpritePosition(&ship, ship.x, ship.y);
    } 
}

int leaderboard(const char *name, int score)
{
    
    char lines[21][100];
    int  scores[21];
    int  count = 0;

    
    FILE *fp = fopen("leaderboard.txt", "r");
    if (fp) {
        while (count < 20 && fgets(lines[count], sizeof lines[count], fp)) {
            sscanf(lines[count], " %*[^:]: %d", &scores[count]);
            count++;
        }
        fclose(fp);
    }

    
    snprintf(lines[count], sizeof lines[count], "%s: %d\n",
             (name && *name) ? name : "PLAYER", score);
    scores[count++] = score;

    
    for (int i = 0; i < count; ++i) {
        int max = scores[i], idx = i;
        for (int j = i + 1; j < count; ++j)
            if (scores[j] > max) { max = scores[j]; idx = j; }

        if (idx != i) {
            int t = scores[i]; scores[i] = scores[idx]; scores[idx] = t;
            char tmp[100]; strcpy(tmp, lines[i]);
            strcpy(lines[i], lines[idx]);
            strcpy(lines[idx], tmp);
        }
    }

    
    fp = fopen("leaderboard.txt", "w");
    if (!fp) { perror("leaderboard.txt"); return 1; }
    for (int i = 0; i < 10 && i < count; ++i)
        fputs(lines[i], fp);
    fclose(fp);
    return 0;
    
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    loadresources();
    iSetTimer(50,move_stars);
    iSetTimer(100,iAnim);
    iSetTimer(20,move_bullet);
    iSetTimer(30,move_asteroid);
    iSetTimer(500,generate_asteroid);
    iSetTimer(500,generate_enemy);
    iSetTimer(30,move_enemies);
    iSetTimer(10,collision_checker);
    iSetTimer(100,update_blast);
    iSetTimer(20,update_hittimer);
    iInitializeSound();
    beethovenChannel = iPlaySound("assets/sounds/beethoven1.wav", true, 100);
    iInitialize(700, 800, "SPACE SHOOTER");
    return 0;
}