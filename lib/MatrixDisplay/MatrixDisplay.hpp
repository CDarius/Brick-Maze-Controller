#pragma once

#include <NeoPixelBus.h>

// Define the specifications of the display
constexpr uint16_t MATRIX_WIDTH = 5;
constexpr uint16_t MATRIX_HEIGHT = 5;
constexpr uint16_t TOTAL_LEDS = MATRIX_WIDTH * MATRIX_HEIGHT; // 25

// Colors definitions
const RgbColor COLOR_BLACK(0, 0, 0);   // Black
const RgbColor COLOR_WHITE(255, 255, 255);   // White
const RgbColor COLOR_RED(255, 0, 0);   // Red
const RgbColor COLOR_ORANGE(255, 127, 0); // Orange
const RgbColor COLOR_YELLOW(255, 255, 0); // Yellow
const RgbColor COLOR_CHARTREUSE(127, 255, 0); // Chartreuse
const RgbColor COLOR_GREEN(0, 255, 0);   // Green
const RgbColor COLOR_SPRING_GREEN(0, 255, 127); // Spring Green
const RgbColor COLOR_CYAN(0, 255, 255); // Cyan
const RgbColor COLOR_AZURE(0, 127, 255); // Azure
const RgbColor COLOR_BLUE(0, 0, 255);   // Blue
const RgbColor COLOR_VIOLET(127, 0, 255); // Violet
const RgbColor COLOR_MAGENTA(255, 0, 255); // Magenta
const RgbColor COLOR_ROSE(255, 0, 127);  // Rose

class MatrixDisplay
{
public:
    /**
     * Constructor: Initialize the NeoPixelBus with the given pin and set default brightness
     * @param pin GPIO pin connected to the Data In of the LED matrix
     */
    MatrixDisplay(uint8_t pin) : strip(TOTAL_LEDS, pin) {
        setBrightness(20); // Default to 20% brightness
    }

    /**
     * Initialize the display (must be called before any drawing operations)
     */
    void begin() {
        strip.Begin();
        strip.Show();
    }

    /**
     * Send the current canvas to the display, applying brightness dimming on the fly.
     */
    void show();

    // --- DISPLAY PROPERTIES ---

    /**
     * Get the width of the display in pixels
     * @return width in pixels
     */
    uint16_t getWidth() const {
        return MATRIX_WIDTH;
    }

    /**
     * Get the height of the display in pixels
     * @return height in pixels
     */
    uint16_t getHeight() const {
        return MATRIX_HEIGHT;
    }
    

    /**
     * Set the global brightness of the display (0-100%)
     * @param percent Brightness percentage (0-100)
     */
    void setBrightness(uint8_t percent) {
        if (percent > 100) {
            _brightness = 255;
            return;
        }
        if (percent < 0) {
            _brightness = 0;
            return;        
        }

        _brightness = (percent * 255) / 100;
    }

    /**
     * Get the current brightness percentage
     * @return Brightness percentage (0-100)
     */
    uint8_t getBrightness() const {
        return ((uint16_t)_brightness * 100) / 255;
    }

    // --- GRAPHIC PRIMITIVES ---

    /**
     * Clear the canvas (set all pixels to black)
     */
    void clear() {
        // Clear the canvas
        for(uint16_t i = 0; i < TOTAL_LEDS; i++) {
            _canvas[i] = RgbColor(0,0,0);
        }
    }

    /**
     * Fill the entire canvas with a specific color
     * @param color The color to fill the canvas with
     */
    void fill(RgbColor color) {
        // Fill the canvas with the specified color
        for(uint16_t i = 0; i < TOTAL_LEDS; i++) {
            _canvas[i] = color;
        }
    }

    /**
     * Draw a single pixel at (x, y) with the specified color
     * @param x X coordinate (0 to TOTAL_WIDTH-1)
     * @param y Y coordinate (0 to PANEL_HEIGHT-1)
     * @param color The color to set the pixel to
     */
    void drawPixel(int16_t x, int16_t y, RgbColor color);

