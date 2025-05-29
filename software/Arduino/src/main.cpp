#include <Stepper.h>
#include <Arduino.h>

// Defines the number of steps per rotation
const int stepsPerRevolution = 1000;
const float lockRevolution = 1.0 / 6.0; // Changed to float for proper calculation
const float unlockRevolution = lockRevolution * 4;

#define laserSensorPin A0
#define armPin 6
#define laserOutPin 7
#define startPin 5
#define crowdPin 2

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 8, 10, 9, 11);

int nonZeroCount = 0; // Counter for consecutive non-zero readings
bool laserTriggered = false;
bool motorLocked = false;
unsigned long unlockTime = 0;
bool timerStarted = false;

void setup()
{
    Serial.begin(9600);
    pinMode(laserSensorPin, INPUT);
    pinMode(armPin, INPUT);   // armPin is an input
    pinMode(startPin, INPUT); // startPin is an input
    pinMode(laserOutPin, OUTPUT);
    pinMode(crowdPin, OUTPUT);
    digitalWrite(laserOutPin, LOW); // Initialize laser output to LOW
    digitalWrite(crowdPin, LOW);

    myStepper.setSpeed(2); // RPM speed
}

int crowdActivated = -1000;

void loop()
{
    int sensorValue = analogRead(laserSensorPin);
    int armValue = digitalRead(armPin); // Read armPin input
    int startValue = digitalRead(startPin);




    // Lock mechanism (counter-clockwise)
    if (armValue == HIGH && !motorLocked && millis() > 1000)
    {
        Serial.println("KILL MEEEEEEEEEE");

        myStepper.step(-stepsPerRevolution * lockRevolution); // Negative for counter-clockwise
        motorLocked = true;
        Serial.println("Motor locked (counter-clockwise)");
    }

    // Start timer when start pin is activated
    if (startValue == HIGH && !timerStarted)
    {
        unlockTime = millis() + 80000; // 90 seconds from now
        timerStarted = true;
        Serial.println("Timer started - will unlock in 90 seconds");
    }

    // Unlock after 90 seconds (clockwise)
    if (timerStarted && millis() >= unlockTime && motorLocked)
    {
        myStepper.step(stepsPerRevolution * unlockRevolution); // Positive for clockwise
        motorLocked = false;
        timerStarted = false;
        Serial.println("Motor unlocked (clockwise)");
    }

    // Laser sensor detection with modified logic
    if (sensorValue != 0)
    {
        nonZeroCount++; // Increment counter if reading is not 0
    }
    else
    {
        nonZeroCount = 0; // Reset counter if reading is 0
    }

    // If laser is broken for more than 50 loops, output HIGH
    if (nonZeroCount > 20)
    {
        if (!laserTriggered)
        {
            laserTriggered = true;
            digitalWrite(laserOutPin, HIGH);
            Serial.println("Laser broken for prolonged period - output HIGH");


            digitalWrite(crowdPin,HIGH);
            crowdActivated = millis();
        }
    }
    else
    {
        if (laserTriggered)
        {
            laserTriggered = false;
            digitalWrite(laserOutPin, LOW);
            Serial.println("Laser restored - output LOW");
        }
    }

    if(millis() > crowdActivated + 1500){
        digitalWrite(crowdPin, LOW);
    }

    delay(5); // Small delay to prevent flooding the serial monitor
}
