#include<main.h>


File bmpFile;
File root;
String m_CurrentFilename = "";
String m_CurrentFoldername = "";
int m_FileIndex = 0;
int m_FolderIndex = 0;
int m_NumberOfFiles = 0;
int m_NumberOfFolder = 0;
String m_FileNames[300];
String m_FolderName[300];

int bmpWidth, bmpHeight;

uint8_t bmpDepth, bmpImageoffset;

#define BUFFPIXEL 432 // Number of Led times 3

unsigned int Color(byte b, byte r, byte g); //placed here to avoid compiler error
#define DATA_PIN 4
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define NUM_LEDS 144
bool blynk = false;

CRGB leds[NUM_LEDS];

int BRIGHTNESS = 20;

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

void lv_port_indev_init(void);
//static void keypad_init(void);
static bool keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static uint32_t keypad_get_key(void);
void bmpDraw(char *filename);
void set_black();

lv_obj_t *list_btn;
lv_obj_t *list1;
static lv_obj_t *brightness_label, *speed_label, *delay_label;
lv_obj_t *slider, *slider2, *slider3;
lv_obj_t *label5;

lv_indev_t *indev_keypad;
lv_group_t *group;

PCF8574 pcf8574(0x38);

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  Serial.flush();
}
#endif

//*************Support Funcitons****************//
// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.
uint16_t read16(File &f)
{
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
}
uint32_t read32(File &f)
{
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
}


static void event_handler(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_CLICKED)
  {
    printf("Clicked: %s\n", lv_list_get_btn_text(obj));
    //printDirectory(lv_list_get_btn_text(obj),0);
    char buff[20];
    String StringF = lv_list_get_btn_text(obj);
    StringF.toCharArray(buff, 20);

    //delay(g_dynParam_delay * 1000);

    FastLED.setBrightness(20);
    Serial.print(".......");
    Serial.println(FastLED.getBrightness());
    //Serial.print("BMP Brightness : ");
    //Serial.println(g_dynParam_brightness);
    //camera_trigger();


    bmpDraw(buff);
    //bmpDraw(buff);
    Serial.println("Print_Success");
    Serial.println(buff);
    //Serial.print("bmpDraw dynparam ");
    //Serial.println(g_dynParam_speed);
  }
}

void printDirectory(File dir, int numTabs)
{

  //lv_anim_
  //lv_list_focus(list_btn,LV_ANIM_OFF);

  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    if (entry.isDirectory())
    {
      list_btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, entry.name());
      lv_obj_set_event_cb(list_btn, event_handler);
    }
    else
    {
      //Serial.print(entry.name());
      list_btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, entry.name());
      lv_obj_set_event_cb(list_btn, event_handler);
    }
    /*if (entry.isDirectory())
        {
            Serial.println("/");
            //printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);

        }
        */
    entry.close();
  }
}
static void slider_event_cb_brightness(lv_obj_t *slider, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    static char buf[10]; /* max 3 bytes for number plus 1 null terminating byte */
    snprintf(buf, 10, "%u %%", lv_slider_get_value(slider));
    lv_label_set_text(brightness_label, buf);
  }
}

static void slider_event_cb_speed(lv_obj_t *slider, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    static char buf[10]; /* max 3 bytes for number plus 1 null terminating byte */
    snprintf(buf, 10, "%u ms", lv_slider_get_value(slider));
    lv_label_set_text(speed_label, buf);
  }
}

static void slider_event_cb_delay(lv_obj_t *slider, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    static char buf[10]; /* max 3 bytes for number plus 1 null terminating byte */
    snprintf(buf, 10, "%u s", lv_slider_get_value(slider));
    lv_label_set_text(delay_label, buf);
  }
}
static void event_cb_direction(lv_obj_t *obj, lv_event_t event)
{
  if (event == LV_EVENT_VALUE_CHANGED)
  {
    bool sw_val = lv_switch_get_state(obj);
    if (sw_val)
    {
      lv_label_set_text(label5, "RIGHT");
    }
    else
    {
      lv_label_set_text(label5, "LEFT");
    }
  }
}

