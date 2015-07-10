
Pebble.addEventListener("showConfiguration",
  function(e) {
    Pebble.openURL("http://theforest.us/kamotstimecfg.php");
    console.log('Opened configuration window.');
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    var config = e.response;
    console.log('Configuration window returned: ' + e.response);
    Pebble.sendAppMessage( { "ChangeConfig": config } );
  }
);