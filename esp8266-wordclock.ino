#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <NTPtimeESP.h>

#define PROJECT_NAME "FalkOnTime"
#define TIME_ZONE 1
#define LED_COUNT 114
#define LED_DATA_PIN D6
#define LDR_SENSOR_PIN A0

#define DISPLAY_HET_IS 1
#define DISPLAY_UUR_WOORD 1
#define DISPLAY_CORNER_MINUTES 1
#define BRIGHTNESS 5 // about 1/5 (max = 255)

NTPtime NTPch("nl.pool.ntp.org");

/*
 * The structure contains following fields:
 * struct strDateTime
{
  byte hour;
  byte minute;
  byte second;
  int year;
  byte month;
  byte day;
  byte dayofWeek;
  boolean valid;
};
 */
strDateTime dateTime;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_DATA_PIN, NEO_GRBW + NEO_KHZ800);

byte LDR_corr_type = 0;
extern const uint16_t LDR_corr_log[];
extern const uint16_t LDR_corr_sqrt[];
static char time_displayed[5] = {};
static char current_time[5] = {};

int summertime = 0;

// Binders
byte het_is_mask[11] = {101, 100, 81, 61, 60, 0, 0, 0, 0, 0, 0};
byte over_half_mask[11] = {103, 98, 83, 78, 0, 0, 0, 0, 0, 0, 0};
byte over_mask[11] = {37, 24, 17, 4, 0, 0, 0, 0, 0, 0, 0};
byte voor_half_mask[11] = {39, 22, 19, 2, 0, 0, 0, 0, 0, 0, 0};
byte voor_mask[11] = {105, 96, 85, 76, 0, 0, 0, 0, 0, 0, 0};

// Minutes
byte vijf_min_mask[11] = {40, 21, 20, 1, 0, 0, 0, 0, 0, 0, 0};
byte tien_min_mask[11] = {102, 99, 82, 79, 0, 0, 0, 0, 0, 0, 0};
byte kwart_min_mask[11] = {43, 38, 23, 18, 3, 0, 0, 0, 0, 0, 0};
byte half_min_mask[11] = {104, 97, 84, 77, 0, 0, 0, 0, 0, 0, 0};

// Hours
byte uur_mask[11] = {30, 11, 10, 0, 0, 0, 0, 0, 0, 0, 0};
byte een_uur[11] = {36, 25, 16, 0, 0, 0, 0, 0, 0, 0, 0};
byte twee_uur[11] = {106, 95, 86, 75, 0, 0, 0, 0, 0, 0, 0};
byte drie_uur[11] = {35, 26, 15, 6, 0, 0, 0, 0, 0, 0, 0};
byte vier_uur[11] = {107, 94, 87, 74, 0, 0, 0, 0, 0, 0, 0};
byte vijf_uur[11] = {67, 54, 47, 34, 0, 0, 0, 0, 0, 0, 0};
byte zes_uur[11] = {27, 14, 7, 0, 0, 0, 0, 0, 0, 0, 0};
byte zeven_uur[11] = {108, 93, 88, 73, 68, 0, 0, 0, 0, 0, 0};
byte acht_uur[11] = {109, 92, 89, 72, 0, 0, 0, 0, 0, 0, 0};
byte negen_uur[11] = {48, 33, 28, 13, 8, 0, 0, 0, 0, 0, 0};
byte tien_uur[11] = {69, 52, 49, 32, 0, 0, 0, 0, 0, 0, 0};
byte elf_uur[11] = {29, 12, 9, 0, 0, 0, 0, 0, 0, 0, 0};
byte twaalf_uur[11] = {110, 91, 90, 71, 70, 51, 0, 0, 0, 0, 0};
byte* hours_mask[12] = {een_uur, twee_uur, drie_uur, vier_uur, vijf_uur, zes_uur, zeven_uur, acht_uur, negen_uur, tien_uur, elf_uur, twaalf_uur};

