// Use this to get hex file:
// arduino-cli compile --fqbn arduboy:avr:arduboy .\CowCounterCode.ino --output-dir .\build\

#include <Arduboy2.h>
#include <EEPROM.h>
#include "sprites.h"
Arduboy2 ab;
#define RIGHT_PLAYER_BUTTON B_BUTTON
#define LEFT_PLAYER_BUTTON A_BUTTON
#define EEPROM_START 512

long RightScore = 0;
long LeftScore = 0;

const Point ADD_POINT = {0,0};
const Point CHURCH_POINT = {1,0};
const Point GRAVEYARD_POINT = {2,0};
const Point DELIVERY_POINT = {0,1};

void setup(){
	ab.begin();
	ab.clear();
	ab.setFrameRate(60);
	ab.initRandomSeed();
    readScores();
    drawScores();
    ab.display();
}

void resetScores(){
    RightScore = 0;
    LeftScore = 0;
    saveScores();
}

void readScores(){
    RightScore = EEPROM.get(EEPROM_START, RightScore);
    LeftScore = EEPROM.get(EEPROM_START + 4, LeftScore);
}

void saveScores(){
    EEPROM.put(EEPROM_START, RightScore);
    EEPROM.put(EEPROM_START + 4, LeftScore);
}

void drawScores(){
    int textScale = 2;
    if(digitCount(LeftScore) > 5 || digitCount(RightScore) > 5){
        textScale = 1;
    }
    else{
        textScale = 2;
    }
    drawScoreBig(LeftScore, textScale, 0, 0);
    drawScoreBig(RightScore, textScale, 68, 0);
}

void drawScoreBig(long score, long textScale, long x, long y){
    drawScoreBig(String(score), textScale, x, y);
}

void drawScoreBig(String score, long textScale, long x, long y){
    ab.setCursor(x,y);
    ab.setTextSize(textScale);
    ab.print(score);
    ab.display();
}

// Number roller, like a bike lock
// Assuming a 5x7 default font
void drawSelectBox(long i, long textScale, long x, long y){
    ab.setCursor(x,y);
    //Might want to use drawRoundRect eventually
    //This doesn't currently scale with textScale exactly
    ab.drawRect((x - 2) + (i * 6 * textScale), y - 3, textScale * 6 + 2, textScale * 10);
    ab.display();
}

long digitCount(long num){
    String str = String(num);
    return str.length();
}


void addCows(long& score){
    long maxDigits = 9;
    long x = 96;
    long y = 32;
    long textScale = 2;
    long selectIndexX = 1;
    String prevScore;
    long prevIndexX;
    String scoreString = "+" + String(0);
    ab.clear();
    drawScoreBig(scoreString, textScale, x, y);
    drawSelectBox(selectIndexX, textScale, x, y);
    while(true){
        prevScore = scoreString;
        prevIndexX = selectIndexX;
        ab.pollButtons();
        if(ab.justPressed(LEFT_PLAYER_BUTTON) || ab.justPressed(RIGHT_PLAYER_BUTTON)){
            score = score + scoreString.toInt();
            saveScores();
            break;
        }
        else if(ab.justPressed(LEFT_BUTTON)){
            if(selectIndexX == 0 && scoreString.length() < maxDigits){
                scoreString = scoreString.substring(0, 1) + "0" + scoreString.substring(1);
                x = x - (ab.getCharacterWidth(textScale) + ab.getCharacterSpacing(textScale));
            }
            else{
                selectIndexX--;
            }
        }
        else if(ab.justPressed(RIGHT_BUTTON)){
            selectIndexX++;
        }
        if(selectIndexX == 0 && (ab.justPressed(UP_BUTTON) || ab.justPressed(DOWN_BUTTON))){
            if(scoreString[selectIndexX] == '+'){
                scoreString[selectIndexX] = '-';
            }
            else{
                scoreString[selectIndexX] = '+';
            }
        }
        else if(ab.justPressed(UP_BUTTON)){
            
            scoreString[selectIndexX] = ((scoreString[selectIndexX] - '0') + 1) % 10 + '0'; 
        }
        else if(ab.justPressed(DOWN_BUTTON)){
            if (scoreString[selectIndexX] == '0') {
                scoreString[selectIndexX] = '9';
            } else {
                scoreString[selectIndexX] = ((scoreString[selectIndexX] - '0') - 1) + '0'; 
            }
        }
        selectIndexX = selectIndexX % scoreString.length();
        if(scoreString != prevScore || selectIndexX != prevIndexX){
            ab.clear();
            drawScoreBig(scoreString, textScale, x, y);
            drawSelectBox(selectIndexX, textScale, x, y);
        }
    }
}

