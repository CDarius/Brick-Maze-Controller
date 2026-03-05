#include <HMI.hpp>

#define MAIN_DISPLAY_DIRECTION_C    0
#define MAIN_DISPLAY_DIRECTION_N    1
#define MAIN_DISPLAY_DIRECTION_S    2
#define MAIN_DISPLAY_DIRECTION_W    3
#define MAIN_DISPLAY_DIRECTION_E    4
#define MAIN_DISPLAY_DIRECTION_NE   5
#define MAIN_DISPLAY_DIRECTION_NW   6
#define MAIN_DISPLAY_DIRECTION_SE   7
#define MAIN_DISPLAY_DIRECTION_SW   8

constexpr unsigned long dizzyDuration = 2000; // Duration to stay dizzy in milliseconds
constexpr float dizzyThreshold = 600.0f; // Rotation speed threshold to trigger dizzy state (in G)

void HMI::updateIMUData(float ax, float ay, float az, float gx, float gy, float gz) {
    // This function can be used to pass IMU data to the HMI for display
    this->ax = ax;
    this->ay = ay;
    this->az = az;
    this->gx = gx;
    this->gy = gy;
    this->gz = gz;

    // Calculate the total rotation speed
    float totalRotationSpeed = abs(gx) + abs(gy) + abs(gz);

    // Check if the controller has been shaken hard
    if (totalRotationSpeed > dizzyThreshold) {
        isDizzy = true;
        dizzyEndTime = millis() + dizzyDuration;
    } else if (isDizzy && millis() > dizzyEndTime) {
        isDizzy = false; // End dizzy state after the duration has passed
    }
}

