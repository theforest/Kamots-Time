
Pebble.addEventListener("showConfiguration",
  function(e) {
    Pebble.openURL("http://theforest.us/kamotstimecfg.php");
    console.log('Opened configuration window.');
  }
);

var colretries = 4;
var optretries = 4;

Pebble.addEventListener('webviewclosed', function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ', JSON.stringify(configuration));
  
    // Send to Pebble (with retries)
    colretries = 0;
    optretries = 0;
    sendconfcolors(configuration);
    sendconfoptions(configuration);
});

function sendconfcolors(conf) {
    var runtime = Date.now() / 1000;
    Pebble.sendAppMessage({ "C_HH": parseInt(conf.color_hh.substring(1,7),16),
                            "C_MH": parseInt(conf.color_mh.substring(1,7),16),
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
    var digitime_string = conf.digitime;
    var bt_stats_string = conf.bt_stats;
    if(digitime_string.match(/^on/)) digitime = 1;
    if(bt_stats_string.match(/^on/)) bt_stats = 1;

    Pebble.sendAppMessage({ "HM_C": parseInt(conf.hm_count,10),
                            "D_DT": digitime,
                            "D_BT": bt_stats },
                            function(e) {
                              console.log("Send options successful @", runtime);
                            }, function(e) {
                              console.log("Send options failed! @", runtime);
                              if (optretries++ < 4) sendconfoptions(conf);
                            }
                           );
}
