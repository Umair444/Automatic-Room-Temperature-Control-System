#define NUM_SAMPLES 10

int sum = 0;                    // sum of samples taken
unsigned char sample_count = 0; // current sample number
float voltage = 0.0;            // calculated voltage

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    while (sample_count < NUM_SAMPLES) {
        sum += analogRead(A2);
        sample_count++;
        delay(10);
    }
    voltage = (((float)sum / (float)NUM_SAMPLES * 5.015) / 1024.0);
    Serial.print(voltage);
    Serial.println (" V");
    sample_count = 0;
    sum = 0;
}




//References: 
/*
 * https://startingelectronics.org/articles/arduino/measuring-voltage-with-arduino/
 * 
 */
