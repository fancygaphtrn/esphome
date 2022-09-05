#include "esphome.h"
using namespace esphome;

class MyCustomFloatOutput : public Component, public FloatOutput {
 public:
  void setup() override {
    // This will be called by App.setup()
    pinMode(D1, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
  }

  void write_state(float state) override {
    // state 1 = high
	// state 0.66 = medium
	// state 0.33 = low
	// state 0 = off

    ESP_LOGD("custom", "State changed to %f", state);

    digitalWrite(D1, LOW);
    digitalWrite(D5, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(D7, LOW);
    
    if ( state == 1 ) {
      ESP_LOGD("custom", "High");
      digitalWrite(D5, HIGH);
      delay(1000);
      digitalWrite(D5, LOW);
      delay(500);
      digitalWrite(D5, HIGH);
      delay(1000);
      digitalWrite(D5, LOW);
    }
    if ( state > 0.6f && state < 0.7f ) {
      ESP_LOGD("custom", "Medium");
      digitalWrite(D6, HIGH);
      delay(1000);
      digitalWrite(D6, LOW);
      delay(500);
      digitalWrite(D6, HIGH);
      delay(1000);
      digitalWrite(D6, LOW);
    }
    if ( state > 0.3f && state < 0.4f ) {
      ESP_LOGD("custom", "Low");
      digitalWrite(D7, HIGH);
      delay(1000);
      digitalWrite(D7, LOW);
      delay(500);
      digitalWrite(D7, HIGH);
      delay(1000);
      digitalWrite(D7, LOW);
    }
    if ( state == 0 ) {
     ESP_LOGD("custom", "Off");
      digitalWrite(D1, HIGH);
      delay(1000);
      digitalWrite(D1, LOW);
      delay(500);
      digitalWrite(D1, HIGH);
      delay(1000);
      digitalWrite(D1, LOW);
    }
  }
};