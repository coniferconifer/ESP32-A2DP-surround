// based on https://www.pschatzmann.ch/home/2023/07/08/esp32-mixing-a2dp-with-a-sine-signal//
// and ESP32 A2DP Arduino library at https://github.com/pschatzmann/ESP32-A2DP
// converted into surround speaker emulator mimicing MX-15 speaker matrix by Tetsuo Nagaoka
// 
// This program works as ESP-32 A2DP receiver and converts R,L signals into 2R-L and 2L-R.
// 
// R+L for center speaker needs to be made from resister mixer as follows.
// R and L channel come from DAC (PCM5102A) output.
//
// 2kOhm x 2 , 100kOhm for GND
// R-----2kOhm -------+-------Rch Output
//                    |
//                    +--100kOhm-----GND Input/Output
//                    |
// L-----2kOhm -------+-------Lch Output
/* PCM5102A board is connected by
GPIO22 -> DIN
GPIO15 -> LCK
GPIO14 -> BCK
5V -> VIN (inout to the on board 3.3V regulator)
GND -> GND
default pin by https://github.com/pschatzmann/ESP32-A2DP

On board switch settings: H1L（FLT),H2L（DEMP),H4L（FMT) are shorted to GND
H3L（XMST un mute ) is connected to High side(3.3V)

copyright 2024 by coniferconifer
LICENSED under apache

To avoid followin message, 

>Sketch uses 1889957 bytes (144%) of program storage space. Maximum is 1310720 bytes.
>Global variables use 62276 bytes (19%) of dynamic memory, leaving 265404 bytes for local variables. Maximum is 327680 bytes.
>Sketch too big; see https://support.arduino.cc/hc/en-us/articles/360013825179 for tips on reducing it.
>text section exceeds available space in board
>
>Compilation error: text section exceeds available space in board

select partition scheme "NO OTA" from Arduino IDE's tools menu

*/

#include "AudioTools.h"
#include "BluetoothA2DPSink.h" // https://github.com/pschatzmann/ESP32-A2DP

AudioInfo info(44100, 2, 16);
BluetoothA2DPSink a2dp_sink;
I2SStream i2s;
OutputMixer<int16_t> mixer(i2s, 2);

const int buffer_size = 16 * 20;  // split up the output into small slices
uint8_t surround_buffer[buffer_size];

uint8_t mode = 2;  // 0.. passthrough , 1..speaker matrix , 2..MX-15 mode speaker matrix
#define PASSTHROUGH 0
#define SPEAKERMATRIX 1
#define MX15MODE 2
void setup() {
  mode = MX15MODE;

  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);
  Serial.printf("ESP32 A2DP surround processor by speaker matrix emulation\r\n");
  switch (mode){
    case PASSTHROUGH:
      Serial.printf("pass through mode\r\n");
      break;
    case SPEAKERMATRIX:
      Serial.printf("L-R,R-L Speaker Matrix mode\r\n");
      break;
    case MX15MODE:
      Serial.printf("MX-15 (2L-R,2R-L) mode\r\n");
      break;
    default:
      break;
  }
  // setup Output mixer with min necessary memory
  mixer.begin(buffer_size);
  // Register data callback
  a2dp_sink.set_stream_reader(read_data_stream, false);

  // Start Bluetooth Audio Receiver
  a2dp_sink.set_auto_reconnect(false);
  a2dp_sink.start("a2dp-i2s");

  // Update sample rate
  info.sample_rate = a2dp_sink.sample_rate();

  // setup output
  auto cfg = i2s.defaultConfig();
  cfg.copyFrom(info);
  // cfg.pin_data = 23;
  cfg.buffer_count = 8;
  cfg.buffer_size = 256;
  i2s.begin(cfg);
}

void loop() {
  delay(100);
}

void read_data_stream(const uint8_t *data, uint32_t length) {
  // To keep the mixing buffer small, we split up the output into small slices
  const int sliceSize = buffer_size;
  int16_t RchData, LchData;
  //Serial.printf("length=%d\r\n", length);
  for (int j = 0; j < length; j += sliceSize) {
    // Write j'th a2dp slice
    int remaining = min(sliceSize, static_cast<int>(length - j));
    //   Serial.printf("remaining=%d j=%d\r\n",remaining,j);

    for (int i = j; i < (j + remaining); i = i + 4) {
      int16_t leftData = (int16_t)(data[i] | (data[i + 1] << 8));
      int16_t rightData = (int16_t)(data[i + 2] | (data[i + 3] << 8));
      //#define PASSTHROUGH
      switch (mode) {
        case PASSTHROUGH:
          LchData = leftData;
          RchData = rightData;
          break;
        case SPEAKERMATRIX:  // output surround soun for rear speaker : R-L, L-R
          LchData = leftData - rightData;
          RchData = rightData - leftData;
          break;
        default:  //MX15MODE
          // Tetsuo Nagaoka's MX-15 matrix speaker mode by default
          LchData = (leftData - rightData / 2) ;
          RchData = (rightData - leftData / 2) ;
          // you can get R+C for center speaker by analog resister mixer from Rch and Lch
          // (L/2-R/4) + (R/2-L/4) => (R+L)/4
          // two 2kOhm and one 100kOhm are enough for mixer
          break;
      }
      surround_buffer[i - j] = (uint8_t)(LchData & 0xff);
      surround_buffer[i - j + 1] = (uint8_t)(LchData >> 8);
      surround_buffer[i - j + 2] = (uint8_t)(RchData & 0xff);
      surround_buffer[i - j + 3] = (uint8_t)(RchData >> 8);

      // Serial.printf("%d %d  i/4-j/4 %d\r\n", leftData,rightData,i/4-j/4);
    }

    mixer.write(surround_buffer, remaining);
    mixer.write(surround_buffer, remaining);
  }
}