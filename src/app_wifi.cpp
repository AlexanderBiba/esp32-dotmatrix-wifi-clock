#include <WiFiServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>

#include "app_wifi.h"
#include "utils.h"

OperationMode operationMode;

// WiFi Server object and parameters
WiFiServer server(80);

const char WebResponse[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";

const char WebPage[] =
    R"html(
<!DOCTYPE html>
<html>
  <head>
    <title>Wifi Dotmatrix Display</title>
    <script>
      strLine = "";
      function submit() {
        nocache = "/&nocache=" + Math.random() * 1000000;
        var request = new XMLHttpRequest();
        if (document.getElementById("txt_form").msg.checked) {
          strLine =
           "&MSG=" + document.getElementById("txt_form").Message.value;
        } else if (document.getElementById("txt_form").clk.checked) {
          strLine = "&CLK";
        }
        request.open("GET", strLine + nocache, false);
        request.send(null);
      }
    </script>
  </head>

  <body>
    <p><b>Wifi Clock Set Function</b></p>
    <form id="txt_form" name="frmText">
      <input type="radio" id="msg" name="fn" />
      <label for="msg"
        >Message: <input type="text" name="Message" maxlength="255" /></label
      ><br /><br />
      <input type="radio" id="clk" name="fn" />
      <label for="clk">Clock</label><br /><br />
    </form>
    <br />
    <input type="submit" value="Submit" onclick="submit()" />
  </body>
</html>
)html";

uint8_t htoi(char c)
{
  c = toupper(c);
  if ((c >= '0') && (c <= '9'))
    return (c - '0');
  if ((c >= 'A') && (c <= 'F'))
    return (c - 'A' + 0xa);
  return (0);
}

OperationMode extractHttpContent(char *szMesg, char msgBuffer[MAX_MSG_SIZE])
{
  bool isValid = false;
  char *pStart, *pEnd, *psz = msgBuffer;

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
      operationMode = OperationMode::MSG;
      isValid = true;
    }
  }

  // handle clock mode
  pStart = strstr(szMesg, "/&CLK");

  if (pStart != NULL)
  {
    operationMode = OperationMode::CLK;
    isValid = true;
  }

  // if (!isValid)
  // {
  //   throw new std::invalid_argument("Invalid message received");
  // }

  return operationMode;
}

OperationMode handleWiFi(char msgBuffer[MAX_MSG_SIZE])
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
    PRINTS("S_IDLE");
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
    PRINT("New client @ ", szTxt);
#endif

    timeStart = millis();
    state = S_READ;
  }
  break;

  case S_READ: // get the first line of data
    PRINTS("S_READ");
    while (client.available())
    {
      char c = client.read();
      if ((c == '\r') || (c == '\n'))
      {
        szBuf[idxBuf] = '\0';
        client.flush();
        PRINT("Recv: ", szBuf);
        state = S_EXTRACT;
      }
      else
        szBuf[idxBuf++] = (char)c;
    }
    if (millis() - timeStart > 1000)
    {
      PRINTS("Wait timeout");
      state = S_DISCONN;
    }
    break;

  case S_EXTRACT: // extract data
    PRINTS("S_EXTRACT");
    // Extract the string from the message if there is one
    operationMode = extractHttpContent(szBuf, msgBuffer);
    state = S_RESPONSE;
    break;

  case S_RESPONSE: // send the response to the client
    PRINTS("S_RESPONSE");
    // Return the response to the client (web page)
    client.print(WebResponse);
    client.print(WebPage);
    state = S_DISCONN;
    break;

  case S_DISCONN: // disconnect client
    PRINTS("S_DISCONN");
    client.flush();
    client.stop();
    state = S_IDLE;
    break;

  default:
    state = S_IDLE;
  }

  return operationMode;
}

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

#define MDNS_DOMAIN "esp32"
void setupWiFi(char *localIp)
{
  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  wifiManager.autoConnect("DotMatrix Clock");
  PRINTS("WIFI Connected");

  // Initialize mDNS
  bool mdnsSuccess = MDNS.begin(MDNS_DOMAIN);
  if (!mdnsSuccess)
  {
    PRINTS("Error setting up MDNS responder!");
  }

  // Start the server
  PRINTS("Starting Server");
  server.begin();

  if (mdnsSuccess)
  {
    sprintf(localIp, "%d:%d:%d:%d - %s.local", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], MDNS_DOMAIN);
    PRINT("Assigned IP ", localIp);
    PRINT("MDNS ", MDNS_DOMAIN);
  }
  else
  {
    sprintf(localIp, "%d:%d:%d:%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    PRINT("Assigned IP ", localIp);
  }
}

void setDefaultOperationMode(OperationMode defaultOperationMode)
{
  operationMode = defaultOperationMode;
}