//////////////////Function to read BMP and send to Led strip a row at a time/////////////////////
void bmpDraw(char *filename)
{

    File bmpFile;
    int bmpWidth, bmpHeight;             // W+H in pixels
    uint8_t bmpDepth;                    // Bit depth (currently must report 24)
    uint32_t bmpImageoffset;             // Start of image data in file
    uint32_t rowSize;                    // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];     // pixel in buffer (R+G+B per pixel)
    uint32_t povbuffer[BUFFPIXEL];       // pixel out buffer (16-bit per pixel)//////mg/////this needs to be 24bit per pixel////////
    uint32_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
    boolean goodBmp = false;             // Set to true on valid header parse
                   // BMP is stored bottom-to-top
    int w, h, row, col;
    int r, g, b;
    uint32_t pos = 0, startTime = millis();
    uint16_t povidx = 0;
    boolean first = true;
    int delay_image_scroll;
    //boolean flip = flipraw;
    boolean flip = 1;  
    Serial.print("FLIP : ");
    Serial.println(flip);
    // Open requested file on SD card
    bmpFile = SD.open(filename);
    Serial.println(filename);
    // Parse BMP header
    if (read16(bmpFile) == 0x4D42)
    { // BMP signature
        Serial.print("File size: ");
        Serial.println(read32(bmpFile));
        (void)read32(bmpFile);            // Read & ignore creator bytes
        bmpImageoffset = read32(bmpFile); // Start of image data
        Serial.print("Image Offset: ");
        Serial.println(bmpImageoffset, DEC);
        // Read DIB header
        Serial.print("Header size: ");
        Serial.println(read32(bmpFile));
        bmpWidth = read32(bmpFile);
        bmpHeight = read32(bmpFile);
        if (read16(bmpFile) == 1)
        {                               // # planes -- must be '1'
            bmpDepth = read16(bmpFile); // bits per pixel
            Serial.print("Bit Depth: ");
            Serial.println(bmpDepth);
            if ((bmpDepth == 24) && (read32(bmpFile) == 0))
            { // 0 = uncompressed

                goodBmp = true; // Supported BMP format -- proceed!
                Serial.print("Image size: ");
                Serial.print(bmpWidth);
                Serial.print('x');
                Serial.println(bmpHeight);

                // BMP rows are padded (if needed) to 4-byte boundary
                rowSize = (bmpWidth * 3 + 3) & ~3;

                // If bmpHeight is negative, image is in top-down order.
                // This is not canon but has been observed in the wild.
                if (bmpHeight < 0)
                {
                    bmpHeight = -bmpHeight;
                    flip = false;
                }

                w = bmpWidth;
                h = bmpHeight;

                for (row = 0; row < h; row++)
                {
                    //fex.drawProgressBar(0,225,240,15,map(row,0,bmpHeight,0,101),TFT_WHITE,TFT_GREEN);
                    //tft.drawRect(0,220,map(row,0,bmpHeight,0,240),20,TFT_WHITE);
                    if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
                        pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
                    else // Bitmap is stored top-to-bottom
                        pos = bmpImageoffset + row * rowSize;
                    if (bmpFile.position() != pos)
                    { // Need seek?
                        bmpFile.seek(pos);
                        buffidx = sizeof(sdbuffer); // Force buffer reload
                    }

                    for (col = 0; col < w; col++)
                    { // For each column...
                        // read more pixel data
                        if (buffidx >= sizeof(sdbuffer))
                        {
                            povidx = 0;
                            bmpFile.read(sdbuffer, sizeof(sdbuffer));
                            buffidx = 0; // Set index to beginning
                        }
                        // set pixel
                        r = sdbuffer[buffidx++];
                        g = sdbuffer[buffidx++];
                        b = sdbuffer[buffidx++];
                        //Serial.print(r);Serial.print(" ");Serial.print(g);Serial.print(" ");Serial.println(b);
                        //we need to output GRB 24bit colour//
                        //povbuffer[povidx++] =(g<<16) + (r<<8) +b; //original code is b r g, should be g r b?
                        povbuffer[povidx++] = (b << 16) + (g << 8) + r; //original code is b r g, should be g r b?
                                                                        //povbuffer[povidx++] = Color_hex (r,g,b);
                                                                        //Serial.print(povbuffer[povidx++]);
                    }

                    for (int x = 0; x < NUM_LEDS; x++)
                    {
                        leds[x] = povbuffer[x];
                    }
                    FastLED.show();
                    delay_image_scroll = 5 ;
                    //Serial.print (delay_image_scroll);
                    delay(delay_image_scroll); // change the delay time depending effect required
                }                              // end scanline

            } // end goodBmp
        }
    } //end of IF BMP
    //Serial.println();

    bmpFile.close();
    set_black();
}


