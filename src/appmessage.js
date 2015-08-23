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

var sendtopebble = function() {
  var stuff = {"D_WX": false}; // TODO weather stuff
  console.log("Sending stuff to pebble...");
  Pebble.sendAppMessage(stuff);
};

Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log("Got message from watch..."); // TODO trigger weather update if requested
                        });

Pebble.addEventListener("ready", function(e) {
  console.log("appmessage.js ready!");

  setInterval(sendtopebble, 1800000); // 30 minutes
  sendtopebble();
});