void display(){
    ab.clear();
    drawScores();
    ab.display();
}

void drawIcon(uint8_t icon[], Point position, int width, int height){
    ab.drawBitmap(position.x * 128/width,position.y * 64/height, icon, 128/width, 64/height);
}

//Need to convert this to 2 layers. Also, shouldn't hard code the 42 and 64
void drawMenuBox(int x, int y, int width, int height){
    ab.drawRect((x * 128/width),(y*64/height),128/width,64/height);
    drawIcon(Graveyard, GRAVEYARD_POINT, width, height);
    drawIcon(Church, CHURCH_POINT, width, height);
    ab.display();
}

void drawMenuBox(Point p, int width, int height){
    drawMenuBox(p.x, p.y, width, height);
}

void church(long& score){
    score = score * 2;
    saveScores();
}

void graveYard(long& score){
    score = score / 2;
    saveScores();
}

void delivery(long& thief, long& victim){
    long lostCows = victim/10;
    victim = victim - lostCows;
    thief = thief + lostCows;
    saveScores();
}

// Need function for normalizing cows, so that actual cows are worth getting

bool checkPoint(Point a, Point b){
    return a.x == b.x && a.y == b.y;
}

void moveIndex(Point& index, int width, int height){
        if(ab.justPressed(RIGHT_BUTTON)){
            index.x++;
        }
        else if(ab.justPressed(LEFT_BUTTON)){
            index.x--;
        }
        if(index.x > (width - 1)){
            index.x = 0;
        }
        else if(index.x < 0){
            index.x = width - 1;
        }
        if(ab.justPressed(UP_BUTTON)){
            index.y++;
        }
        else if(ab.justPressed(DOWN_BUTTON)){
            index.y--;
        }
        if(index.y > (height - 1)){
            index.y = 0;
        }
        else if(index.y < 0){
            index.y = height - 1;
        }
}

bool triggerMenu(Point index, long& score, long& otherScore){
    if(checkPoint(index,ADD_POINT)){
        addCows(score);
        return true;
    }
    else if(checkPoint(index,CHURCH_POINT)){
        church(score);
        return true;
    }
    else if(checkPoint(index,GRAVEYARD_POINT)){
        graveYard(score);
        return true;
    }
    else if(checkPoint(index,DELIVERY_POINT)){
        delivery(score, otherScore);
    }
}

void menu(long& score, long& otherScore){
    ab.clear();
    Point selectIndex = {0,0};
    Point prevIndex = {0,0};
    int selectIndexWidth = 3;
    int selectIndexHeight = 2;
    drawMenuBox(selectIndex, selectIndexWidth, selectIndexHeight);
    while(true){
        ab.pollButtons();
        prevIndex.x = selectIndex.x;
        prevIndex.y = selectIndex.y;
        moveIndex(selectIndex, selectIndexWidth, selectIndexHeight);
        if(!checkPoint(selectIndex,prevIndex)){
            ab.clear();
            drawMenuBox(selectIndex, selectIndexWidth, selectIndexHeight);
        }
        if(ab.justPressed(A_BUTTON) || ab.justPressed(B_BUTTON)){
            if(triggerMenu(selectIndex, score, otherScore)){
                break;
            }
        }
    }
}

long resetCounter = 0;
void loop()
{
	if (!ab.nextFrame()) return;
    ab.pollButtons();
    if(ab.pressed(LEFT_BUTTON | RIGHT_BUTTON | UP_BUTTON | DOWN_BUTTON)){
        resetCounter++;
    }
    if(resetCounter > 60){
        resetScores();
        resetCounter = 0;
        display();
    }
    long rightOld = RightScore;
    long leftOld = LeftScore;
	
    if(rightOld != RightScore || leftOld != LeftScore){
        display();
    }
    
    if(ab.justPressed(LEFT_PLAYER_BUTTON)){
        menu(LeftScore, RightScore);
        display();
    }
    else if(ab.justPressed(RIGHT_PLAYER_BUTTON)){
        menu(RightScore, LeftScore);
        display();
    }
	
}
