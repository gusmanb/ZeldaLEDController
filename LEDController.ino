#include <FastLED.h>
#include <avr\sleep.h>
#include <avr\power.h>

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
	//Disable all unused features to reduce consumption
	ADCSRA = 0; //Disable analog inputs
	power_adc_disable(); // ADC converter
	power_spi_disable(); // SPI
	power_usart0_disable();// Serial (USART)
	power_timer1_disable();// Timer 1
	power_timer2_disable();// Timer 2
	power_twi_disable(); // TWI (I2C)

	//Set input pin as input with pullups
	pinMode(INPUT_BUTTON, INPUT_PULLUP);

	//Configure LED strip
	FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
		//.setCorrection(TypicalLEDStrip);
	
	//Set LED strip brightness
	FastLED.setBrightness(BRIGHTNESS);

	//Fill LED array with black color
	FillArray(&currentColor);
	FastLED.show();
}

void loop()
{
	CRGB targetColor;
	CRGB stepColor;

	//If input has happened
	if (wasEnabled != enable)
	{
		wasEnabled = enable;
		delay(100);
		if(enable)
			attachInterrupt(digitalPinToInterrupt(INPUT_BUTTON), HandleInput, LOW); //If enabled, attach interrupt
		
	}

	if (!enable)
	{
		//Interrupt happened in the middle of the loop function, exit
		if (wasEnabled != enable)
			return;

		//Fade to black from current color

		targetColor = CRGB::Black;

		for (int buc = 0; buc < 256; buc++)
		{
			stepColor = Interpolate(&currentColor, &targetColor, buc);
			FillArray(&stepColor);
			FastLED.show();
			delay(STEP_DELAY);
		}

		//Reset color index and current color
		currentColorIndex = -1;
		currentColor = targetColor;

		//Configurte sleep and disable interrupts
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		noInterrupts();
		
		//Wait until user has released the button (to avoid deadlocks)
		while (!digitalRead(INPUT_BUTTON))
			delay(50);

		//Clear interrupt and attach it to the input button
		EIFR |= (1 << digitalPinToInterrupt(INPUT_BUTTON));
		attachInterrupt(digitalPinToInterrupt(INPUT_BUTTON), HandleInput, LOW);
		EIFR |= (1 << digitalPinToInterrupt(INPUT_BUTTON));

		//Reenable interrupts and take a nap
		interrupts();
		sleep_mode();

		//Awake!
		sleep_disable();
		return;
	}

	//Next color!
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

	//Do a linear interpolation in 256 steps from current color to target color
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

	//Do a delay in the color but be aware if the user has pressed the input button
	for (int buc = 0; buc < 10; buc++)
	{
		if (!enable) //If pressed, exit
			return;

		delay(COLOR_DELAY / 10);
	}

}

void HandleInput()
{
	//Detach completely the interrupt
	detachInterrupt(digitalPinToInterrupt(INPUT_BUTTON));

	//Swap enable
	enable = !enable;

	//Wait until the user has released the button
	while (!digitalRead(INPUT_BUTTON))
		delay(50);

	//Small delay for debouncing
	delay(100);
	
}

//Function to interpolate two colors in 256 steps
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

//Fills the LED array with one concrete color
void FillArray(CRGB* Value)
{
	for (int buc = 0; buc < NUM_LEDS; buc++)
		leds[buc] = *Value;
}