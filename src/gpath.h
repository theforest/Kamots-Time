#pragma once
/*
*** Kamots Time ***

Copyright 2015 kamotswind

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing permissions and
limitations under the License.
*/

GPath *path_bolt_ptr = NULL;
GPath *path_bt_ptr = NULL;
GPath *path_plug_ptr = NULL;
GPath *path_lcloud_ptr = NULL;
GPath *path_scloud_ptr = NULL;
GPath *path_lightning_ptr = NULL;
GPath *path_snowflake_ptr = NULL;

const GPathInfo BOLT_PATH_INFO = { // Lightning bolt
  .num_points = 6,
  .points = (GPoint []) {
    {(float)4.5, 0},
    {(float)4.5, (float)13.95},
    {8, (float)13.95},
    {(float)2.25, 26},
    {(float)4.5, 16},
    {0, 16}
  }
};

const GPathInfo BT_PATH_INFO = { // Bluetooth symbol
  .num_points = 8,
  .points = (GPoint []) {
    {0, 0},
    {3, 3},
    {-3, 9},
    {0, 6},
    {-3, 3},
    {3, 9},
    {0, 12},
    {0, 6}
  }
};

const GPathInfo PLUG_PATH_INFO = { // Power plug symbol
  .num_points = 20,
  .points = (GPoint []) {
    {0, 3},
    {2, 3},
    {2, 0},
    {3, 0},
    {3, 3},
    {6, 3},
    {6, 0},
    {7, 0},
    {7, 3},
    {9, 3},
    {9, 7},
    {8, 7},
    {8, 8},
    {5, 8},
    {5, 12},
    {4, 12},
    {4, 8},
    {1, 8},
    {1, 7},
    {0, 7}
  }
};

// Cloud designs below inspired by watchface "Straight" created by "dPunisher"

const GPathInfo WX_LCLOUD_PATH_INFO = { // Large Cloud
  .num_points = 8,
  .points = (GPoint []) {
    {5, 0},
    {14, 0},
    {14, 4},
    {20, 4},
    {20, 11},
    {0, 11},
    {0, 6},
    {5, 6}
  }
};

const GPathInfo WX_SCLOUD_PATH_INFO = { // Small Cloud
  .num_points = 8,
  .points = (GPoint []) {
    {3, 0},
    {7, 0},
    {7, 2},
    {9, 2},
    {9, 6},
    {0, 6},
    {0, 3},
    {3, 3}
  }
};

const GPathInfo WX_LIGHTNING_PATH_INFO = { // Lightning bolt
  .num_points = 24,
  .points = (GPoint []) {
    {6, 0},
    {9, 0},
    {9, 2},
    {8, 2},
    {8, 4},
    {7, 4},
    {7, 5},
    {13, 7},
    {13, 8},
    {11, 8},
    {11, 11},
    {9, 12},
    {9, 13},
    {7, 13},
    {6, 15},
    {5, 15},
    {5, 14},
    {6, 12},
    {7, 9},
    {2, 9},
    {2, 7},
    {3, 6},
    {4, 4},
    {5, 4}
  }
};

const GPathInfo WX_SNOWFLAKE_PATH_INFO = { // Snowflake
  .num_points = 20,
  .points = (GPoint []) {
    {3, 0},
    {4, 0},
    {4, 2},
    {5, 2},
    {5, 3},
    {7, 3},
    {7, 4},
    {5, 4},
    {5, 5},
    {4, 5},
    {4, 7},
    {3, 7},
    {3, 5},
    {2, 5},
    {2, 4},
    {0, 4},
    {0, 3},
    {2, 3},
    {2, 2},
    {3, 2}
  }
};
