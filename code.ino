// === LED Cube GPIO Configuration ===
// Column GPIOs (Anode side)
const int columnPins[16] = {
  32, 33, 25, 26, 27, 14, 12, 13,
  23, 22, 21, 19, 18, 5, 17, 15
};

// Layer GPIOs (Cathode side)
const int layerPins[4] = {4, 0, 2, 15};

// Frame buffer
bool frameBuffer[16][4] = {false};

// === FreeRTOS Task Handles ===
TaskHandle_t displayTaskHandle;
TaskHandle_t patternTaskHandle;

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 16; i++) {
    pinMode(columnPins[i], OUTPUT);
    digitalWrite(columnPins[i], HIGH); // Anode OFF
  }

  for (int i = 0; i < 4; i++) {
    pinMode(layerPins[i], OUTPUT);
    digitalWrite(layerPins[i], HIGH); // Cathode OFF
  }

  // Start FreeRTOS Tasks
  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 2048, NULL, 2, &displayTaskHandle, 0);
  xTaskCreatePinnedToCore(patternTask, "PatternTask", 4096, NULL, 1, &patternTaskHandle, 1);
}

// === Frame Display Task ===
void displayTask(void *pvParameters) {
  while (true) {
    for (int l = 0; l < 4; l++) {
      // Turn off all layers
      for (int i = 0; i < 4; i++) {
        digitalWrite(layerPins[i], HIGH);
      }

      // Set columns for this layer
      for (int c = 0; c < 16; c++) {
        digitalWrite(columnPins[c], frameBuffer[c][l] ? LOW : HIGH);
      }

      // Turn on current layer
      digitalWrite(layerPins[l], LOW);
      delay(2);
    }
  }
}

// === Utility Functions ===
void clearFrame() {
  for (int c = 0; c < 16; c++) {
    for (int l = 0; l < 4; l++) {
      frameBuffer[c][l] = false;
    }
  }
}

// === Effects ===
void raindropEffect() {
  clearFrame();
  for (int drop = 0; drop < 10; drop++) {
    int column = random(16);
    for (int layer = 0; layer < 4; layer++) {
      frameBuffer[column][layer] = true;
      delay(60);
      frameBuffer[column][layer] = false;
    }
  }
}

void spiralEffect() {
  clearFrame();
  int spiral[16][2] = {
    {0,0},{1,0},{2,0},{3,0},
    {3,1},{3,2},{3,3},
    {2,3},{1,3},{0,3},
    {0,2},{0,1},
    {1,1},{2,1},{2,2},{1,2}
  };

  for (int step = 0; step < 16; step++) {
    clearFrame();
    for (int i = 0; i <= step; i++) {
      int col = spiral[i][0] * 4 + spiral[i][1];
      frameBuffer[col][0] = true;
    }
    delay(100);
  }
}

void layerWave() {
  for (int cycles = 0; cycles < 3; cycles++) {
    for (int l = 0; l < 4; l++) {
      clearFrame();
      for (int c = 0; c < 16; c++) frameBuffer[c][l] = true;
      delay(150);
    }
    for (int l = 3; l >= 0; l--) {
      clearFrame();
      for (int c = 0; c < 16; c++) frameBuffer[c][l] = true;
      delay(150);
    }
  }
}

void randomSparkle() {
  for (int i = 0; i < 50; i++) {
    clearFrame();
    for (int j = 0; j < 5; j++) {
      int col = random(16);
      int layer = random(4);
      frameBuffer[col][layer] = true;
    }
    delay(60);
  }
}

void cubeOutline() {
  clearFrame();
  for (int x = 0; x < 4; x++) {
    frameBuffer[x*4 + 0][0] = true;
    frameBuffer[x*4 + 3][0] = true;
    frameBuffer[0*4 + x][0] = true;
    frameBuffer[3*4 + x][0] = true;

    frameBuffer[x*4 + 0][3] = true;
    frameBuffer[x*4 + 3][3] = true;
    frameBuffer[0*4 + x][3] = true;
    frameBuffer[3*4 + x][3] = true;
  }

  for (int l = 0; l < 4; l++) {
    frameBuffer[0*4 + 0][l] = true;
    frameBuffer[0*4 + 3][l] = true;
    frameBuffer[3*4 + 0][l] = true;
    frameBuffer[3*4 + 3][l] = true;
  }

  delay(1200);
}

void diagonalWipe() {
  clearFrame();
  for (int diag = 0; diag < 7; diag++) {
    clearFrame();
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        if (x + y == diag) {
          for (int z = 0; z < 4; z++) {
            frameBuffer[x * 4 + y][z] = true;
          }
        }
      }
    }
    delay(100);
  }
}

// === Animation Manager (RTOS Task) ===
void loopEffect(void (*effect)(), int repeats) {
  for (int i = 0; i < repeats; i++) {
    effect();
  }
}

const int numEffects = 6;
int currentEffect = 0;

void patternTask(void *pvParameters) {
  while (true) {
    switch (currentEffect) {
      case 0: loopEffect(raindropEffect, 6); break;
      case 1: loopEffect(spiralEffect, 3); break;
      case 2: loopEffect(layerWave, 3); break;
      case 3: loopEffect(randomSparkle, 3); break;
      case 4: loopEffect(cubeOutline, 3); break;
      case 5: loopEffect(diagonalWipe, 3); break;
    }
    currentEffect = (currentEffect + 1) % numEffects;
  }
}

// Add this mandatory Arduino loop function, even if empty
void loop() {
  // Intentionally empty because FreeRTOS tasks are running the logic
}
