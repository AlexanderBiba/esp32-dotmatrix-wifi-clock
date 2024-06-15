
#ifndef APP_WEB_PAGE_H
#define APP_WEB_PAGE_H
const char WebPage[] PROGMEM = R"html(<!DOCTYPE html>
<html>
  <head>
    <title>Wifi Dotmatrix Display</title>
    <script>
      function submitModeOfOperation() {
        let strLine = "";
        const nocache = "/&nocache=" + Math.random() * 1000000;
        const form = document.getElementById("operation_mode_form");
        const request = new XMLHttpRequest();

        if (form["message-radio"].checked) {
          strLine = "&MSG=" + form["message-input"].value;
        } else if (form["clk-radio"].checked) {
          strLine = "&CLK";
        } else if (form["weather-radio"].checked) {
          strLine = "&WEATHER";
        } else if (form["ticker-radio"].checked) {
          strLine = "&TICKER=" + form["ticker-input"].value;
        }

        request.open("GET", strLine + nocache, false);
        request.send(null);
      }

      function setCntl(settings) {
        const strLine = `&CNTL=${encodeURI(JSON.stringify(settings))}`;
        const nocache = "/&nocache=" + Math.random() * 1000000;
        const request = new XMLHttpRequest();

        request.open("GET", strLine + nocache, false);
        request.send(null);
      }

      function populateTimezones() {
        const timezoneElem = document.getElementById("timezone");
        fetch("https://worldtimeapi.org/api/timezone")
          .then((response) => response.json())
          .then((data) => {
            data.forEach((timezone) => {
              const opt = document.createElement("option");
              opt.value = timezone;
              opt.innerHTML = timezone;
              timezoneElem.appendChild(opt);
            });
          })
          .catch((error) => console.error("Error fetching timezones:", error));
      }

      window.onload = function () {
        populateTimezones();
      };
    </script>
  </head>
  <body>
    <h1>Set Mode Of Operation</h1>
    <form id="operation_mode_form">
      <fieldset>
        <legend>Select Mode</legend>
        <input type="radio" id="message-radio" name="operation-mode" />
        <label for="message-radio">
          Message:
          <input
            type="text"
            id="message-input"
            name="message-input"
            maxlength="255"
            required
          />
        </label>
        <br /><br />
        <input type="radio" id="clk-radio" name="operation-mode" />
        <label for="clk-radio">Clock</label>
        <br /><br />
        <input type="radio" id="weather-radio" name="operation-mode" />
        <label for="weather-radio">Weather</label>
        <br /><br />
        <input type="radio" id="ticker-radio" name="operation-mode" />
        <label for="ticker-radio">
          Ticker:
          <input
            type="text"
            id="ticker-input"
            name="ticker-input"
            maxlength="255"
            required
          />
        </label>
        <br /><br />
      </fieldset>
      <input
        type="submit"
        value="Submit"
        onclick="submitModeOfOperation(); return false;"
      />
    </form>

    <h1>Settings</h1>
    <form id="settings_form">
      <fieldset>
        <legend>General Settings</legend>
        <label for="timezone">
          Timezone:
          <select id="timezone"></select>
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ timezone: document.getElementById('timezone').value }); return false;"
        />
        <br /><br />
        <label for="brightness">
          Brightness:
          <input
            type="number"
            id="brightness"
            name="brightness"
            min="0"
            max="15"
            required
          />
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ brightness: document.getElementById('brightness').value }); return false;"
        />
      </fieldset>
      <br />
      <fieldset>
        <legend>Weather Settings</legend>
        <label for="weather-latitude">
          Weather Location:
          <label
            >Latitude:
            <input
              id="weather-latitude"
              name="weather-latitude"
              maxlength="32"
              required
          /></label>
          <label
            >Longitude:
            <input
              id="weather-longitude"
              name="weather-longitude"
              maxlength="32"
              required
          /></label>
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ latitude: document.getElementById('weather-latitude').value, longitude: document.getElementById('weather-longitude').value }); return false;"
        />
        <br /><br />
        <label>
          Weather Units:
          <input
            type="radio"
            id="celsius-radio"
            name="weather-units"
            value="c"
            required
          /><label for="celsius-radio">Celsius</label>
          <input
            type="radio"
            id="fahrenheit-radio"
            name="weather-units"
            value="f"
            required
          /><label for="fahrenheit-radio">Fahrenheit</label>
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ weatherUnits: document.querySelector('input[name=weather-units]:checked').value }); return false;"
        />
      </fieldset>
      <br />
      <fieldset>
        <legend>API Key</legend>
        <label for="finazon-api-key">
          Finazon.io API Key:
          <input
            id="finazon-api-key"
            name="finazon-api-key"
            maxlength="255"
            required
          />
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ finazonApiKey: document.getElementById('finazon-api-key').value }); return false;"
        />
      </fieldset>
    </form>
  </body>
</html>
)html";
#endif