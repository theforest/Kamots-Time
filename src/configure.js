
Pebble.addEventListener("showConfiguration",
  function(e) {
    Pebble.openURL("http://theforest.us/kamotstimecfg.php");
    console.log('Opened configuration window.');
  }
);

Pebble.addEventListener('webviewclosed', function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ', JSON.stringify(configuration));
    
    // Send to Pebble
//    Pebble.sendAppMessage({ "KEY_COLOR_HH": configuration.color_hh });
//    Pebble.sendAppMessage({ "KEY_COLOR_MH": configuration.color_mh });
//    Pebble.sendAppMessage({ "KEY_COLOR_WO": configuration.color_wo });
//    Pebble.sendAppMessage({ "KEY_COLOR_WB": configuration.color_wb });
//    Pebble.sendAppMessage({ "KEY_COLOR_SB": configuration.color_sb });
    Pebble.sendAppMessage({ "KEY_DIGITIME": configuration.digitime });
});
