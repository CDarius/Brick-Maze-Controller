#include <MatrixDisplay.hpp>

int32_t MatrixDisplay::getPixelIndex(int16_t x, int16_t y) const {
    // 1. Boundary Check
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return -1;
    }

    return (MATRIX_WIDTH - 1 - x) * MATRIX_HEIGHT + y;
}

void MatrixDisplay::show() {
    // Copy every pixel from Canvas to Strip, applying Dimming on the fly
    for(uint16_t i = 0; i < TOTAL_LEDS; i++) {
        // RgbColor::Dim(x) scales the color by x/255
        strip.SetPixelColor(i, _canvas[i].Dim(_brightness));
    }
    strip.Show();
}

void MatrixDisplay::drawPixel(int16_t x, int16_t y, RgbColor color) {
    int32_t index = getPixelIndex(x, y);
    if (index != -1) {
        // Store the ORIGINAL color in the canvas
        _canvas[index] = color;
    }
}

// 2. Draw a filled rectangle
void MatrixDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, RgbColor color) {
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) {
        return; // Rectangle is completely off-screen
    }
    if (x + w <= 0 || y + h <= 0) {
        return; // Rectangle is completely off-screen
    }
    // Clip the rectangle to the display boundaries
    int16_t startX = x > 0 ? x : 0;
    int16_t endX = (x + w) < MATRIX_WIDTH ? (x + w) : MATRIX_WIDTH;
    int16_t startY = y > 0 ? y : 0;
    int16_t endY = (y + h) < MATRIX_HEIGHT ? (y + h) : MATRIX_HEIGHT;
    for (int16_t i = startX; i < endX; i++) {
        uint16_t pixelIndex = getPixelIndex(i, startY);
        for (int16_t j = startY; j < endY; j++) {
            _canvas[pixelIndex] = color;
            pixelIndex++; // Move to the next pixel in the same column
        }
    }
}

// 3. Draw a rectangle outline
void MatrixDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, RgbColor color) {
    drawLine(x, y, x + w - 1, y, color);         // Top
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, color); // Bottom
    drawLine(x, y, x, y + h - 1, color);         // Left
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color); // Right
}

// 4. Draw a Line (Bresenham's Algorithm) with optimization for horizontal and vertical lines
void MatrixDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, RgbColor color) {
    // Check for horizontal line
    if (y0 == y1) {
        // Optimization for horizontal line
        if (y0 < 0 || y0 >= MATRIX_HEIGHT) {
            return; // Line is completely off-screen vertically
        }
        
        int16_t start = x0 < x1 ? x0 : x1;
        int16_t end = x0 > x1 ? x0 : x1;
        if (start >= MATRIX_WIDTH) {
            return; // Line is completely off-screen to the right
        }
        if (end < 0) {
            return; // Line is completely off-screen to the left
        }
        // Clip the line to the display boundaries
        start = start > 0 ? start : 0;
        end = end < MATRIX_WIDTH ? end : MATRIX_WIDTH - 1;
        
        int16_t pixeIndex = getPixelIndex(start, y0);
        for (int16_t x = start; x <= end; x++) {
            _canvas[pixeIndex] = color;
            pixeIndex -= MATRIX_HEIGHT; // Move to the next pixel in the same row
        }
        return;
    }

    // Check for vertical line
    if (x0 == x1) {
        // Optimization for vertical line
        if (x0 < 0 || x0 >= MATRIX_WIDTH) {
            return; // Line is completely off-screen horizontally
        }

        int16_t start = y0 < y1 ? y0 : y1;
        int16_t end = y0 > y1 ? y0 : y1;
        if (start >= MATRIX_HEIGHT) {
            return; // Line is completely off-screen below
        }
        if (end < 0) {
            return; // Line is completely off-screen above
        }
        // Clip the line to the display boundaries
        start = start > 0 ? start : 0;
        end = end < MATRIX_HEIGHT ? end : MATRIX_HEIGHT - 1;

        int16_t pixelIndex = getPixelIndex(x0, start);
        for (int16_t y = start; y <= end; y++) {
            _canvas[pixelIndex] = color;
            pixelIndex++; // Move to the next pixel in the same column (decreasing index because of hardware layout)
        }
        return;
    }

    // Bresenham's algorithm for general lines
    int16_t dx = abs(x1 - x0);
    int16_t dy = -abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    while (true) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void MatrixDisplay::copyCanvasTo(RgbColor* targetCanvas) const {
    for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
        targetCanvas[i] = _canvas[i];
    }
}

