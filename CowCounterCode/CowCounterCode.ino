// Use this to get hex file:
// arduino-cli compile --fqbn arduboy:avr:arduboy .\CowCounterCode.ino --output-dir .\build\

#include <Arduboy2.h>
#include <EEPROM.h>
Arduboy2 ab;
#define BEN_BUTTON B_BUTTON
#define ABBY_BUTTON A_BUTTON
#define BACK_BUTTON LEFT_BUTTON
#define EEPROM_START 512
int BenScore = 0;
int AbbyScore = 0;
int CowCache = 0;
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
    BenScore = 0;
    AbbyScore = 0;
    saveScores();
}

void readScores(){
    BenScore = EEPROM.get(EEPROM_START, BenScore);
    AbbyScore = EEPROM.get(EEPROM_START + 2, AbbyScore);
}

void saveScores(){
    EEPROM.put(EEPROM_START, BenScore);
    EEPROM.put(EEPROM_START + 2, AbbyScore);
}

void drawScores(){
    drawScoreBig(AbbyScore, 2, 3, 3);
    drawScoreBig(BenScore, 2, 64, 3);
    // ab.setCursor(3,3);
    // ab.print(AbbyScore);
    // ab.setCursor(64,3);
    // ab.print(BenScore);
}

void drawScoreBig(int score, int textScale, int x, int y){
    drawScoreBig(String(score), textScale, x, y);
}

void drawScoreBig(String score, int textScale, int x, int y){
    ab.setCursor(x,y);
    ab.setTextSize(textScale);
    ab.print(score);
    ab.display();
}

// Number roller, like a bike lock
// Assuming a 5x7 default font
void drawSelectBox(int i, int textScale, int x, int y){
    ab.setCursor(x,y);
    //Might want to use drawRoundRect eventually
    //This doesn't currently scale with textScale exactly
    ab.drawRect((x - 1) + (i * 6 * textScale), y - 3, textScale * 6, textScale * 10);
    ab.display();
}

int digitCount(int num){
    String str = String(num);
    return str.length();
}


void addCows(int button, int& score){
    int x = 96;
    int y = 32;
    int textScale = 2;
    int selectIndex = 0;
    String prevScore;
    int prevIndex;
    String scoreString = String(0);
    ab.clear();
    drawScoreBig(scoreString, textScale, x, y);
    drawSelectBox(selectIndex, textScale, x, y);
    while(true){
        prevScore = scoreString;
        prevIndex = selectIndex;
        ab.pollButtons();
        if(ab.justPressed(button)){
            score = score + scoreString.toInt();
            saveScores();
            break;
        }
        else if(ab.justPressed(LEFT_BUTTON)){
            if(selectIndex == 0 && scoreString.length() < 8){
                scoreString = "0" + scoreString;
                x = x - (ab.getCharacterWidth(textScale) + ab.getCharacterSpacing(textScale));
            }
            else{
                selectIndex--;
            }
        }
        else if(ab.justPressed(RIGHT_BUTTON)){
            selectIndex++;
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

int resetCounter = 0;
void loop()
{
	if (!ab.nextFrame()) return;
    if(ab.pressed(LEFT_BUTTON | RIGHT_BUTTON | UP_BUTTON | DOWN_BUTTON)){
        resetCounter++;
    }
    if(resetCounter > 60){
        resetScores();
        resetCounter = 0;
        display();
    }
    int benOld = BenScore;
    int abbyOld = AbbyScore;
	ab.pollButtons();
    if(benOld != BenScore || abbyOld != AbbyScore){
        display();
    }
    
    if(ab.justPressed(ABBY_BUTTON)){
        addCows(ABBY_BUTTON, AbbyScore);
        display();
    }
    else if(ab.justPressed(BEN_BUTTON)){
        addCows(BEN_BUTTON, BenScore);
        display();
    }
	
}

// void saveScore(){
//     int highScore = EEPROM.read(EEPROM_START);
//     if(turn > highScore){
//         EEPROM.update(EEPROM_START, turn);
//     }
// }

