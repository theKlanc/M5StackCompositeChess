#include <Arduino.h>
#include <M5Core2.h>
#include <driver/dac.h>
#include <string>
#include "driver/i2s.h"
#include "freertos/queue.h"

void dacSet(float val, dac_channel_t dacChannel = DAC_CHANNEL_1){
  dac_output_voltage(dacChannel, int(float(255)*val));
}

bool systemUpdate(){
  return true;
}

void loop(){exit(0);}

static const int hz = 60;
static const int sampleRate = 44100;
static const int bufferCount = 32;
static const int bufferSize = 1024;

static int currentBuffer = 0;

static i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
    .sample_rate = sampleRate,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_BITS_PER_SAMPLE_16BIT), /* the DAC module will only take the 8bits from MSB */
    .channel_format =  i2s_channel_fmt_t(I2S_CHANNEL_FMT_RIGHT_LEFT),
    .communication_format = i2s_comm_format_t(I2S_CHANNEL_FMT_RIGHT_LEFT),
};

void setup() {
    i2s_config.dma_buf_len = bufferSize;
    i2s_config.dma_buf_count = bufferCount;
    i2s_config.use_apll = false;
    
    M5.begin();
    {
        i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);   //install and start i2s driver
        i2s_set_pin(I2S_NUM_0, NULL); //for internal DAC, this will enable both of the internal channels
        i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
        i2s_zero_dma_buffer(I2S_NUM_0);
    }
    pinMode(G35, INPUT);
    Serial.begin(115200);
    char* buffer = (char*)malloc(bufferCount * bufferSize * sizeof(char));
    for(int i =0; i < bufferCount*bufferSize;++i){
        buffer[i] = i%256;
    }
    int potVal = 0;
    size_t bytes_written;
    
    while(systemUpdate()){
        M5.update();
        potVal = analogRead(G35);
        i2s_write(I2S_NUM_0, buffer + bufferSize*currentBuffer, bufferSize * sizeof(char), &bytes_written, 100);
        currentBuffer = (currentBuffer+1)%bufferCount;
        Serial.println(int(bytes_written));
    }
    i2s_driver_uninstall(I2S_NUM_0); //stop & destroy i2s driver
}