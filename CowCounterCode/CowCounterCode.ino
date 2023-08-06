// Use this to get hex file:
// arduino-cli compile --fqbn arduboy:avr:arduboy .\CowCounterCode.ino --output-dir .\build\

#include <Arduboy2.h>
#include <EEPROM.h>
#include "sprites.h"
Arduboy2 ab;
#define RIGHT_PLAYER_BUTTON B_BUTTON
#define LEFT_PLAYER_BUTTON A_BUTTON
#define EEPROM_START 512
//May need to start using longs as some polong
long RightScore = 0;
long LeftScore = 0;
long CowCache = 0;
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

//May need to change to longs, so + 4 apart
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
    long selectIndex = 1;
    String prevScore;
    long prevIndex;
    String scoreString = "+" + String(0);
    ab.clear();
    drawScoreBig(scoreString, textScale, x, y);
    drawSelectBox(selectIndex, textScale, x, y);
    while(true){
        prevScore = scoreString;
        prevIndex = selectIndex;
        ab.pollButtons();
        if(ab.justPressed(LEFT_PLAYER_BUTTON) || ab.justPressed(RIGHT_PLAYER_BUTTON)){
            score = score + scoreString.toInt();
            saveScores();
            break;
        }
        else if(ab.justPressed(LEFT_BUTTON)){
            if(selectIndex == 0 && scoreString.length() < maxDigits){
                scoreString = scoreString.substring(0, 1) + "0" + scoreString.substring(1);
                x = x - (ab.getCharacterWidth(textScale) + ab.getCharacterSpacing(textScale));
            }
            else{
                selectIndex--;
            }
        }
        else if(ab.justPressed(RIGHT_BUTTON)){
            selectIndex++;
        }
        if(selectIndex == 0 && (ab.justPressed(UP_BUTTON) || ab.justPressed(DOWN_BUTTON))){
            if(scoreString[selectIndex] == '+'){
                scoreString[selectIndex] = '-';
            }
            else{
                scoreString[selectIndex] = '+';
            }
        }
        else if(ab.justPressed(UP_BUTTON)){
            
            scoreString[selectIndex] = ((scoreString[selectIndex] - '0') + 1) % 10 + '0'; 
        }
        else if(ab.justPressed(DOWN_BUTTON)){
            if (scoreString[selectIndex] == '0') {
                scoreString[selectIndex] = '9';
            } else {
                scoreString[selectIndex] = ((scoreString[selectIndex] - '0') - 1) + '0'; 
            }
        }
        selectIndex = selectIndex % scoreString.length();
        if(scoreString != prevScore || selectIndex != prevIndex){
            ab.clear();
            drawScoreBig(scoreString, textScale, x, y);
            drawSelectBox(selectIndex, textScale, x, y);
        }
    }
}

void display(){
    ab.clear();
    drawScores();
    ab.display();
}

void drawMenuBox(long i){
    ab.drawRoundRect(0 + (i * 42),0,42,64,6);
    ab.drawBitmap(0,0,Cow, 42, 64);
    ab.drawBitmap(42,0,Church,42,64);
    ab.drawBitmap(84,0,Graveyard,42,64);
    ab.display();
}

void church(long& score){
    score = score * 2;
    saveScores();
}

void graveYard(long& score){
    score = score / 2;
    saveScores();
}

#define ADD_INDEX 0
#define CHURCH_INDEX 1
#define GRAVEYARD_INDEX 2

void menu(long& score){
    ab.clear();
    long selectIndex = 0;
    long prevIndex = selectIndex;
    drawMenuBox(selectIndex);
    while(true){
        ab.pollButtons();
        prevIndex = selectIndex;
        if(ab.justPressed(RIGHT_BUTTON)){
            selectIndex++;
        }
        else if(ab.justPressed(LEFT_BUTTON)){
            selectIndex--;
        }
        if(selectIndex > 2){
            selectIndex = 0;
        }
        else if(selectIndex < 0){
            selectIndex = 2;
        }
        if(selectIndex != prevIndex){
            ab.clear();
            drawMenuBox(selectIndex);
        }
        if(ab.justPressed(A_BUTTON) || ab.justPressed(B_BUTTON)){
            if(selectIndex == ADD_INDEX){
                addCows(score);
                break;
            }
            else if(selectIndex == CHURCH_INDEX){
                church(score);
                break;
            }
            else if(selectIndex == GRAVEYARD_INDEX){
                graveYard(score);
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
        menu(LeftScore);
        display();
    }
    else if(ab.justPressed(RIGHT_PLAYER_BUTTON)){
        menu(RightScore);
        display();
    }
	
}
