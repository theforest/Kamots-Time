/* Kamots Time Pebble JS
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
      localStorage.setItem(KEY_CONFVER, "4"); // placeholder, still need to write version upgrade code
      localStorage.setItem(KEY_CONFDAT, JSON.stringify(configuration)); // save configuration
    }
});

function sendconfcolors(conf) {
    var runtime = Date.now() / 100;
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
    var runtime = Date.now() / 100;
    var digitime = 0;
    var bt_stats = 0;
    var sechand = 0;
    var dtzulu = 0;
    var digitime_string = conf.digitime;
    var bt_stats_string = conf.bt_stats;
    var sechand_string = conf.sechand;
    var dtzulu_string = conf.dtzulu;
    if(digitime_string.match(/^on/)) digitime = 1;
    if(bt_stats_string.match(/^on/)) bt_stats = 1;
    if(sechand_string.match(/^on/)) sechand = 1;
    if(dtzulu_string.match(/^on/)) dtzulu = 1;

    Pebble.sendAppMessage({ "HM_C": parseInt(conf.hm_count,10),
                            "D_DT": digitime,
                            "D_BT": bt_stats,
                            "D_SH": sechand,
                            "DT_Z": dtzulu },
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