// Single minutes
byte min_1[11] = {113,   0,   0,  0, 0, 0, 0, 0, 0, 0, 0};
byte min_2[11] = {113, 112,   0,  0, 0, 0, 0, 0, 0, 0, 0};
byte min_3[11] = {113, 112, 111,  0, 0, 0, 0, 0, 0, 0, 0};
byte min_4[11] = {113, 112, 111, 114, 0, 0, 0, 0, 0, 0, 0};
byte* single_minutes_mask[4] = {min_1, min_2, min_3, min_4};

void on_ota_start() {
  Serial.println("OTA Start\n");
}

void on_ota_end() {
  Serial.println("OTA End\n");
}

void on_ota_progress(unsigned int progress, unsigned int total) {
  char buf[32];
  memset(buf, '\0', sizeof(buf));
  snprintf(buf, sizeof(buf) - 1, "Upgrade - %02u%%\n", (progress / (total / 100)));
  Serial.println(buf);
}

void on_ota_error(ota_error_t error) {
  Serial.println("Error - ");

  if (error == OTA_AUTH_ERROR)
    Serial.println("Auth Failed\n");
  else if (error == OTA_BEGIN_ERROR)
    Serial.println("Begin Failed\n");
  else if (error == OTA_CONNECT_ERROR)
    Serial.println("Connect Failed\n");
  else if (error == OTA_RECEIVE_ERROR)
    Serial.println("Receive Failed\n");
  else if (error == OTA_END_ERROR)
    Serial.println("End Failed\n");
}

void on_wifi_ap_callback(WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFiManager wifiManager;
  
  //reset settings - for testing
  //wifiManager.resetSettings();
  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(on_wifi_ap_callback);
  
  // Fetches ssid and pass and tries to connect
  // If it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(PROJECT_NAME)) {
    Serial.println("failed to connect and hit timeout");
    // Reset and try again, or maybe put it to deep sleep
    ESP.restart(); // originally ESP.reset()
    delay(1000);
  }
  
  Serial.print("IP for web server is ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname(PROJECT_NAME);
  ArduinoOTA.onStart(on_ota_start);
  ArduinoOTA.onStart(on_ota_end);
  ArduinoOTA.onProgress(on_ota_progress);
  ArduinoOTA.onError(on_ota_error);
  ArduinoOTA.begin();
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Turn OFF all pixels ASAP
}

void loop() {
//  Serial.println("Start loop");
  
  if (WiFi.status() == WL_CONNECTED) {
//    Serial.println("Wifi connected");
    ArduinoOTA.handle();
    update_time();
    update_time_displayed();
  } else {
    Serial.println("Wifi not connected, restarting");
    ESP.restart();
  }
}

void update_time() {
  strDateTime newDateTime = NTPch.getNTPtime(1.0, 1);

  if (newDateTime.valid) {
    dateTime = newDateTime;
  }
}

void update_time_displayed() {
  byte cur_hours = dateTime.hour;
  byte cur_minutes  = dateTime.minute;
  sprintf(current_time, "%02d%02d", cur_hours, cur_minutes);

  if (strcmp(time_displayed, current_time) != 0) {
    Serial.println("Time not equal!");
    Serial.println(current_time);
    strncpy(time_displayed, current_time, 4);
    onboard_led_flash(1000);
    strip_update_time_shown();
  }
}

void flash_leds() {
  uint8_t wait = 1000;
  strip_pulse_white(wait);
  onboard_led_flash(wait);
}

void onboard_led_flash(uint8_t wait) {
  digitalWrite(LED_BUILTIN, LOW);
  delay(wait);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(wait);  
}

