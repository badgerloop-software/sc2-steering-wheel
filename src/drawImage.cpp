#include "drawImage.h"
#include "JPEGDecoder.h"
// Create a TFT_eSPI object
TFT_eSPI tft = TFT_eSPI();

//Array to hold 10 TFT color values
uint16_t colors[10] = {TFT_RED, TFT_BLUE, TFT_GREEN, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_PINK, TFT_PURPLE};

//Store old image data to clear the relevant portion of screen
volatile int old_start_x = 0;
volatile int old_start_y = 0;
volatile int old_width = 0;
volatile int old_height = 0;

//Heap pointer for animation frames
uint8_t *frameHeap = NULL;

//Initialize TFT display
void initDisp(){

    digitalWrite(A3, HIGH); 
    digitalWrite(D3, HIGH); 

    initSD(); // Initialize the SD card

    if (SD.exists("/bsr/JONATHAN.JPG")) {
        Serial.println("BSR folder exists on SD card.");
    } else {
        Serial.println("BSR folder does not exist on SD card.");
    }

    //---------Until this point, expected behavior
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
            HeapAnim(clear_old);  // Play the animation frames
            break;
        case JONATHAN:
            drawSdJpeg("/bsr/jonathan.jpg", WIDTH/2 - 200/2, HEIGHT/2 - 200/2);
            old_start_x = WIDTH/2 - 200/2;
            old_start_y = HEIGHT/2 - 200/2;
            old_width = 200;
            old_height = 200;
            break;
        default:
            return false;
    }  

    return true;
}

void clearScreen(){
    // Clear the entire screen to black
    tft.fillScreen(TFT_BLACK);

    if (frameHeap != NULL) {
        free(frameHeap);  // Free the allocated memory for the frame heap
        frameHeap = NULL; // Set the pointer to NULL after freeing
    }

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

void HeapAnim(bool clear_old){
    if (clear_old){
        tft.fillRect(old_start_x, old_start_y, old_width, old_height, TFT_BLACK);
    }

    frameHeap = initFrameHeap(ANIMATION_WIDTH * ANIMATION_HEIGHT);

    if (frameHeap == NULL) {
        Serial.println("Failed to allocate memory for frame heap");
        return;
    }

    for (int i = 0; i < 14; i++) {
        HeapDispFrame(i);
        delay(42);  // Adjust for desired frame rate
        Serial.printf("Displaying frame: %d\n", i);  // Debug message to indicate the current frame being displayed
    }
}

void HeapDispFrame(int frameIndex){
    loadFrameIntoHeap(frameIndex*ANIMATION_HEIGHT*ANIMATION_WIDTH, ANIMATION_WIDTH * ANIMATION_HEIGHT, frameHeap);

    // Calculate the position and size of the frame to display
    int frame_width = ANIMATION_WIDTH;  // Assuming a fixed width for each frame
    int frame_height = ANIMATION_HEIGHT;  // Assuming a fixed height for each frame

    // Calculate the position to center the frame on the screen
    int start_x = (WIDTH - frame_width) / 2;
    int start_y = (HEIGHT - frame_height) / 2;

    // Display the frame on the TFT screen
    tft.pushImage(start_x, start_y, frame_width, frame_height, frameHeap, true);
}


void drawSdJpeg(const char *filename, int xpos, int ypos) {

    // Open the named file (the Jpeg decoder library will close it)
    File jpegFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library
   
    if ( !jpegFile ) {
      Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
      return;
    }
  
    // Use one of the following methods to initialise the decoder:
    bool decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
    //bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)
  
    if (decoded) {
      // render the image onto the screen at given coordinates
      jpegRender(xpos, ypos);
    }
    else {
      Serial.println("Jpeg file format not supported!");
    }
  }

  void jpegRender(int xpos, int ypos) {

    //jpegInfo(); // Print information from the JPEG file (could comment this line out)
  
    uint16_t *pImg;
    uint16_t mcu_w = JpegDec.MCUWidth;
    uint16_t mcu_h = JpegDec.MCUHeight;
    uint32_t max_x = JpegDec.width;
    uint32_t max_y = JpegDec.height;
  
    bool swapBytes = tft.getSwapBytes();
    tft.setSwapBytes(true);
    
    // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
    // Typically these MCUs are 16x16 pixel blocks
    // Determine the width and height of the right and bottom edge image blocks
    uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
    uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);
  
    // save the current image block size
    uint32_t win_w = mcu_w;
    uint32_t win_h = mcu_h;
  
    // record the current time so we can measure how long it takes to draw an image
    uint32_t drawTime = millis();
  
    // save the coordinate of the right and bottom edges to assist image cropping
    // to the screen size
    max_x += xpos;
    max_y += ypos;
  
    // Fetch data from the file, decode and display
    while (JpegDec.read()) {    // While there is more data in the file
      pImg = JpegDec.pImage ;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)
  
      // Calculate coordinates of top left corner of current MCU
      int mcu_x = JpegDec.MCUx * mcu_w + xpos;
      int mcu_y = JpegDec.MCUy * mcu_h + ypos;
  
      // check if the image block size needs to be changed for the right edge
      if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
      else win_w = min_w;
  
      // check if the image block size needs to be changed for the bottom edge
      if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
      else win_h = min_h;
  
      // copy pixels into a contiguous block
      if (win_w != mcu_w)
      {
        uint16_t *cImg;
        int p = 0;
        cImg = pImg + win_w;
        for (int h = 1; h < win_h; h++)
        {
          p += mcu_w;
          for (int w = 0; w < win_w; w++)
          {
            *cImg = *(pImg + w + p);
            cImg++;
          }
        }
      }
  
      // calculate how many pixels must be drawn
      uint32_t mcu_pixels = win_w * win_h;
  
      // draw image MCU block only if it will fit on the screen
      if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
        tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
      else if ( (mcu_y + win_h) >= tft.height())
        JpegDec.abort(); // Image has run off bottom of screen so abort decoding
    }
  
    tft.setSwapBytes(swapBytes);
  }
  
  