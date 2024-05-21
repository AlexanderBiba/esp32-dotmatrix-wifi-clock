// Use the MD_MAX72XX library to scroll text on the display
// received through the ESP32 WiFi interface.
//
// Demonstrates the use of the callback function to control what
// is scrolled on the display text. User can enter text through
// a web browser and this will display as a scrolling message on
// the display.
//
// IP address for the ESP32 is displayed on the scrolling display
// after startup initialization and connected to the WiFi network.
//
// Connections for ESP32 hardware SPI are:
// Vcc       3.3V - A few matrices seem to work at 3.3V
// GND       GND
// DIN       VSPI_MOSI
// CS or LD  VSPI_CS
// CLK       VSPI_SCK
//

#include <ctime>
#include <WiFi.h>
#include <WiFiServer.h>
#include <MD_MAX72xx.h>
#include <HttpClient.h>
#include <ArduinoJson.h>
#include <Timezone.h>

#define PRINT_CALLBACK 0
#define DEBUG 1
#define LED_HEARTBEAT 0

#if DEBUG
#define PRINT(s, v)     \
  {                     \
    Serial.print(F(s)); \
    Serial.print(v);    \
  }
#define PRINTS(s)       \
  {                     \
    Serial.print(F(s)); \
  }
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

#if LED_HEARTBEAT
#define HB_LED D2
#define HB_LED_TIME 500 // in milliseconds
#endif

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 4

// GPIO pins
#define CLK_PIN 14  // VSPI_SCK
#define DATA_PIN 13 // VSPI_MOSI
#define CS_PIN 12   // VSPI_SS

// SPI hardware interface
// MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// WiFi login parameters - network name and password
const char ssid[] = "Hogwarts";
const char password[] = "shakshuka";

// WiFi Server object and parameters
WiFiServer server(80);

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule myDST = {"EDT", Second, Sun, Mar, 2, -240}; // Daylight time = UTC - 4 hours
TimeChangeRule mySTD = {"EST", First, Sun, Nov, 2, -300};  // Standard time = UTC - 5 hours
Timezone myTZ(myDST, mySTD);

// Global message buffers shared by Wifi and Scrolling functions
const uint8_t MESG_SIZE = 255;
const uint8_t CHAR_SPACING = 1;
const uint8_t SCROLL_DELAY = 75;

enum FnType
{
  MSG,
  CLK
};

FnType operationMode = CLK;
char curMessage[MESG_SIZE];
char newMessage[MESG_SIZE];
bool newMessageAvailable = false;
long epoch = 0;

const char WebResponse[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";

const char WebPage[] =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<title>Wifi Dotmatrix Display</title>"

    "<script>"
    "strLine = \"\";"

    "function submit()"
    "{"
    "  nocache = \"/&nocache=\" + Math.random() * 1000000;"
    "  var request = new XMLHttpRequest();"
    "  if (document.getElementById(\"txt_form\").msg.checked)"
    "  {"
    "    strLine = \"&MSG=\" + document.getElementById(\"txt_form\").Message.value;"
    "  }"
    "  else if (document.getElementById(\"txt_form\").clk.checked)"
    "  {"
    "    strLine = \"&CLK\""
    "  }"
    "  request.open(\"GET\", strLine + nocache, false);"
    "  request.send(null);"
    "}"
    "</script>"
    "</head>"

    "<body>"
    "<p><b>Wifi Clock Set Function</b></p>"

    "<form id=\"txt_form\" name=\"frmText\">"
    "<input type=\"radio\" id=\"msg\" name=\"fn\" />"
    "<label for=\"msg\">Message: <input type=\"text\" name=\"Message\" maxlength=\"255\"></label><br><br>"
    "<input type=\"radio\" id=\"clk\" name=\"fn\" />"
    "<label for=\"clk\">Clock</label><br><br>"
    "</form>"
    "<br>"
    "<input type=\"submit\" value=\"Submit\" onclick=\"submit()\">"
    "</body>"
    "</html>";

const char *err2Str(wl_status_t code)
{
  switch (code)
  {
  case WL_IDLE_STATUS:
    return ("IDLE");
    break; // WiFi is in process of changing between statuses
  case WL_NO_SSID_AVAIL:
    return ("NO_SSID_AVAIL");
    break; // case configured SSID cannot be reached
  case WL_CONNECTED:
    return ("CONNECTED");
    break; // successful connection is established
  case WL_CONNECT_FAILED:
    return ("CONNECT_FAILED");
    break; // password is incorrect
  case WL_DISCONNECTED:
    return ("CONNECT_FAILED");
    break; // module is not configured in station mode
  default:
    return ("??");
  }
}

uint8_t htoi(char c)
{
  c = toupper(c);
  if ((c >= '0') && (c <= '9'))
    return (c - '0');
  if ((c >= 'A') && (c <= 'F'))
    return (c - 'A' + 0xa);
  return (0);
}

boolean getText(char *szMesg, char *psz, uint8_t len)
{
  boolean isValid = false; // text received flag
  char *pStart, *pEnd;     // pointer to start and end of text

  // handle message mode
  pStart = strstr(szMesg, "/&MSG=");

  if (pStart != NULL)
  {
    pStart += 6; // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      while (pStart != pEnd)
      {
        if ((*pStart == '%') && isxdigit(*(pStart + 1)))
        {
          // replace %xx hex code with the ASCII character
          char c = 0;
          pStart++;
          c += (htoi(*pStart++) << 4);
          c += htoi(*pStart++);
          *psz++ = c;
        }
        else
          *psz++ = *pStart++;
      }

      *psz = '\0'; // terminate the string
      operationMode = MSG;
      isValid = true;
    }
  }

  // handle clock mode
  pStart = strstr(szMesg, "/&CLK");

  if (pStart != NULL)
  {
    operationMode = CLK;
    isValid = true;
  }

  return (isValid);
}