void strip_update_time_shown() {
  strip_all_off();

  int hours = dateTime.hour;
  int minutes  = dateTime.minute;
  
  // Single minutes
  int single_minutes = minutes % 5;
  if (single_minutes > 0 && DISPLAY_CORNER_MINUTES) {
    strip_apply_mask(single_minutes_mask[single_minutes - 1]);
  }
 
  // Five minutes
  int five_minutes = minutes - single_minutes;

//  if (five_minutes > 15) {
//    hours++;
//  }
  
  // Hours
  if (hours > 12)
    hours = hours % 12;
  if (hours == 0)
    hours = 12;

  // Display: Het is
  if (DISPLAY_HET_IS)
    strip_apply_mask(het_is_mask);

  switch (five_minutes) {
    case 0:
      strip_apply_mask(hours_mask[hours]);
      if (DISPLAY_UUR_WOORD) strip_apply_mask(uur_mask);
      break;
    case 5:
      strip_apply_mask(vijf_min_mask);
      strip_apply_mask(over_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 10:
      strip_apply_mask(tien_min_mask);
      strip_apply_mask(over_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 15:
      strip_apply_mask(kwart_min_mask);
      strip_apply_mask(over_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 20:
      strip_apply_mask(tien_min_mask);
      strip_apply_mask(voor_half_mask);
      strip_apply_mask(half_min_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 25:
      strip_apply_mask(vijf_min_mask);
      strip_apply_mask(voor_half_mask);
      strip_apply_mask(half_min_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 30:
      strip_apply_mask(half_min_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 35:
      strip_apply_mask(vijf_min_mask);
      strip_apply_mask(over_half_mask);
      strip_apply_mask(half_min_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 40:
      strip_apply_mask(tien_min_mask);
      strip_apply_mask(over_half_mask);
      strip_apply_mask(half_min_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 45:
      strip_apply_mask(kwart_min_mask);
      strip_apply_mask(voor_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 50:
      strip_apply_mask(tien_min_mask);
      strip_apply_mask(voor_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
    case 55:
      strip_apply_mask(vijf_min_mask);
      strip_apply_mask(voor_mask);
      strip_apply_mask(hours_mask[hours]);
      break;
  }

  strip.show();
}

void strip_apply_mask(byte x[]) {
  for (byte i = 0; i < 11; i++) {
    if (x[i] != 0) {
      strip.setPixelColor(x[i] - 1, strip.Color(0, 0, 0, 255));
    }
  }

  //Serial.print("LED color: H: ");
  //Serial.print(h_clock);
  //Serial.print(", S: ");
  //Serial.print(s_clock);
  //Serial.print(", V: ");
  //Serial.println(v_clock);
}

void strip_all_off() {
  strip.clear();
}

void strip_pulse_white(uint8_t wait) {
  for (int j = 0; j < 256; j++) { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
  }
  strip.show();
  delay(wait);

  for (int j = 255; j >= 0; j--) { // Ramp down from 255 to 0
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(j)));
  }
  strip.show();
  delay(wait);
}

int read_light_level() {
  int sensor_value = analogRead(LDR_SENSOR_PIN); // Value between 0 and 1023
  sensor_value = constrain(sensor_value, 0, 1023);
  sensor_value = ldr_correction(sensor_value);
  
  return sensor_value;
}

int ldr_correction(int sensor_value) {
  switch (LDR_corr_type) {
    case 0: // None
      return sensor_value;
      break;

    case 1: // Square root
      return pgm_read_word(&LDR_corr_sqrt[sensor_value]);
      break;

    case 2: // Logarithmic
      return pgm_read_word(&LDR_corr_log[sensor_value]);
      break;
  }
}

// LDR correction (logarithm)
// Function: y = log10(x+1)*1023/log10(1023+1);
const uint16_t PROGMEM LDR_corr_log[] = {
  0, 102, 162, 204, 237, 264, 287, 306, 324, 339, 353, 366, 378, 389, 399, 409,
  418, 426, 434, 442, 449, 456, 462, 469, 475, 480, 486, 491, 496, 501, 506, 511,
  516, 520, 524, 528, 532, 536, 540, 544, 548, 551, 555, 558, 561, 565, 568, 571,
  574, 577, 580, 583, 585, 588, 591, 594, 596, 599, 601, 604, 606, 609, 611, 613,
  616, 618, 620, 622, 624, 627, 629, 631, 633, 635, 637, 639, 641, 642, 644, 646,
  648, 650, 652, 653, 655, 657, 659, 660, 662, 664, 665, 667, 668, 670, 672, 673,
  675, 676, 678, 679, 681, 682, 684, 685, 686, 688, 689, 691, 692, 693, 695, 696,
  697, 699, 700, 701, 702, 704, 705, 706, 707, 709, 710, 711, 712, 713, 714, 716,
  717, 718, 719, 720, 721, 722, 723, 725, 726, 727, 728, 729, 730, 731, 732, 733,
  734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749,
  749, 750, 751, 752, 753, 754, 755, 756, 757, 757, 758, 759, 760, 761, 762, 763,
  763, 764, 765, 766, 767, 768, 768, 769, 770, 771, 772, 772, 773, 774, 775, 775,
  776, 777, 778, 778, 779, 780, 781, 781, 782, 783, 784, 784, 785, 786, 787, 787,
  788, 789, 789, 790, 791, 791, 792, 793, 794, 794, 795, 796, 796, 797, 798, 798,
  799, 800, 800, 801, 801, 802, 803, 803, 804, 805, 805, 806, 807, 807, 808, 808,
  809, 810, 810, 811, 811, 812, 813, 813, 814, 814, 815, 816, 816, 817, 817, 818,
  818, 819, 820, 820, 821, 821, 822, 822, 823, 824, 824, 825, 825, 826, 826, 827,
  827, 828, 828, 829, 830, 830, 831, 831, 832, 832, 833, 833, 834, 834, 835, 835,
  836, 836, 837, 837, 838, 838, 839, 839, 840, 840, 841, 841, 842, 842, 843, 843,
  844, 844, 845, 845, 846, 846, 847, 847, 848, 848, 849, 849, 849, 850, 850, 851,
  851, 852, 852, 853, 853, 854, 854, 854, 855, 855, 856, 856, 857, 857, 858, 858,
  858, 859, 859, 860, 860, 861, 861, 862, 862, 862, 863, 863, 864, 864, 864, 865,
  865, 866, 866, 867, 867, 867, 868, 868, 869, 869, 869, 870, 870, 871, 871, 871,
  872, 872, 873, 873, 873, 874, 874, 875, 875, 875, 876, 876, 877, 877, 877, 878,
  878, 879, 879, 879, 880, 880, 880, 881, 881, 882, 882, 882, 883, 883, 883, 884,
  884, 885, 885, 885, 886, 886, 886, 887, 887, 887, 888, 888, 888, 889, 889, 890,
  890, 890, 891, 891, 891, 892, 892, 892, 893, 893, 893, 894, 894, 894, 895, 895,
  895, 896, 896, 896, 897, 897, 897, 898, 898, 899, 899, 899, 900, 900, 900, 900,
  901, 901, 901, 902, 902, 902, 903, 903, 903, 904, 904, 904, 905, 905, 905, 906,
  906, 906, 907, 907, 907, 908, 908, 908, 909, 909, 909, 909, 910, 910, 910, 911,
  911, 911, 912, 912, 912, 913, 913, 913, 913, 914, 914, 914, 915, 915, 915, 916,
  916, 916, 916, 917, 917, 917, 918, 918, 918, 918, 919, 919, 919, 920, 920, 920,
  920, 921, 921, 921, 922, 922, 922, 922, 923, 923, 923, 924, 924, 924, 924, 925,
  925, 925, 926, 926, 926, 926, 927, 927, 927, 928, 928, 928, 928, 929, 929, 929,
  929, 930, 930, 930, 930, 931, 931, 931, 932, 932, 932, 932, 933, 933, 933, 933,
  934, 934, 934, 934, 935, 935, 935, 936, 936, 936, 936, 937, 937, 937, 937, 938,
  938, 938, 938, 939, 939, 939, 939, 940, 940, 940, 940, 941, 941, 941, 941, 942,
  942, 942, 942, 943, 943, 943, 943, 944, 944, 944, 944, 945, 945, 945, 945, 946,
  946, 946, 946, 947, 947, 947, 947, 947, 948, 948, 948, 948, 949, 949, 949, 949,
  950, 950, 950, 950, 951, 951, 951, 951, 952, 952, 952, 952, 952, 953, 953, 953,
  953, 954, 954, 954, 954, 955, 955, 955, 955, 955, 956, 956, 956, 956, 957, 957,
  957, 957, 957, 958, 958, 958, 958, 959, 959, 959, 959, 959, 960, 960, 960, 960,
  961, 961, 961, 961, 961, 962, 962, 962, 962, 963, 963, 963, 963, 963, 964, 964,
  964, 964, 964, 965, 965, 965, 965, 966, 966, 966, 966, 966, 967, 967, 967, 967,
  967, 968, 968, 968, 968, 968, 969, 969, 969, 969, 969, 970, 970, 970, 970, 971,
  971, 971, 971, 971, 972, 972, 972, 972, 972, 973, 973, 973, 973, 973, 974, 974,
  974, 974, 974, 975, 975, 975, 975, 975, 976, 976, 976, 976, 976, 977, 977, 977,
  977, 977, 978, 978, 978, 978, 978, 978, 979, 979, 979, 979, 979, 980, 980, 980,
  980, 980, 981, 981, 981, 981, 981, 982, 982, 982, 982, 982, 983, 983, 983, 983,
  983, 983, 984, 984, 984, 984, 984, 985, 985, 985, 985, 985, 986, 986, 986, 986,
  986, 986, 987, 987, 987, 987, 987, 988, 988, 988, 988, 988, 988, 989, 989, 989,
  989, 989, 990, 990, 990, 990, 990, 990, 991, 991, 991, 991, 991, 991, 992, 992,
  992, 992, 992, 993, 993, 993, 993, 993, 993, 994, 994, 994, 994, 994, 994, 995,
  995, 995, 995, 995, 996, 996, 996, 996, 996, 996, 997, 997, 997, 997, 997, 997,
  998, 998, 998, 998, 998, 998, 999, 999, 999, 999, 999, 999, 1000, 1000, 1000, 1000,
  1000, 1000, 1001, 1001, 1001, 1001, 1001, 1001, 1002, 1002, 1002, 1002, 1002, 1002, 1003, 1003,
  1003, 1003, 1003, 1003, 1004, 1004, 1004, 1004, 1004, 1004, 1005, 1005, 1005, 1005, 1005, 1005,
  1006, 1006, 1006, 1006, 1006, 1006, 1007, 1007, 1007, 1007, 1007, 1007, 1007, 1008, 1008, 1008,
  1008, 1008, 1008, 1009, 1009, 1009, 1009, 1009, 1009, 1010, 1010, 1010, 1010, 1010, 1010, 1010,
  1011, 1011, 1011, 1011, 1011, 1011, 1012, 1012, 1012, 1012, 1012, 1012, 1013, 1013, 1013, 1013,
  1013, 1013, 1013, 1014, 1014, 1014, 1014, 1014, 1014, 1015, 1015, 1015, 1015, 1015, 1015, 1015,
  1016, 1016, 1016, 1016, 1016, 1016, 1016, 1017, 1017, 1017, 1017, 1017, 1017, 1018, 1018, 1018,
  1018, 1018, 1018, 1018, 1019, 1019, 1019, 1019, 1019, 1019, 1019, 1020, 1020, 1020, 1020, 1020,
  1020, 1020, 1021, 1021, 1021, 1021, 1021, 1021, 1021, 1022, 1022, 1022, 1022, 1022, 1022, 1023
};

// LDR correction (sqrt)
// Function: y = sqrt(x)*1023/sqrt(x);
const uint16_t PROGMEM LDR_corr_sqrt[] = {
  0, 31, 45, 55, 63, 71, 78, 84, 90, 95, 101, 106, 110, 115, 119, 123,
  127, 131, 135, 139, 143, 146, 150, 153, 156, 159, 163, 166, 169, 172, 175, 178,
  180, 183, 186, 189, 191, 194, 197, 199, 202, 204, 207, 209, 212, 214, 216, 219,
  221, 223, 226, 228, 230, 232, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253,
  255, 257, 259, 261, 263, 265, 267, 269, 271, 273, 275, 276, 278, 280, 282, 284,
  286, 287, 289, 291, 293, 294, 296, 298, 300, 301, 303, 305, 306, 308, 310, 311,
  313, 315, 316, 318, 319, 321, 323, 324, 326, 327, 329, 330, 332, 333, 335, 336,
  338, 339, 341, 342, 344, 345, 347, 348, 350, 351, 353, 354, 356, 357, 359, 360,
  361, 363, 364, 366, 367, 368, 370, 371, 372, 374, 375, 377, 378, 379, 381, 382,
  383, 385, 386, 387, 389, 390, 391, 393, 394, 395, 396, 398, 399, 400, 402, 403,
  404, 405, 407, 408, 409, 410, 412, 413, 414, 415, 417, 418, 419, 420, 421, 423,
  424, 425, 426, 427, 429, 430, 431, 432, 433, 435, 436, 437, 438, 439, 440, 442,
  443, 444, 445, 446, 447, 448, 450, 451, 452, 453, 454, 455, 456, 457, 459, 460,
  461, 462, 463, 464, 465, 466, 467, 468, 470, 471, 472, 473, 474, 475, 476, 477,
  478, 479, 480, 481, 482, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494,
  495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510,
  511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526,
  527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 539, 540, 541,
  542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 553, 554, 555, 556,
  557, 558, 559, 560, 561, 562, 563, 564, 564, 565, 566, 567, 568, 569, 570, 571,
  572, 573, 573, 574, 575, 576, 577, 578, 579, 580, 581, 581, 582, 583, 584, 585,
  586, 587, 588, 588, 589, 590, 591, 592, 593, 594, 594, 595, 596, 597, 598, 599,
  600, 600, 601, 602, 603, 604, 605, 606, 606, 607, 608, 609, 610, 611, 611, 612,
  613, 614, 615, 616, 616, 617, 618, 619, 620, 621, 621, 622, 623, 624, 625, 625,
  626, 627, 628, 629, 630, 630, 631, 632, 633, 634, 634, 635, 636, 637, 638, 638,
  639, 640, 641, 642, 642, 643, 644, 645, 646, 646, 647, 648, 649, 649, 650, 651,
  652, 653, 653, 654, 655, 656, 657, 657, 658, 659, 660, 660, 661, 662, 663, 664,
  664, 665, 666, 667, 667, 668, 669, 670, 670, 671, 672, 673, 673, 674, 675, 676,
  676, 677, 678, 679, 679, 680, 681, 682, 682, 683, 684, 685, 685, 686, 687, 688,
  688, 689, 690, 691, 691, 692, 693, 694, 694, 695, 696, 697, 697, 698, 699, 700,
  700, 701, 702, 702, 703, 704, 705, 705, 706, 707, 708, 708, 709, 710, 710, 711,
  712, 713, 713, 714, 715, 715, 716, 717, 718, 718, 719, 720, 720, 721, 722, 723,
  723, 724, 725, 725, 726, 727, 727, 728, 729, 730, 730, 731, 732, 732, 733, 734,
  734, 735, 736, 737, 737, 738, 739, 739, 740, 741, 741, 742, 743, 743, 744, 745,
  745, 746, 747, 748, 748, 749, 750, 750, 751, 752, 752, 753, 754, 754, 755, 756,
  756, 757, 758, 758, 759, 760, 760, 761, 762, 762, 763, 764, 764, 765, 766, 766,
  767, 768, 768, 769, 770, 770, 771, 772, 772, 773, 774, 774, 775, 776, 776, 777,
  778, 778, 779, 780, 780, 781, 782, 782, 783, 784, 784, 785, 786, 786, 787, 788,
  788, 789, 789, 790, 791, 791, 792, 793, 793, 794, 795, 795, 796, 797, 797, 798,
  798, 799, 800, 800, 801, 802, 802, 803, 804, 804, 805, 805, 806, 807, 807, 808,
  809, 809, 810, 811, 811, 812, 812, 813, 814, 814, 815, 816, 816, 817, 817, 818,
  819, 819, 820, 821, 821, 822, 822, 823, 824, 824, 825, 826, 826, 827, 827, 828,
  829, 829, 830, 830, 831, 832, 832, 833, 834, 834, 835, 835, 836, 837, 837, 838,
  838, 839, 840, 840, 841, 841, 842, 843, 843, 844, 845, 845, 846, 846, 847, 848,
  848, 849, 849, 850, 851, 851, 852, 852, 853, 854, 854, 855, 855, 856, 857, 857,
  858, 858, 859, 860, 860, 861, 861, 862, 862, 863, 864, 864, 865, 865, 866, 867,
  867, 868, 868, 869, 870, 870, 871, 871, 872, 873, 873, 874, 874, 875, 875, 876,
  877, 877, 878, 878, 879, 880, 880, 881, 881, 882, 882, 883, 884, 884, 885, 885,
  886, 886, 887, 888, 888, 889, 889, 890, 890, 891, 892, 892, 893, 893, 894, 894,
  895, 896, 896, 897, 897, 898, 898, 899, 900, 900, 901, 901, 902, 902, 903, 904,
  904, 905, 905, 906, 906, 907, 908, 908, 909, 909, 910, 910, 911, 911, 912, 913,
  913, 914, 914, 915, 915, 916, 917, 917, 918, 918, 919, 919, 920, 920, 921, 922,
  922, 923, 923, 924, 924, 925, 925, 926, 926, 927, 928, 928, 929, 929, 930, 930,
  931, 931, 932, 933, 933, 934, 934, 935, 935, 936, 936, 937, 937, 938, 939, 939,
  940, 940, 941, 941, 942, 942, 943, 943, 944, 945, 945, 946, 946, 947, 947, 948,
  948, 949, 949, 950, 950, 951, 952, 952, 953, 953, 954, 954, 955, 955, 956, 956,
  957, 957, 958, 958, 959, 960, 960, 961, 961, 962, 962, 963, 963, 964, 964, 965,
  965, 966, 966, 967, 968, 968, 969, 969, 970, 970, 971, 971, 972, 972, 973, 973,
  974, 974, 975, 975, 976, 976, 977, 978, 978, 979, 979, 980, 980, 981, 981, 982,
  982, 983, 983, 984, 984, 985, 985, 986, 986, 987, 987, 988, 988, 989, 989, 990,
  990, 991, 992, 992, 993, 993, 994, 994, 995, 995, 996, 996, 997, 997, 998, 998,
  999, 999, 1000, 1000, 1001, 1001, 1002, 1002, 1003, 1003, 1004, 1004, 1005, 1005, 1006, 1006,
  1007, 1007, 1008, 1008, 1009, 1009, 1010, 1010, 1011, 1011, 1012, 1012, 1013, 1013, 1014, 1014,
  1015, 1015, 1016, 1016, 1017, 1017, 1018, 1018, 1019, 1019, 1020, 1020, 1021, 1021, 1022, 1023
};