void HMI::setMode(Mode newMode) {
    if (newMode == currentMode) 
        return; // No change

    currentMode = newMode;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void HMI::updateLoop() {
    while (true) {
        switch (currentMode) {
            case Mode::NO_GAME:
                noGameUpdateLoop();
                break;
            case Mode::IN_GAME:
                inGameUpdateLoop();
                break;
            case Mode::WRITE_PLAYER_NAME:
                writePlayerNameUpdateLoop();
                break;
            default:
                // Invalid mode, just wait
                delay(100);
                break;
        }
    }
}

void HMI::rainbowStripesLoop(uint32_t maxTimeMs, CancelToken& cancelToken) {
    uint16_t width = display.getWidth();
    uint16_t height = display.getHeight();
    
    unsigned long endTime = millis() + maxTimeMs;
    float hueOffset = 0.0f; // Variable used to scroll the rainbow
    
    while (!cancelToken.isCancelled() && millis() < endTime) {
        for (uint8_t x = 0; x < 5; x++) {
            for (uint8_t y = 0; y < 5; y++) {
                // Compute hue based on diagonal position
                // Divide by a factor (e.g. 10.0) to control wave "width"
                float hue = hueOffset + (float)(x + y) / 10.0f;
                
                // Keep hue between 0.0 and 1.0 (looping effect)
                while (hue > 1.0f) hue -= 1.0f;

                // Create the HSB color and convert it to RGB
                // Saturation: 1.0 (full), Brightness: 0.1 (to avoid glare/heat)
                HsbColor hsb(hue, 1.0f, 0.1f);
                RgbColor rgb(hsb);

                display.drawPixel(x, y, rgb);
            }
        }  

        display.show();

        hueOffset += 0.01f; // Scroll speed
        if (hueOffset > 1.0f) hueOffset = 0.0f;

        delay(30); // Animation smoothness
    }
}

void HMI::snakeLoop(uint32_t maxTimeMs, CancelToken& cancelToken) {
    uint16_t width = display.getWidth();
    uint16_t height = display.getHeight();
    
    struct Point {
        int8_t x, y;
    };

    const uint8_t snakeLen = 4;
    Point snake[snakeLen] = {{2,2}, {2,1}, {2,0}, {2,0}}; // Initial state
    int8_t dx = 1, dy = 0; // Initial direction (to the right)
    float snakeHue = 0.0f;

    unsigned long endTime = millis() + maxTimeMs;

    while (!cancelToken.isCancelled() && millis() < endTime) {
        // 1. Clear strip (black)
        display.clear();

        // 2. Compute the next head position
        int8_t nextX = snake[0].x + dx;
        int8_t nextY = snake[0].y + dy;

        // 3. Check edge collisions or random direction change
        // If it hits an edge, or with 10% probability, change direction
        bool shouldTurn = (nextX < 0 || nextX >= width || nextY < 0 || nextY >= height || random(10) == 0);
        if (shouldTurn) {
            const int8_t candidateDx[4] = {0, 0, -1, 1};
            const int8_t candidateDy[4] = {-1, 1, 0, 0};
            uint8_t validChoices[4];
            uint8_t validCount = 0;

            for (uint8_t i = 0; i < 4; i++) {
                int8_t candDx = candidateDx[i];
                int8_t candDy = candidateDy[i];

                // Exclude opposite direction
                if (candDx == -dx && candDy == -dy) {
                    continue;
                }

                int8_t candNextX = snake[0].x + candDx;
                int8_t candNextY = snake[0].y + candDy;

                // Exclude moves outside the display
                if (candNextX < 0 || candNextX >= width || candNextY < 0 || candNextY >= height) {
                    continue;
                }

                // Exclude move onto the "neck" (avoids perceived reversal)
                if (candNextX == snake[1].x && candNextY == snake[1].y) {
                    continue;
                }

                validChoices[validCount++] = i;
            }

            if (validCount > 0) {
                uint8_t selected = validChoices[random(validCount)];
                dx = candidateDx[selected];
                dy = candidateDy[selected];
            }

            nextX = snake[0].x + dx;
            nextY = snake[0].y + dy;
        }

        // 4. Move the body (each segment follows the previous one)
        for (int i = snakeLen - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }
        snake[0].x = nextX;
        snake[0].y = nextY;

        // 5. Draw the snake with a color gradient
        for (int i = 0; i < snakeLen; i++) {
            // The head is brighter than the tail
            float brightness = 0.15f - ((float)i * 0.03f); 
            HsbColor color(snakeHue, 1.0f, max(0.02f, brightness));
            display.drawPixel(snake[i].x, snake[i].y, RgbColor(color));
        }

        display.show();

        // 6. Update color and wait
        snakeHue += 0.01f;
        if (snakeHue > 1.0f) snakeHue = 0.0f;
        
        delay(150); // Snake speed
    }
}

void HMI::eyesLoop(uint32_t maxTimeMs, CancelToken& cancelToken) {
    unsigned long endTime = millis() + maxTimeMs;

    while (!cancelToken.isCancelled() && millis() < endTime) {
        // 1. Map acceleration (-1.0 to 1.0) to an offset (-1, 0, 1)
        // Use a threshold (0.2) to reduce eye jitter
        int8_t offsetX = 0;
        int8_t offsetY = 0;

        if (ax > 0.2)  offsetY = 1; 
        else if (ax < -0.2) offsetY = -1;

        if (ay > 0.2)  offsetX = 1;
        else if (ay < -0.2) offsetX = -1;

        // 2. Define the base eye positions (X, Y)
        int8_t eye1_X = 1 + offsetX;
        int8_t eye1_Y = 1 + offsetY;
        int8_t eye2_X = 3 + offsetX;
        int8_t eye2_Y = 1 + offsetY;

        // 3. Draw
        display.clear();

        // Eye color (cyan-like blue)
        RgbColor eyeColor(0, 255, 255); 

        // Draw the two eye pixels
        display.drawPixel(eye1_X, eye1_Y, eyeColor);
        display.drawPixel(eye1_X, eye1_Y + 1, eyeColor);
        display.drawPixel(eye2_X, eye2_Y, eyeColor);
        display.drawPixel(eye2_X, eye2_Y + 1, eyeColor);
        
        // 4. Bonus: occasionally blink
        if (millis() % 5000 < 200) { 
            display.clear(); // Close eyes for 200ms every 5 seconds
        }

        display.show();
        delay(50);
    }
}

void HMI::noGameUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    while (!cancelToken->isCancelled()) {
        // Start with eye animation for 60 seconds
        eyesLoop(60 * 1000, localCancelToken);
        if (localCancelToken.isCancelled()) 
            break; // Check if we were cancelled during the eyes loop

        // Switch to snake animation for 10 minutes
        snakeLoop(10 * 60 * 1000, localCancelToken);
        if (localCancelToken.isCancelled()) 
            break; // Check if we were cancelled during the snake loop
    }

    cancelToken = nullptr; // Clear the cancel token when exiting the loop
}