void handleWiFi(void)
{
  static enum { S_IDLE,
                S_WAIT_CONN,
                S_READ,
                S_EXTRACT,
                S_RESPONSE,
                S_DISCONN } state = S_IDLE;
  static char szBuf[1024];
  static uint16_t idxBuf = 0;
  static WiFiClient client;
  static uint32_t timeStart;

  switch (state)
  {
  case S_IDLE: // initialize
    PRINTS("\nS_IDLE");
    idxBuf = 0;
    state = S_WAIT_CONN;
    break;

  case S_WAIT_CONN: // waiting for connection
  {
    client = server.accept();
    if (!client)
      break;
    if (!client.connected())
      break;

#if DEBUG
    char szTxt[20];
    sprintf(szTxt, "%d:%d:%d:%d", client.remoteIP()[0], client.remoteIP()[1], client.remoteIP()[2], client.remoteIP()[3]);
    PRINT("\nNew client @ ", szTxt);
#endif

    timeStart = millis();
    state = S_READ;
  }
  break;

  case S_READ: // get the first line of data
    PRINTS("\nS_READ");
    while (client.available())
    {
      char c = client.read();
      if ((c == '\r') || (c == '\n'))
      {
        szBuf[idxBuf] = '\0';
        client.flush();
        PRINT("\nRecv: ", szBuf);
        state = S_EXTRACT;
      }
      else
        szBuf[idxBuf++] = (char)c;
    }
    if (millis() - timeStart > 1000)
    {
      PRINTS("\nWait timeout");
      state = S_DISCONN;
    }
    break;

  case S_EXTRACT: // extract data
    PRINTS("\nS_EXTRACT");
    // Extract the string from the message if there is one
    newMessageAvailable = getText(szBuf, newMessage, MESG_SIZE);
    PRINT("\nNew Msg: ", newMessage);
    state = S_RESPONSE;
    break;

  case S_RESPONSE: // send the response to the client
    PRINTS("\nS_RESPONSE");
    // Return the response to the client (web page)
    client.print(WebResponse);
    client.print(WebPage);
    state = S_DISCONN;
    break;

  case S_DISCONN: // disconnect client
    PRINTS("\nS_DISCONN");
    client.flush();
    client.stop();
    state = S_IDLE;
    break;

  default:
    state = S_IDLE;
  }
}

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col)
// Callback function for data that is being scrolled off the display
{
#if PRINT_CALLBACK
  Serial.print("\n cb ");
  Serial.print(dev);
  Serial.print(' ');
  Serial.print(t);
  Serial.print(' ');
  Serial.println(col);
#endif
}

uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t)
// Callback function for data that is required for scrolling into the display
{
  static enum { S_IDLE,
                S_NEXT_CHAR,
                S_SHOW_CHAR,
                S_SHOW_SPACE } state = S_IDLE;
  static char *p;
  static uint16_t curLen, showLen;
  static uint8_t cBuf[8];
  uint8_t colData = 0;

  // finite state machine to control what we do on the callback
  switch (state)
  {
  case S_IDLE: // reset the message pointer and check for new message to load
    PRINTS("\nS_IDLE");
    p = curMessage;          // reset the pointer to start of message
    if (newMessageAvailable) // there is a new message waiting
    {
      switch (operationMode)
      {
      case MSG:
        PRINT("\nNew message - ", newMessage);
        strcpy(curMessage, newMessage); // copy it in
        newMessageAvailable = false;
        break;
      case CLK:
        break;
      }
    }
    state = S_NEXT_CHAR;
    break;

  case S_NEXT_CHAR: // Load the next character from the font table
    PRINT("\nS_NEXT_CHAR ", *p);
    if (*p == '\0')
      state = S_IDLE;
    else
    {
      showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state = S_SHOW_CHAR;
    }
    break;

  case S_SHOW_CHAR: // display the next part of the character
    PRINTS("\nS_SHOW_CHAR");
    colData = cBuf[curLen++];
    if (curLen < showLen)
      break;

    // set up the inter character spacing
    showLen = (*p != '\0' ? CHAR_SPACING : (MAX_DEVICES * COL_SIZE) / 2);
    curLen = 0;
    state = S_SHOW_SPACE;
    // fall through

  case S_SHOW_SPACE: // display inter-character spacing (blank column)
    PRINT("\nS_SHOW_SPACE: ", curLen);
    PRINT("/", showLen);
    curLen++;
    if (curLen == showLen)
      state = S_NEXT_CHAR;
    break;

  default:
    state = S_IDLE;
  }

  return (colData);
}

