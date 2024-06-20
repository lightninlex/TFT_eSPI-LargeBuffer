#include <TFT_eSPI.h>
#include "sprite.h"

/*
This sketch was made for the Freenove Wrover-e, which has 4mb flash and 4mb psram. Bodmer has done a wonderful job on this, my most used library.
That being said, the most enviable feature in my opinion, is using the pushToSprite function to make a screen buffer, lowering flickering immensely, 
and all around smoothing out animation. I found that a generic esp32 wroom32 simply does not have the memory to support a screen buffer for the popular
ILI9341 320x240 display. I also noticed that there is no direct buffer function. Now, while we can just push the background sprite to the display after
making whatever changes, I noticed there was still a slight flicker. I have written this to demonstrate how we can not only make a larger sprite, but also 
to store the buffer within psram, allowing for seemingly seamless frame changes!

The user setup 42 text file can be modified to contain the proper pins for the wrover board, as it is programmed with the same "Esp32 Dev Module" board selection
that we would use on a wroom32.
The pins are as follows:
MOSI - 13
SCLK - 14
CS - 15 
DC - 2
RST - 4
*/

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite sprite = TFT_eSprite(&tft);

int x = 120;
int y = 160;
int dX, dY;
int speedX, speedY;

// Here, we create a pointer to the buffer that we can reference
uint16_t* buffer = nullptr;

void setSpeed() {
    speedX = random(2, 5);
    speedY = random(2, 5);
}

void move() {
    x += dX * speedX;
    y += dY * speedY;
    if (x >= 205) {
        dX = -1;
        setSpeed();
    }
    if (x <= -15) {
        dX = 1;
        setSpeed();
    }
    if (y >= 270) {
        dY = -1;
        setSpeed();
    }
    if (y <= -15) {
        dY = 1;
        setSpeed();
    }
}

void setup() {
    tft.init();
    tft.setRotation(2);
    background.createSprite(240, 320);
    background.setSwapBytes(true);
    sprite.createSprite(50, 50, TFT_BLACK);
    sprite.setSwapBytes(true);
    background.fillSprite(TFT_BLACK);

    /* We then allocate memory for the buffer in the psram. 
     We multiply the screen width by the height, and then by the size of the color depth in bytes(2, since we are using 16bit color), 
     which determines the size(in bytes) of the buffer allocation. */

    buffer = (uint16_t*)malloc(240 * 320 * sizeof(uint16_t));
    if (buffer == nullptr) {
        Serial.println("Failed to allocate memory for the buffer!");
        while (1); // Halt the program if memory allocation fails
    }

    randomSeed(analogRead(39));
    setSpeed();
    dX = 1;
    dY = 1;
}

void loop() {
    sprite.pushImage(0, 0, 50, 50, flame);
    sprite.pushToSprite(&background, x, y);

    // Now that we've drawn the sprite data to the background sprite,
    // we copy the background sprite's buffer to the allocated memory
    memcpy(buffer, background.frameBuffer(1), 240 * 320 * sizeof(uint16_t));

    // And finally, we push the buffer to the screen
    tft.pushImage(0, 0, 240, 320, buffer);

    sprite.fillSprite(TFT_BLACK);
    move();
}


// This is not necessary or used in this sketch, but I think garbage collection is something that doesn't get taught often or well enough.
// In another application, you could use this to make your buffers more dynamic, than just one static screen.
// You could have a character sprite buffer, which changes according to whatever variables, and this is what you would use to be able to write
// a new allocation, effectively recycling memory that you need for a different purpose, later in the application


void cleanup() {
    // Free the allocated memory
    if (buffer != nullptr) {
        free(buffer);
        buffer = nullptr;
    }
}
