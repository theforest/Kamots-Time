/* Kamots Time Pebble JS
Weather portions based from: https://github.com/pebble/pebble-sdk-examples/tree/4c5181c8a8b291cc90fd5f2b9465089851564059/pebblekit-js/weather/src/js
...which is Copyright (C) 2013 Pebble Technology and licensed under the MIT License

Modifications to weather portions and all other code...
Copyright 2015 kamotswind

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing permissions and
limitations under the License.
*/

var colretries = 4;
var optretries = 4;
var KEY_CONFVER = 52668701; // configuration version ID (no this has nothing to do with the keys in main.h)
var KEY_CONFDAT = 52668711; // configuration data (using the same key IDs for localStorage just for my own OCD)
var wxreq = 2;
var cachedcoords = null;

Pebble.addEventListener("showConfiguration",
  function(e) {
    var url = "http://theforest.us/kamotstimecfgCOLOR.php#";
    if(Pebble.getActiveWatchInfo) {
      var watch = Pebble.getActiveWatchInfo();
      if(watch.platform == 'aplite') url = "http://theforest.us/kamotstimecfgBW.php#";
    }

    if (checkforlocalstorage()) if (parseInt(localStorage.getItem(KEY_CONFVER),10) >= 3) {
      url = url + encodeURIComponent(localStorage.getItem(KEY_CONFDAT));
      console.log('Loaded config from localStorage.');
    }
    Pebble.openURL(url);
    console.log('Opened configuration window.');
  }
);

Pebble.addEventListener('webviewclosed', function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ', JSON.stringify(configuration));

    // Send to Pebble (with retries)
    colretries = 0;
    optretries = 0;
    sendconfcolors(configuration);
    sendconfoptions(configuration);
  
    // Pebble.showSimpleNotificationOnPebble("Kamots Time", "Watchface settings changed! Press the back button.");

    if (checkforlocalstorage()) {
      // Store in localStorage for retrieval later
      localStorage.setItem(KEY_CONFVER, "5"); // placeholder, still need to write version upgrade code
      localStorage.setItem(KEY_CONFDAT, JSON.stringify(configuration)); // save configuration
    }
});

function sendconfcolors(conf) {
    var runtime = Date.now() / 1000;
    Pebble.sendAppMessage({ "C_HH": parseInt(conf.color_hh.substring(1,7),16),
                            "C_MH": parseInt(conf.color_mh.substring(1,7),16),
                            "C_SH": parseInt(conf.color_sh.substring(1,7),16),
                            "C_HM": parseInt(conf.color_hm.substring(1,7),16),
                            "C_WB": parseInt(conf.color_wb.substring(1,7),16),
                            "C_WO": parseInt(conf.color_wo.substring(1,7),16),
                            "C_SB": parseInt(conf.color_sb.substring(1,7),16) },
                            function(e) {
                              console.log("Send colors successful @", runtime);
                            }, function(e) {
                              console.log("Send colors failed! @", runtime);
                              if (colretries++ < 4) sendconfcolors(conf);
                            }
                           );
}

function sendconfoptions(conf) {
    var runtime = Date.now() / 1000;
    var digitime = 0;
    var bt_stats = 0;
    var sechand = 0;
    var dtzulu = 0;
    var weather = 0;
    var wxformat = 1;
    var digitime_string = conf.digitime;
    var bt_stats_string = conf.bt_stats;
    var sechand_string = conf.sechand;
    var dtzulu_string = conf.dtzulu;
    var weather_string = conf.wx;
    var wxformat_string = conf.wx_format;
    if(digitime_string.match(/^on/)) digitime = 1;
    if(bt_stats_string.match(/^on/)) bt_stats = 1;
    if(sechand_string.match(/^on/)) sechand = 1;
    if(dtzulu_string.match(/^on/)) dtzulu = 1;
    if(weather_string.match(/^on/)) weather = 1;
    if(wxformat_string.match(/^off/)) wxformat = 0;

    Pebble.sendAppMessage({ "HM_C": parseInt(conf.hm_count,10),
                            "D_DT": digitime,
                            "D_BT": bt_stats,
                            "D_SH": sechand,
                            "DT_Z": dtzulu,
                            "D_WX": weather,
                            "WFRQ": parseInt(conf.wx_freq,10),
                            "WFMT": wxformat },
                            function(e) {
                              console.log("Send options successful @", runtime);
                            }, function(e) {
                              console.log("Send options failed! @", runtime);
                              if (optretries++ < 4) sendconfoptions(conf);
                            }
                           );
}

function checkforlocalstorage() {
    var lstest = 'thingsandjunk';
    try {
        localStorage.setItem(lstest, lstest);
        localStorage.removeItem(lstest);
        return true;
    } catch(e) {
        return false;
    }
}

