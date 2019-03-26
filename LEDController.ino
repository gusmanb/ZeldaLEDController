#include <FastLED.h>

#define NUM_LEDS 20
#define LED_PIN 5
#define BRIGHTNESS 128
#define STEP_DELAY 5
#define COLOR_DELAY 2000

CRGB leds[NUM_LEDS];
CRGB currentColor = CRGB::Black;
int currentColorIndex = -1;

void setup() 
{ 
	Serial.begin(9600);

	FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS)
		.setCorrection(TypicalLEDStrip);
	
	FastLED.setBrightness(BRIGHTNESS);

	FillArray(&currentColor);
	FastLED.show();
}

void loop()
{
	currentColorIndex++;

	CRGB targetColor;

	if (currentColorIndex > 2)
		currentColorIndex = 0;

	switch (currentColorIndex)
	{
	case 0:
		targetColor = CRGB::Red;
		break;
	case 1:
		targetColor = CRGB::Green;
		break;
	case 2:
		targetColor = CRGB::Blue;
		break;
	}

	CRGB stepColor = currentColor;

	for (int buc = 0; buc < 256; buc++)
	{
		stepColor = Interpolate(&currentColor, &targetColor, buc);
		FillArray(&stepColor);
		FastLED.show();
		delay(STEP_DELAY);
	}

	currentColor = targetColor;

	delay(COLOR_DELAY);
}

CRGB Interpolate(CRGB* Start, CRGB* End, int Level)
{
	if (Level == 0)
		return *Start;

	float factor = (float)Level / 255.0f;

	int R = Start->red + (End->red - Start->red) * factor;
	int G = Start->green + (End->green - Start->green) * factor;
	int B = Start->blue + (End->blue - Start->blue) * factor;

	CRGB value;
	value.red = (uint8_t)R;
	value.green = (uint8_t)G;
	value.blue = (uint8_t)B;
	return value;
}

void FillArray(CRGB* Value)
{
	for (int buc = 0; buc < NUM_LEDS; buc++)
		leds[buc] = *Value;
}