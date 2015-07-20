
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
    Pebble.sendAppMessage({ "KEY_COLOR_HH": parseInt(configuration.color_hh.substring(1,7),16),
                            "KEY_COLOR_MH": parseInt(configuration.color_mh.substring(1,7),16),
                            "KEY_COLOR_HM": parseInt(configuration.color_hm.substring(1,7),16),
                            "KEY_COLOR_WB": parseInt(configuration.color_wb.substring(1,7),16),
                            "KEY_COLOR_WO": parseInt(configuration.color_wo.substring(1,7),16),
                            "KEY_COLOR_SB": parseInt(configuration.color_sb.substring(1,7),16),
                            "KEY_DIGITIME": configuration.digitime,
                            "KEY_HM_COUNT": parseInt(configuration.hm_count,10),
                            "KEY_BT_STATS": configuration.bt_stats });
});