void HMI::inGameUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    uint8_t direction = 255; // Start with an invalid direction to force an update on the first loop

    float spinAngle = 0.0f; // Angle for dizzy eye animation

    while (!cancelToken->isCancelled()) {
        if (!isDizzy) {
            // Only update pointing direction if not dizzy
            displayPointingDirection(direction);
        } else {
            displayDizzyEyes();

            direction = 255; // Reset direction to force update when coming out of dizzy state
        }

        delay(50); // Update rate
    }

    cancelToken = nullptr; // Clear the cancel token when exiting the loop
}

void HMI::writePlayerNameUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    constexpr float tiltThreshold = 0.2f;
    
    while (!cancelToken->isCancelled()) {
        uint8_t newDirection;
        // Direction determination logic (kept as in the original)
        if (ax > tiltThreshold) {
            newDirection = MAIN_DISPLAY_DIRECTION_N;
        } else if (ax < -tiltThreshold) {
            newDirection = MAIN_DISPLAY_DIRECTION_S;
        } else {
            newDirection = MAIN_DISPLAY_DIRECTION_C;
        }

        // Fade current pixels slightly to create a ghosting trail effect
        RgbColor buffer[TOTAL_LEDS];
        display.copyCanvasTo(buffer);
        for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
            buffer[i] = buffer[i].Dim(127); // Dim existing pixels by 50%
        }
        display.copyCanvasFrom(buffer);

        // 1980s arcade palette
        RgbColor neonCyan(0, 200, 255);   // Main movement color
        RgbColor neonMagenta(255, 0, 150); // Color for center/crosshair
        RgbColor dimCyan = neonCyan.Dim(64); // Trail/glow effect

        switch (newDirection) {
            case MAIN_DISPLAY_DIRECTION_N:
                display.drawPixel(2, 0, neonCyan);
                display.drawPixel(1, 1, neonCyan);
                display.drawPixel(3, 1, neonCyan);
                display.drawPixel(2, 1, dimCyan);
                display.drawPixel(0, 2, dimCyan);
                display.drawPixel(1, 2, dimCyan);
                display.drawPixel(2, 2, dimCyan);
                display.drawPixel(3, 2, dimCyan);
                display.drawPixel(4, 2, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_S:
                display.drawPixel(2, 4, neonCyan);
                display.drawPixel(1, 3, neonCyan);
                display.drawPixel(3, 3, neonCyan);
                display.drawPixel(2, 3, dimCyan);
                display.drawPixel(0, 2, dimCyan);
                display.drawPixel(1, 2, dimCyan);
                display.drawPixel(2, 2, dimCyan);
                display.drawPixel(3, 2, dimCyan);
                display.drawPixel(4, 2, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_C:
                // Blinking center bar
                if (millis() % 800 < 400) {
                    display.drawPixel(0, 2, neonMagenta.Dim(100));
                    display.drawPixel(1, 2, neonMagenta);
                    display.drawPixel(2, 2, neonMagenta); 
                    display.drawPixel(3, 2, neonMagenta); 
                    display.drawPixel(4, 2, neonMagenta.Dim(100)); // East
                }
                break;
        }

        display.show();
        delay(50);
    }

    cancelToken = nullptr; // Clear the cancel token when exiting the loop
}

void HMI::displayDizzyEyes() {
    display.clear();

    // Draw dizzy eyes (simple rotating pattern)
    if (millis() % 400 < 200) {
        // First frame of dizzy animation
        // Left eye
        display.drawPixel(1, 0, COLOR_RED);
        display.drawPixel(0, 1, COLOR_RED);
        // Right eye
        display.drawPixel(3, 0, COLOR_RED);
        display.drawPixel(4, 1, COLOR_RED);
    } else {
        // Second frame of dizzy animation
        // Left eye
        display.drawPixel(0, 0, COLOR_RED);
        display.drawPixel(1, 1, COLOR_RED);
        // Right eye
        display.drawPixel(4, 0, COLOR_RED);
        display.drawPixel(3, 1, COLOR_RED);
    }

    // Sad mouth
    display.drawPixel(0, 4, COLOR_RED);
    display.drawPixel(1, 3, COLOR_RED);
    display.drawPixel(2, 3, COLOR_RED);
    display.drawPixel(3, 3, COLOR_RED);
    display.drawPixel(4, 4, COLOR_RED);

    display.show();
}

