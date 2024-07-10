
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

        if (form["message-checkbox"].checked) {
          const message = form["message-input"].value;
          if (message.trim() === "") {
            alert("Message cannot be empty when selected.");
            return;
          }
          strLine += "/&MSG=" + message;
        }
        if (form["clock-checkbox"].checked) {
          strLine += "/&CLK";
        }
        if (form["date-checkbox"].checked) {
          strLine += "/&DATE";
        }
        if (form["weather-checkbox"].checked) {
          strLine += "/&WEATHER";
        }
        if (form["snake-checkbox"].checked) {
          strLine += "/&SNAKE";
        }

        fetch(strLine + nocache);
      }

      function setCntl(settings) {
        for (const key in settings) {
          if (
            settings[key] === "" ||
            settings[key] === undefined ||
            settings[key] === null
          ) {
            alert("Please fill out all fields.");
            return;
          }
        }
        const strLine = `&CNTL=${encodeURI(JSON.stringify(settings))}`;
        const nocache = "/&nocache=" + Math.random() * 1000000;
        fetch(strLine + nocache);
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

      function getSettings() {
        fetch("/&SETT")
          .then((response) => response.json())
          .then((data) => {
            document.getElementById("timezone").value = data.timezone;
            document.getElementById("brightness").value = data.brightness;
            document.getElementById("weather-latitude").value = data.latitude;
            document.getElementById("weather-longitude").value = data.longitude;
            document.querySelector(
              `input[name=weather-units][value=${data.weatherUnits}]`
            ).checked = true;
            data.activeCards.forEach((card) => {
              document.getElementById(
                `${card.toLowerCase()}-checkbox`
              ).checked = true;
            });
            document.getElementById("alarm-hour").value = data.alarmHour;
            document.getElementById("alarm-minute").value = data.alarmMinute;
            document.getElementById("alarm-enabled").checked =
              data.alarmEnabled;
          })
          .catch((error) => console.error("Error fetching settings:", error));
      }

      window.onload = function () {
        populateTimezones();
        getSettings();
      };
    </script>
  </head>
  <body>
    <h1>Set Mode Of Operation</h1>
    <form
      id="operation_mode_form"
      onsubmit="submitModeOfOperation(); return false;"
    >
      <fieldset>
        <legend>Select Mode</legend>
        <input type="checkbox" id="message-checkbox" name="operation-mode" />
        <label for="message-checkbox">
          Message:
          <input
            type="text"
            id="message-input"
            name="message-input"
            maxlength="255"
          />
        </label>
        <br /><br />
        <input type="checkbox" id="clock-checkbox" name="operation-mode" />
        <label for="clock-checkbox">Clock</label>
        <br /><br />
        <input type="checkbox" id="date-checkbox" name="operation-mode" />
        <label for="date-checkbox">Date</label>
        <br /><br />
        <input type="checkbox" id="weather-checkbox" name="operation-mode" />
        <label for="weather-checkbox">Weather</label>
        <br /><br />
        <input type="checkbox" id="snake-checkbox" name="operation-mode" />
        <label for="snake-checkbox">Snake</label>
        <br /><br />
      </fieldset>
      <input type="submit" value="Submit" />
    </form>

    <h1>Settings</h1>
    <form id="settings_form" onsubmit="setCntl(); return false;">
      <fieldset>
        <legend>General Settings</legend>
        <label for="timezone">
          Timezone:
          <select id="timezone"></select>
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ timezone: document.getElementById('timezone').value });"
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
          />
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ brightness: document.getElementById('brightness').value });"
        />
      </fieldset>
      <br />
      <fieldset>
        <legend>Weather Settings</legend>
        <label for="weather-latitude">
          Weather Location:
          <label
            >Latitude:
            <input id="weather-latitude" name="weather-latitude" maxlength="32"
          /></label>
          <label
            >Longitude:
            <input
              id="weather-longitude"
              name="weather-longitude"
              maxlength="32"
          /></label>
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ latitude: document.getElementById('weather-latitude').value, longitude: document.getElementById('weather-longitude').value });"
        />
        <br /><br />
        <label>
          Weather Units:
          <input
            type="radio"
            id="celsius-radio"
            name="weather-units"
            value="c"
          /><label for="celsius-radio">Celsius</label>
          <input
            type="radio"
            id="fahrenheit-radio"
            name="weather-units"
            value="f"
          /><label for="fahrenheit-radio">Fahrenheit</label>
        </label>
        <input
          type="submit"
          value="Set"
          onclick="setCntl({ weatherUnits: document.querySelector('input[name=weather-units]:checked').value });"
        />
      </fieldset>
      <br />
      <fieldset>
        <legend>Alarm Settings</legend>
        <label for="alarm-hour">Alarm Hour:</label>
        <input
          type="number"
          id="alarm-hour"
          name="alarm-hour"
          min="0"
          max="23"
        />
        <br /><br />
        <label for="alarm-minute">Alarm Minute:</label>
        <input
          type="number"
          id="alarm-minute"
          name="alarm-minute"
          min="0"
          max="59"
        />
        <br /><br />
        <input type="checkbox" id="alarm-enabled" name="alarm-enabled" />
        <label for="alarm-enabled">Enable Alarm</label>
        <br /><br />
        <input
          type="submit"
          value="Set Alarm"
          onclick="setCntl({
            alarmHour: document.getElementById('alarm-hour').value,
            alarmMinute: document.getElementById('alarm-minute').value,
            alarmEnabled: document.getElementById('alarm-enabled').checked
          });"
        />
        <br /><br />
        <input type="button" value="Stop Alarm" onclick="fetch('/&STOP');" />
      </fieldset>
    </form>
  </body>
</html>
)html";
#endif