void scrollText(void)
{
  static uint32_t prevTime = 0;

  // Is it time to scroll the text?
  if (millis() - prevTime >= SCROLL_DELAY)
  {
    mx.transform(MD_MAX72XX::TSL); // scroll along - the callback will load all the data
    prevTime = millis();           // starting point for next time
  }
}

void getTimeFromServer()
{
  PRINTS("\ngetting time from server");

  static WiFiClient wifiClient = WiFiClient();
  HttpClient client = HttpClient(wifiClient, "worldtimeapi.org");
  client.get("/api/timezone/America/New_York");
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  PRINT("\nStatus code: ", statusCode);
  PRINT("\nResponse: ", response);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error)
  {
    PRINT("\ndeserializeJson() failed: ", error.f_str());
    return;
  }

  long unixtime = doc["unixtime"];
  PRINT("\nUnix time: ", unixtime);
  epoch = unixtime - (millis() / 1000);
}

void setCharOverride(int pos, char c)
{
  if (c == '1')
  {
    mx.setChar(pos, '1');
    mx.setColumn(pos - 3, 0);
    mx.setColumn(pos - 4, 0);
  }
  else
  {
    mx.setChar(pos, c);
  }
}

void printTime(boolean reset)
{
  static enum { S_IDLE,
                S_RESET } state;
  static long time = epoch + (millis() / 1000);

  if (reset)
  {
    state = S_RESET;
  }

  switch (state)
  {
  case S_RESET:
    PRINTS("\nS_RESET");
    if (WiFi.status() == WL_CONNECTED)
    {
      getTimeFromServer();
      state = S_IDLE;
      mx.clear();
    }
    break;
  case S_IDLE:
    const uint8_t charSize = 5;

    long newTime = epoch + (millis() / 1000);
    if (newTime != time)
    {
      time = newTime;
      if (WiFi.status() == WL_CONNECTED && (newTime % 3600) == 0) // sync time every hour
      {
        getTimeFromServer();
      }

      char timeStr[sizeof("hh:mm:ss")];
      time_t nyTime = myTZ.toLocal(newTime);
      strftime((timeStr), sizeof(timeStr), "%T", gmtime(&nyTime));

      uint8_t pos = -1;

      // seconds
      pos += charSize;
      setCharOverride(pos, timeStr[7]);
      pos += charSize;
      setCharOverride(pos, timeStr[6]);

      // delim
      pos += 1;
      setCharOverride(pos, 0);

      // minutes
      pos += charSize;
      setCharOverride(pos, timeStr[4]);
      pos += charSize;
      setCharOverride(pos, timeStr[3]);

      // delim
      pos += 1;
      setCharOverride(pos, 0);

      // hours
      pos += charSize;
      setCharOverride(pos, timeStr[1]);
      pos += charSize;
      setCharOverride(pos, timeStr[0]);
    }
    break;
  }
}

void setup(void)
{
#if DEBUG
  Serial.begin(115200);
  PRINTS("\n[MD_MAX72XX WiFi Message Display]\nType a message for the scrolling display from your internet browser");
#endif

  // Display initialization
  PRINTS("\nInitializing Display");
  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);

  curMessage[0] = newMessage[0] = '\0';

  // Connect to and initialize WiFi network
  PRINT("\nConnecting to ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    PRINT("\n", err2Str(WiFi.status()));
    uint32_t t = millis();
    while (millis() - t <= 1000)
      yield();
  }
  PRINTS("\nWiFi connected");

  // Start the server
  PRINTS("\nStarting Server");
  server.begin();

  // Set up first message as the IP address
  sprintf(curMessage, "%d:%d:%d:%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  PRINT("\nAssigned IP ", curMessage);

  getTimeFromServer();
}

void loop(void)
{
  FnType prevOperationMode = operationMode;

  handleWiFi();
  switch (operationMode)
  {
  case MSG:
    scrollText();
    break;
  case CLK:
    printTime(prevOperationMode != operationMode);
    break;
  }
}