        /**
     * Draw a filled rectangle
     * @param x Top-left X coordinate
     * @param y Top-left Y coordinate
     * @param w Width of the rectangle
     * @param h Height of the rectangle
     * @param color Rectangle color
     */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, RgbColor color);

    /**
     * Draw a rectangle outline
     * @param x Top-left X coordinate
     * @param y Top-left Y coordinate
     * @param w Width of the rectangle
     * @param h Height of the rectangle
     * @param color Rectangle color
     */
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, RgbColor color);

    /**
     * Draw a line between (x0, y0) and (x1, y1) using Bresenham's Algorithm
     * with optimization for horizontal and vertical lines
     * @param x0 Starting X coordinate
     * @param y0 Starting Y coordinate
     * @param x1 Ending X coordinate
     * @param y1 Ending Y coordinate
     * @param color Line color
     */
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, RgbColor color);

        // --- CANVAS METHODS ---

    /** Copy the current canvas to another canvas 
     * This is useful to create animations by manipulating the canvas before showing it, or to create effects that require reading the current pixel colors (e.g. for a fade effect, we need to read the current color of each pixel to calculate the new dimmed color).
     * @param targetCanvas The target canvas to copy to (must be pre-allocated with at least TOTAL_LEDS elements)
    */
    void copyCanvasTo(RgbColor* targetCanvas) const;

    /**
     * Copy another canvas to the current canvas
     * @param sourceCanvas The source canvas to copy from (must have at least TOTAL_LEDS elements)
     */
    void copyCanvasFrom(const RgbColor* sourceCanvas);

    /**
     * Copy a portion of another canvas to a portion of the current canvas (useful for drawing effects that require reading and manipulating a portion of the current canvas, e.g. for a wipe effect where we copy a portion of the current canvas and shift it to create the wiping motion)
     * @param sourceCanvas The source canvas to copy from (must have at least TOTAL_LEDS elements)
     * @param sourceX The top-left X coordinate of the portion to copy from
     * @param sourceY The top-left Y coordinate of the portion to copy from
     * @param width The width of the portion to copy
     * @param height The height of the portion to copy
     * @param destX The top-left X coordinate of the portion to copy to
     * @param destY The top-left Y coordinate of the portion to copy to
     */
    void copyCanvasFrom(const RgbColor* sourceCanvas, int16_t sourceX, int16_t sourceY, int16_t width, int16_t height, int16_t destX, int16_t destY);

    // --- COLOR METHODS ---

    /**
     * Calculate a linear color gradient between two colors
     * @param startColor The starting color of the gradient
     * @param endColor The ending color of the gradient
     * @param colors Output array to hold the resulting gradient colors (must be pre-allocated with at least colorsLength elements)
     * @param colorsLength The number of colors to generate in the gradient
     */
    void linearColorGradient(RgbColor startColor, RgbColor endColor, RgbColor* colors, uint8_t colorsLength) const;

    /**
     * Calculate a mirrored color gradient between two colors. The gradient will go from startColor to endColor and then back to startColor.
     * @param startColor The starting color of the gradient
     * @param endColor The ending color of the gradient
     * @param colors Output array to hold the resulting gradient colors (must be pre-allocated with at least colorsLength elements)
     * @param colorsLength The number of colors to generate in the gradient
     */
    void mirroredColorGradient(RgbColor startColor, RgbColor endColor, RgbColor* colors, uint8_t colorsLength) const;

private:
    // The hardware strip object
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
    NeoPixelBus<NeoGrbFeature, NeoEsp32LcdX8Ws2812xMethod> strip;
    #else
    NeoPixelBus<NeoGrbFeature, NeoEsp32I2s0Ws2812xMethod> strip;
    #endif

    // The Virtual Canvas (Stores the "True" colors)
    RgbColor _canvas[TOTAL_LEDS];

    uint8_t _brightness; // 0-255

    // Helper to calculate the hardware index from X, Y coordinates
    // Returns -1 if out of bounds
    int32_t getPixelIndex(int16_t x, int16_t y) const;
};