function convertIconToGraphic(icon, wxid, clouds) {
  var ret = 0;
  switch(icon) {
    case "01d":
      ret = 1;
      break;
    case "01n":
      ret = 101;
      break;
    case "02d":
      ret = 2;
      break;
    case "02n":
      ret = 102;
      break;
    case "03d":
      ret = 3;
      break;
    case "03n":
      ret = 103;
      break;
    case "04d":
      ret = 4;
      break;
    case "04n":
      ret = 104;
      break;
    case "09d":
      ret = 9;
      break;
    case "09n":
      ret = 109;
      break;
    case "10d":
      ret = 10;
      break;
    case "10n":
      ret = 110;
      break;
    case "11d":
      ret = 11;
      break;
    case "11n":
      ret = 111;
      break;
    case "13d":
      ret = 13;
      break;
    case "13n":
      ret = 113;
      break;
    case "50d":
      ret = 50;
      break;
    case "50n":
      ret = 150;
      break;
    default:
      ret = 0;
  }
  switch(wxid) {
    case 501:
    case 502:
    case 503:
    case 504:
      if((ret == 10 || ret == 110) && clouds >= 70) ret -= 1;
      break;
    case 520:
      if(ret == 9 || ret == 109) ret += 1;
      break;
  }
  return ret;
}

function fetchWeather(latitude, longitude) {
  var owm_appid = "8861e7329a75864f502870d5c8714a76"; // This is registered to Kamots Time. Please generate and use your own.
  console.log("Fetching weather for: " + latitude + " " + longitude + " format: " + wxreq);
  var req = new XMLHttpRequest();
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" + latitude + "&lon=" + longitude + "&cnt=1";
  if(wxreq == 1) url = url + "&units=metric&APPID=" + owm_appid;
  else url = url + "&units=imperial&APPID=" + owm_appid;
  req.onerror = function(e) {
    console.error("Request Error " + req.status + " " + e.target.response.reason);
    Pebble.sendAppMessage({
      "WX_C":202,
      "WX_T":2000,
      "WX_A":0,
      "TOFF":0
    });
  }
  req.ontimeout = function(e) {
    console.error("Request Timeout " + req.status);
    Pebble.sendAppMessage({
      "WX_C":202,
      "WX_T":2000,
      "WX_A":0,
      "TOFF":0
    });
  }
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        //console.log(req.responseText); // Only for debugging
        var response = JSON.parse(req.responseText);
        if(response.cod == "200") {
          var temperature = Math.round(response.main.temp * 10);
          var icon = response.weather[0].icon;
          var wxid = response.weather[0].id;
          var clouds = response.clouds.all;
          // var city = response.name;
          var timestamp = response.dt;
          /* console.log(temperature);
          console.log(icon);
          console.log(city);
          console.log(timestamp); */
          var graphic = convertIconToGraphic(icon, wxid, clouds);
          // console.log(graphic);
          var runtime = Date.now() / 1000;
          var tzoffset = (new Date().getTimezoneOffset()) * 60;
          Pebble.sendAppMessage({ // Send current conditions and temp with TZ offset
            "WX_C":graphic,
            "WX_T":temperature,
            "WX_A":timestamp,
            "TOFF":tzoffset},
            function(e) {
              console.log("Send weather successful @", runtime);
            }, function(e) {
              console.error("Send weather failed! @", runtime);
            }
          );
        } else {
        console.error("API Error " + response.cod);
        Pebble.sendAppMessage({
          "WX_C":203,
          "WX_T":2000,
          "WX_A":0,
          "TOFF":0
        });
        }
      } else {
        console.error("HTTP Error " + req.status);
        Pebble.sendAppMessage({
          "WX_C":202,
          "WX_T":2000,
          "WX_A":0,
          "TOFF":0
        });
      }
    } else {
      console.error("Request Error " + req.status + " readyState=" + req.readyState);
      Pebble.sendAppMessage({
        "WX_C":202,
        "WX_T":2000,
        "WX_A":0,
        "TOFF":0
      });
    }
  };
  req.timeout = 20000;
  req.open('GET', url, true);
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  cachedcoords = pos.coords;
  // console.log(coordinates.latitude + " " + coordinates.longitude);
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn("location error: " + err);
  if(cachedcoords != null) {
    console.log("Using cached location: " + cachedcoords.latitude + " " + cachedcoords.longitude);
    fetchWeather(cachedcoords.latitude, cachedcoords.longitude);
  } else {
    Pebble.sendAppMessage({
      "WX_C":201,
      "WX_T":2000,
      "WX_A":0,
      "TOFF":0
    });
  }
}

var startWeatherFetch = function() {
  var locationOptions = { "timeout": 30000, "maximumAge": 300000, "enableHighAccuracy": false };
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
};

Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log("Got message from watch:" + JSON.stringify(e.payload));
                          wxreq = e.payload.wxreq;
                          startWeatherFetch();
});

Pebble.addEventListener("ready", function(e) {
  console.log("phonecomm.js ready!");
});