void HMI::displayPointingDirection(uint8_t& direction) {
    constexpr float tiltThreshold = 0.2f;

    uint8_t newDirection;
    // Direction determination logic (kept as in the original)
    if (ax > tiltThreshold) {
        if (ay > tiltThreshold)      newDirection = MAIN_DISPLAY_DIRECTION_NW;
        else if (ay < -tiltThreshold) newDirection = MAIN_DISPLAY_DIRECTION_NE;
        else                          newDirection = MAIN_DISPLAY_DIRECTION_N;
    } else if (ax < -tiltThreshold) {
        if (ay > tiltThreshold)      newDirection = MAIN_DISPLAY_DIRECTION_SW;
        else if (ay < -tiltThreshold) newDirection = MAIN_DISPLAY_DIRECTION_SE;
        else                          newDirection = MAIN_DISPLAY_DIRECTION_S;
    } else if (ay > tiltThreshold) {
        newDirection = MAIN_DISPLAY_DIRECTION_W;
    } else if (ay < -tiltThreshold) {
        newDirection = MAIN_DISPLAY_DIRECTION_E;
    } else {
        newDirection = MAIN_DISPLAY_DIRECTION_C;
    }

    // Update only when the direction changes to avoid extra bus load
    //if (newDirection != direction) {
    if (true) {
        direction = newDirection;
        
        // Fade current pixels slightly to create a ghosting trail effect
        RgbColor buffer[TOTAL_LEDS];
        display.copyCanvasTo(buffer);
        for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
            buffer[i] = buffer[i].Dim(127); // Dim existing pixels by 50%
        }
        display.copyCanvasFrom(buffer);

        // 1980s arcade palette
        RgbColor neonCyan(0, 200, 255);   // Main movement color
        RgbColor neonMagenta(255, 0, 150); // Color for center/crosshair
        RgbColor dimCyan = neonCyan.Dim(64); // Trail/glow effect

        switch (direction) {
            case MAIN_DISPLAY_DIRECTION_N:
                display.drawPixel(2, 0, neonCyan); // Tip
                display.drawPixel(1, 1, neonCyan); // Left wing
                display.drawPixel(3, 1, neonCyan); // Right wing
                display.drawPixel(2, 1, dimCyan);  // Inner trail
                break;

            case MAIN_DISPLAY_DIRECTION_S:
                display.drawPixel(2, 4, neonCyan);
                display.drawPixel(1, 3, neonCyan);
                display.drawPixel(3, 3, neonCyan);
                display.drawPixel(2, 3, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_E:
                display.drawPixel(0, 2, neonCyan);
                display.drawPixel(1, 1, neonCyan);
                display.drawPixel(1, 3, neonCyan);
                display.drawPixel(1, 2, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_W:
                display.drawPixel(4, 2, neonCyan);
                display.drawPixel(3, 1, neonCyan);
                display.drawPixel(3, 3, neonCyan);
                display.drawPixel(3, 2, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_NE:
                display.drawPixel(0, 0, neonCyan); // Corner tip
                display.drawPixel(1, 0, neonCyan); // Top side
                display.drawPixel(2, 0, dimCyan);  // Top-side glow
                display.drawPixel(0, 1, neonCyan); // Left side
                display.drawPixel(0, 2, dimCyan);  // Left-side glow
                break;

            case MAIN_DISPLAY_DIRECTION_NW:
                display.drawPixel(4, 0, neonCyan);
                display.drawPixel(3, 0, neonCyan);
                display.drawPixel(2, 0, dimCyan);
                display.drawPixel(4, 1, neonCyan);
                display.drawPixel(4, 2, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_SE:
                display.drawPixel(0, 4, neonCyan);
                display.drawPixel(1, 4, neonCyan);
                display.drawPixel(2, 4, dimCyan);
                display.drawPixel(0, 3, neonCyan);
                display.drawPixel(0, 2, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_SW:
                display.drawPixel(4, 4, neonCyan);
                display.drawPixel(3, 4, neonCyan);
                display.drawPixel(2, 4, dimCyan);
                display.drawPixel(4, 3, neonCyan);
                display.drawPixel(4, 2, dimCyan);
                break;

            case MAIN_DISPLAY_DIRECTION_C:
                // "Star Wars Arcade" (1983)-style crosshair
                display.drawPixel(2, 2, neonMagenta); // Center
                display.drawPixel(2, 1, neonMagenta.Dim(100)); // North
                display.drawPixel(2, 3, neonMagenta.Dim(100)); // South
                display.drawPixel(1, 2, neonMagenta.Dim(100)); // West
                display.drawPixel(3, 2, neonMagenta.Dim(100)); // East
                break;
        }
        display.show();
    }
}