void MatrixDisplay::copyCanvasFrom(const RgbColor* sourceCanvas) {
    for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
        _canvas[i] = sourceCanvas[i];
    }
}

void MatrixDisplay::copyCanvasFrom(const RgbColor* sourceCanvas, int16_t sourceX, int16_t sourceY, int16_t width, int16_t height, int16_t destX, int16_t destY) {
    if (destX >= MATRIX_WIDTH || destY >= MATRIX_HEIGHT) {
        return; // Destination is completely out of bounds
    }

    if (destX < 0) {
        // If the destination X is negative, we need to skip some columns from the source
        int16_t skipColumns = -destX;
        sourceX += skipColumns;
        width -= skipColumns;
        destX = 0; // Start at the left edge of the display
    }
    if (destY < 0) {
        // If the destination Y is negative, we need to skip some rows from the source
        int16_t skipRows = -destY;
        sourceY += skipRows;
        height -= skipRows;
        destY = 0; // Start at the top edge of the display
    }

    // Adjust sourceX and sourceY if they are negative, and reduce width and height to the maximum possible
    int16_t sourceX2 = sourceX > 0 ? sourceX : 0;
    int16_t sourceY2 = sourceY > 0 ? sourceY : 0;

    int16_t maxWidth = MATRIX_WIDTH - destX;
    int16_t maxHeight = MATRIX_HEIGHT - destY;

    width = width < maxWidth ? width : maxWidth;
    height = height < maxHeight ? height : maxHeight;

    if (width <= 0 || height <= 0) {
        return; // Nothing to copy
    }
    
    if ((destX + width) < 0 || (destY + height) < 0) {
        return; // Destination is completely off-screen
    }
    
    for (int16_t x = 0; x < width; x++) {
        int16_t xs = sourceX2 + x;
        int16_t xd = destX + x;
        int32_t sourcePixelIndex = getPixelIndex(xs, sourceY2);
        int32_t destPixelIndex = getPixelIndex(xd, destY);
        for (int16_t y = 0; y < height; y++) {
            _canvas[destPixelIndex] = sourceCanvas[sourcePixelIndex];
            sourcePixelIndex++;
            destPixelIndex++;
        }
    }    
}

void MatrixDisplay::linearColorGradient(RgbColor startColor, RgbColor endColor, RgbColor* colors, uint8_t colorsLength) const {
    // 1. Handle edge cases to prevent division by zero or errors
    if (colorsLength == 0) return;
    
    if (colorsLength == 1) {
        colors[0] = startColor;
        return;
    }

    // 2. Loop through the array and calculate the gradient
    for (uint8_t i = 0; i < colorsLength; i++) {
        // Calculate progress ratio between 0.0 and 1.0
        float progress = (float)i / (colorsLength - 1);

        // LinearBlend calculates the mixed color based on the progress
        colors[i] = RgbColor::LinearBlend(startColor, endColor, progress);
    }
}

void MatrixDisplay::mirroredColorGradient(RgbColor startColor, RgbColor endColor, RgbColor* colors, uint8_t colorsLength) const {
    // 1. Safety check
    if (colorsLength == 0) return;

    // 2. Determine how many steps we have to the "peak" (middle)
    // If length is 10 (even), we have 5 steps (0-4).
    // If length is 11 (odd), we have 6 steps (0-5), where 5 is the exact center.
    uint8_t steps = (colorsLength + 1) / 2;

    // 3. Loop only through the first half
    for (uint8_t i = 0; i < steps; i++) {
        
        // Calculate progress towards the center (0.0 to 1.0)
        // We use (steps - 1) because at the last step of the half, we want exactly 'endColor'
        float progress;
        if (steps <= 1) {
            progress = 1.0f; // Edge case for very small arrays
        } else {
            progress = (float)i / (float)(steps - 1);
        }

        // Calculate the color
        RgbColor color = RgbColor::LinearBlend(startColor, endColor, progress);

        // Assign to the front
        colors[i] = color;

        // Assign to the back (mirroring)
        // This math works for both odd and even lengths.
        // If odd, the middle element overwrites itself, which is harmless.
        colors[colorsLength - 1 - i] = color;
    }
}