
// // #include <boards/pico.h>
#include <hardware/gpio.h>
// #include <hardware/spi.h>
// #include <hardware/i2c.h>
#include <pico/time.h>

#include <motor.h>
// #include <pico/stdio.h>

#define SERVO1 22
#define SERVO2 23
#define SERVO3 24
#define SERVO4 25

int main() {
  // Servo
  motor_init(SERVO2);
  motor_set(SERVO2, 1500);
  
  // BLDC
  sleep_ms(2000);
  motor_init(SERVO1);
  motor_set(SERVO1, 2000);
  sleep_ms(2000);
  motor_set(SERVO1, 1000);
  sleep_ms(2000);
  motor_set(SERVO1, 500);
  sleep_ms(2000);

  // motor_set(SERVO2, 1000);
  // motor_set(SERVO1, 1200);
  // sleep_ms(500);

  // for (int i = 0; i < 16 * 2; i++) {
  //   motor_set(SERVO1, 1100);
  //   sleep_ms(1500);
  //   // motor_set(SERVO1, 1000);
  //   // sleep_ms(300);
  // }

  motor_set(SERVO1, 1200);
  sleep_ms(250);
  motor_set(SERVO1, 1300);
  motor_set(SERVO2, 1000);
  sleep_ms(15000);

  motor_set(SERVO1, 1000);
  
  // motor_set(SERVO2, 1800);
  // sleep_ms(500);

  // motor_set(SERVO1, 1000);
  // motor_set(SERVO2, 1500);

  while(1) {
      // motor_set(SERVO2, 1200);
      // sleep_ms(1000);
      // motor_set(SERVO2, 1500);
      // sleep_ms(1000);
      // motor_set(SERVO2, 1800);
      // sleep_ms(1000);
      // motor_set(SERVO2, 1500);
      // sleep_ms(1000);
    }
}

// int main() {
//   // Camera
//   gpio_init(0); // SDA
//   gpio_set_function(0, GPIO_FUNC_I2C);
//   gpio_pull_up(0);

//   gpio_init(1); // SCL
//   gpio_set_function(1, GPIO_FUNC_I2C);
//   gpio_pull_up(1);

//   i2c_init(i2c0, 100000); // 100kHz

//   gpio_init(5); // io
//   gpio_init(3);
//   gpio_init(2);
//   gpio_init(4);
//   gpio_init(6);
//   gpio_init(8);
//   gpio_init(9);
//   gpio_init(11);

//   gpio_init(7); // pclk
//   gpio_init(10); // mclk
//   gpio_init(12); // hd
//   gpio_init(13); // pwdn
//   gpio_init(14); // vd
//   gpio_init(15); // rst

//   // Accelerometer
//   spi_init(spi0, 1000 * 1000);
//   gpio_set_function(16, GPIO_FUNC_SPI);
//   gpio_init(17);
//   gpio_set_function(18, GPIO_FUNC_SPI);
//   gpio_set_function(19, GPIO_FUNC_SPI);

//   gpio_init(20);
//   gpio_init(21);

//   // BLDC
//   bldc_init(22);

//   bldc_set_speed(22, 2000);
//   sleep_ms(1000);
//   bldc_set_speed(22, 1000);
//   sleep_ms(2000);

//   while (1) {
//     bldc_set_speed(22, 1100);
//   }
