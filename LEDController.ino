#include <FastLED.h>

#define NUM_LEDS 20
#define LED_PIN 5
#define BRIGHTNESS 128
#define STEP_DELAY 5
#define COLOR_DELAY 2000
//Input button can only be 2
#define INPUT_BUTTON 2

CRGB leds[NUM_LEDS];
CRGB currentColor = CRGB::Black;
int currentColorIndex = -1;

static volatile bool enable = true;
static volatile bool wasEnabled = false;

void setup() 
{ 
	pinMode(INPUT_BUTTON, INPUT_PULLUP);

	FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS)
		.setCorrection(TypicalLEDStrip);
	
	FastLED.setBrightness(BRIGHTNESS);

	FillArray(&currentColor);
	FastLED.show();
}

void loop()
{
	CRGB targetColor;
	CRGB stepColor;

	if (wasEnabled != enable)
	{
		wasEnabled = enable;
		delay(100);
		attachInterrupt(digitalPinToInterrupt(INPUT_BUTTON), HandleInput, LOW);
		
	}

	if (!enable)
	{

		if (currentColor.raw[0] != 0 || currentColor.raw[1] != 0 || currentColor.raw[2] != 0)
		{
			targetColor = CRGB::Black;

			for (int buc = 0; buc < 256; buc++)
			{
				stepColor = Interpolate(&currentColor, &targetColor, buc);
				FillArray(&stepColor);
				FastLED.show();
				delay(STEP_DELAY);
			}

			currentColorIndex = -1;
			currentColor = targetColor;
		}

		delay(1000);
		return;
	}

	currentColorIndex++;

	

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

	stepColor = currentColor;

	for (int buc = 0; buc < 256; buc++)
	{
		if (!enable)
		{
			currentColor = stepColor;
			return;
		}

		stepColor = Interpolate(&currentColor, &targetColor, buc);
		FillArray(&stepColor);
		FastLED.show();
		delay(STEP_DELAY);
	}

	currentColor = targetColor;

	for (int buc = 0; buc < 10; buc++)
	{
		if (!enable)
			return;
		delay(COLOR_DELAY / 10);
	}

}

void HandleInput()
{
	noInterrupts();
	detachInterrupt(digitalPinToInterrupt(INPUT_BUTTON));
	enable = !enable;

	while (!digitalRead(INPUT_BUTTON))
		delay(50);

	interrupts();

	delay(100);
	
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