/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
  static int32_t last_diff = 0;
  int32_t diff = 0;                   /* Dummy - no movement */
  int btn_state = LV_INDEV_STATE_REL; /* Dummy - no press */

  data->enc_diff = diff - last_diff;
  ;
  data->state = btn_state;

  last_diff = diff;

  return false;
}

void print_Directory(File dir, int numTabs)
{
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
      list_btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, entry.name());
      lv_obj_set_event_cb(list_btn, event_handler);
    }
    else
    {
      // files have sizes, directories do not
      list_btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, entry.name());
      lv_obj_set_event_cb(list_btn, event_handler);
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void setup()
{

  Serial.begin(115200); /* prepare for possible serial debug */
  //pinMode(17,OUTPUT);
  //digitalWrite(17, HIGH); // TFT screen chip select
  //digitalWrite( 5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)

  initOTA();

  if (!SD.begin(5))
  {
    Serial.println("initialization failed!");
    while (1);
  }

  Serial.println("SD CARD initialization done.");
  pcf8574.pinMode(P0, INPUT_PULLUP);
  pcf8574.pinMode(P1, INPUT_PULLUP);
  pcf8574.pinMode(P2, INPUT_PULLUP);
  pcf8574.pinMode(P3, INPUT_PULLUP);
  pcf8574.pinMode(P4, INPUT_PULLUP);
  pcf8574.pinMode(P5, INPUT_PULLUP);
  pcf8574.pinMode(P6, INPUT_PULLUP);
  pcf8574.pinMode(P7, INPUT_PULLUP);

  pcf8574.begin();

  lv_init();

    FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    Serial.print("Setup Brightness : ");
    Serial.println(BRIGHTNESS);
    for (int x = 0; x < NUM_LEDS; x++)
    {
        leds[x] = CRGB::Green;
    }
    FastLED.show();
    delay(300);
    for (int x = 0; x < NUM_LEDS; x++)
    {
        leds[x] = CRGB::Red;
    }
    FastLED.show();
    delay(300);
    for (int x = 0; x < NUM_LEDS; x++)
    {
        leds[x] = CRGB::Blue;
    }
    FastLED.show();
    delay(300);
    for (int x = 0; x < NUM_LEDS; x++)
    {
        leds[x] = CRGB::Black;
    }
    FastLED.show();
    delay(10);
    
  //root = SD.open("/");
#if USE_LV_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin();        /* TFT init */
  tft.setRotation(2); /* Landscape orientation */

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 320;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Register a keypad input device*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_KEYPAD;
  indev_drv.read_cb = keypad_read;
  indev_keypad = lv_indev_drv_register(&indev_drv);
  group = lv_group_create();
  lv_indev_set_group(indev_keypad, group);

  /********************************************************************/
  lv_obj_t *tabview;
  tabview = lv_tabview_create(lv_scr_act(), NULL);
  lv_tabview_set_anim_time(tabview, 0);

  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "File");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Settings");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "BOI");

  /*lv_obj_t * img1 = lv_img_create(tab3, NULL);
    lv_img_set_src(img1, &img_cogwheel_argb);
    lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, -20);*/

  lv_obj_t *img2 = lv_img_create(tab3, NULL);
  lv_img_set_src(img2, LV_SYMBOL_USB " Accept");
  //lv_img_set_src(img2, LV_SYMBOL_VIDEO);
  lv_obj_align(img2, tab3, LV_ALIGN_CENTER, 0, 0);

  //lv_obj_align(img2, img1, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  /*lv_obj_t *label = lv_label_create(tab1, NULL);
    lv_label_set_text(label, "Second tab");*/

  /* Create a slider in the center of the display */
  lv_obj_t *slider = lv_slider_create(tab2, NULL);
  lv_obj_set_width(slider, 180);
  lv_obj_align(slider, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 33);
  lv_obj_set_event_cb(slider, slider_event_cb_brightness);
  lv_slider_set_range(slider, 0, 100);
  lv_slider_set_value(slider, 20, LV_ANIM_ON);

  /* Create a slider in the center of the display */
  lv_obj_t *slider2 = lv_slider_create(tab2, NULL);
  lv_obj_set_width(slider2, 180);
  lv_obj_align(slider2, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 77);
  lv_obj_set_event_cb(slider2, slider_event_cb_speed);
  lv_slider_set_range(slider2, 0, 100);
  lv_slider_set_value(slider2, 5, LV_ANIM_ON);

  /* Create a slider in the center of the display */
  lv_obj_t *slider3 = lv_slider_create(tab2, NULL);
  lv_obj_set_width(slider3, 180);
  lv_obj_align(slider3, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 121);
  lv_obj_set_event_cb(slider3, slider_event_cb_delay);
  lv_slider_set_range(slider3, 0, 10);
  lv_slider_set_value(slider3, 0, LV_ANIM_ON);

  lv_obj_t *label = lv_label_create(tab2, NULL);
  lv_label_set_text(label, "Brightness  : ");
  lv_obj_set_pos(label, 20, 5);

  lv_obj_t *label2 = lv_label_create(tab2, NULL);
  lv_label_set_text(label2, "Speed  : ");
  lv_obj_set_pos(label2, 20, 49);

  lv_obj_t *label3 = lv_label_create(tab2, NULL);
  lv_label_set_text(label3, "Delay  : ");
  lv_obj_set_pos(label3, 20, 93);

  lv_obj_t *label4 = lv_label_create(tab2, NULL);
  lv_label_set_text(label4, "Direction  : ");
  lv_obj_set_pos(label4, 20, 148);

  label5 = lv_label_create(tab2, NULL);
  lv_label_set_text(label5, "LEFT");
  lv_obj_set_pos(label5, 177, 148);

  /* Create a label below the slider */
  brightness_label = lv_label_create(tab2, NULL);
  lv_label_set_text(brightness_label, "20 %");
  lv_obj_set_auto_realign(brightness_label, true);
  lv_obj_align(brightness_label, label, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  /* Create a label below the slider */
  speed_label = lv_label_create(tab2, NULL);
  lv_label_set_text(speed_label, "5 ms");
  lv_obj_set_auto_realign(speed_label, true);
  lv_obj_align(speed_label, label2, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

  delay_label = lv_label_create(tab2, NULL);
  lv_label_set_text(delay_label, "0 s");
  lv_obj_set_auto_realign(delay_label, true);
  lv_obj_align(delay_label, label3, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

  lv_obj_t *sw1 = lv_switch_create(tab2, NULL);
  lv_obj_set_pos(sw1, 115, 144);
  lv_obj_set_event_cb(sw1, event_cb_direction);
  //lv_obj_align(sw1, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -100, 0);

  //lv_obj_t *sw2 = lv_switch_create(tab2, NULL);
  //lv_obj_align(sw2, NULL, LV_ALIGN_IN_BOTTOM_MID, 55, -30);

  list1 = lv_list_create(tab1, NULL);
  lv_obj_set_size(list1, 216, 240);
  lv_obj_align(list1, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_list_set_anim_time(list1, 0);
  lv_list_set_scrollbar_mode(list1, LV_SCROLLBAR_MODE_AUTO);
  //root = SD.open("/");
  root = SD.open("/");
  print_Directory(root, 0);
  /*Create a list*/

  /*Add buttons to the list*/

  lv_group_add_obj(group, tabview);

  lv_group_add_obj(group, slider);
  lv_group_add_obj(group, slider2);
  lv_group_add_obj(group, slider3);
  lv_group_add_obj(group, sw1);
  //lv_group_add_obj(group, sw2);
  lv_group_add_obj(group, list1);

  
}

void loop()
{
  loopOTA();

  lv_task_handler(); /* let the GUI do its work */
  delay(5);
  //uint8_t val = pcf8574.digitalRead(P1);
  //if (val==LOW) Serial.println("YOLO");
  //delay(200);
}

/* ===================================================================== *
 *                                                                       *
 * BMPdraw             *
 *                                                                       *
 * ===================================================================== *
 */


 

void set_black() {
  for (int x = 0; x < NUM_LEDS; x++) {
    leds[x] = CRGB::Black;
  }
  FastLED.show();
}

/* Will be called by the library to read the mouse */
static bool keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
  static uint32_t last_key = 0;

  //Serial.println("KEYPAD_READ");
  /*Get whether the a key is pressed and save the pressed key*/
  uint32_t act_key = keypad_get_key();
  //Serial.println(act_key);
  if (act_key != 0)
  {
    data->state = LV_INDEV_STATE_PR;

    /*Translate the keys to LVGL control characters according to your key definitions*/
    switch (act_key)
    {
    case 1:
      act_key = LV_KEY_PREV;
      //Serial.println("NEXT");
      break;
    case 2:
      act_key = LV_KEY_ESC;
      //Serial.println("NEXT");
      break;
    case 3:
      act_key = LV_KEY_NEXT;
      //Serial.println("PREV");
      break;
    case 4:
      act_key = LV_KEY_UP;
      break;
    case 5:
      act_key = LV_KEY_RIGHT;
      break;
    case 6:
      act_key = LV_KEY_ENTER;
      //Serial.println("ENTER");
      break;
    case 7:
      act_key = LV_KEY_DOWN;
      //Serial.println("ESC");
      break;
    case 8:
      act_key = LV_KEY_LEFT;
      //Serial.println("ESC");
      break;
    }

    last_key = act_key;
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }

  data->key = last_key;

  /*Return `false` because we are not buffering and no more data to read*/
  return false;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
  /************************************
  PCB_PINS

  P0  PREV
  P1  ESC
  P2  NEXT
  P3  UP
  P4  RIGHT
  P5  SELECT
  P6  DOWN
  P7  LEFT
  *************************************/
  /*Your code comes here*/
  if (!pcf8574.digitalRead(P0))
  {
    return 1;
    Serial.println("KEY_PREV");
  }
  if (!pcf8574.digitalRead(P1))
  {
    return 2;
    Serial.println("KEY_ESC");
  }
  if (!pcf8574.digitalRead(P2))
  {
    return 3;
    Serial.println("KEY_NEXT");
  }
  if (!pcf8574.digitalRead(P3))
  {
    return 4;
    Serial.println("KEY_UP");
  }
  if (!pcf8574.digitalRead(P4))
  {
    return 5;
    Serial.println("KEY_RIGHT");
  }
  if (!pcf8574.digitalRead(P5))
  {
    return 6;
    Serial.println("KEY_SELECT");
  }
  if (!pcf8574.digitalRead(P6))
  {
    return 7;
    Serial.println("KEY_DOWN");
  }
  if (!pcf8574.digitalRead(P7))
  {
    return 8;
    Serial.println("KEY_LEFT");
  }
  return 0;
}