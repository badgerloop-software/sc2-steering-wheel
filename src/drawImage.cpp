#include "drawImage.h"

// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

//Array to hold 10 TFT color values
uint16_t colors[10] = {TFT_RED, TFT_BLUE, TFT_GREEN, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_PINK, TFT_PURPLE};

//Store old image data to clear the relevant portion of screen
volatile int old_start_x = 0;
volatile int old_start_y = 0;
volatile int old_width = 0;
volatile int old_height = 0;

//Initialize TFT display
void initDisp(){
    tft.begin();
    tft.fillScreen(TFT_BLACK);
}

//Slide Show of colors to verify TFT display is working
void rotateColors(){
    static uint16_t color = 10;
    tft.fillScreen(colors[color]);
    color = (color + 1) % 10;
}

//Draw image on TFT display from bitmap
//image_code: image to be drawn
//clear_old: clear the previous image
//returns: true if image is drawn successfully, false otherwise
bool drawImageFromBitmap(image_t image_code, bool clear_old) {
    // Clear the previous image
    if (clear_old){
        tft.fillRect(old_start_x, old_start_y, old_width, old_height, TFT_BLACK);
    }

    // Use the converted data
    switch(image_code)
    {
        // case BSR_LOGO:
        //     tft.pushImage(WIDTH/2 - BSRlogo_WIDTH/2, HEIGHT/2 - BSRlogo_HEIGHT/2, BSRlogo_WIDTH, BSRlogo_HEIGHT  , BSRlogo, true);
        //     old_start_x = WIDTH/2 - BSRlogo_WIDTH/2;
        //     old_start_y = HEIGHT/2 - BSRlogo_HEIGHT/2;
        //     old_width = BSRlogo_WIDTH;
        //     old_height = BSRlogo_HEIGHT;
        //     break;
        // case CAR_IMAGE:
        //     tft.pushImage(WIDTH/2 - kar_WIDTH/2, HEIGHT/2 - kar_HEIGHT/2, kar_WIDTH, kar_HEIGHT, kar, true);
        //     old_start_x = WIDTH/2 - kar_WIDTH/2;
        //     old_start_y = kar_HEIGHT/2 - 168/2;
        //     old_width = kar_WIDTH;
        //     old_height = kar_HEIGHT;
        //     break;
        case ANIMATION_1:
            playAnimation(clear_old);  // Play the animation frames
            break;
        default:
            return false;
    }  

    return true;
}

void clearScreen(){
    // Clear the entire screen to black
    tft.fillScreen(TFT_BLACK);

    // Reset old image data
    old_start_x = 0;
    old_start_y = 0;
    old_width = 0;
    old_height = 0;
}

void displayFrame(int frameIndex){
    // Calculate the position and size of the frame to display
    int frame_width = ANIMATION_WIDTH;  // Assuming a fixed width for each frame
    int frame_height = ANIMATION_HEIGHT;  // Assuming a fixed height for each frame

    // Calculate the position to center the frame on the screen
    int start_x = (WIDTH - frame_width) / 2;
    int start_y = (HEIGHT - frame_height) / 2;

    int offset = frameIndex * frame_width * frame_height;  // Calculate the offset for the current frame

    // Display the frame on the TFT screen
    tft.pushImage(start_x, start_y, frame_width, frame_height, animation + offset, true);
}

// To animate through all frames:
void playAnimation(bool clear_old) {
    // Clear the previous image if any
    if (clear_old){
        tft.fillRect(old_start_x, old_start_y, old_width, old_height, TFT_BLACK);
    }

    for (int i = 0; i < ANIMATION_FRAMES; i++) {
        displayFrame(i);
        delay(42);  // Adjust for desired frame rate
        Serial.printf("Displaying frame: %d\n", i);  // Debug message to indicate the current frame being displayed
    }
}