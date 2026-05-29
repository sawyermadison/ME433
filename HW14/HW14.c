#include <stdio.h>
#include "pico/stdlib.h"

#define DT 17 //input data pin
#define SCK 16 //output clock pin

void initialize()
{
    gpio_init(DT);
    gpio_set_dir(DT, GPIO_IN);
    gpio_init(SCK);
    gpio_set_dir(SCK, GPIO_OUT);

    gpio_put(SCK, 0);
}

void read_bits(uint32_t *raw)
{
    while(gpio_get(DT) == 1){
        tight_loop_contents();
    }
    for(int i = 0; i < 24; i++) {
        gpio_put(SCK, 1);
        sleep_us(10);

        *raw = (*raw << 1) | gpio_get(DT);

        gpio_put(SCK, 0);
        sleep_us(10);
    }
    gpio_put(SCK, 1);
    sleep_us(10);
    gpio_put(SCK, 0);

    // sign-extend 24-bit two's complement to 32-bit signed int
    if (*raw & 0x800000) {
        *raw |= 0xFF000000;
    }
}

uint32_t get_average()
{
    uint32_t raw_data_for_avg[100];
    uint32_t tot = 0;
    for (int i = 0; i<100; i++){
        raw_data_for_avg[i] = 0;
        read_bits(&raw_data_for_avg[i]);
        tot += raw_data_for_avg[i];
    }
    uint32_t average = tot / 100;
    return average;
}


int main()
{
    stdio_init_all();
    initialize();

    sleep_ms(5000); // Wait for the USB connection to be established
    //printf("Entering loop\n");

    while(true){
        //printf("Calculating average...\n");
        int avg = get_average();
        //printf("Average calculated: %d\n", (int32_t)avg);

        printf("Enter the number of samples to read: ");
        int num = 0; 
        scanf("%d", &num);

        uint64_t t0 = to_us_since_boot(get_absolute_time());

        uint32_t raw_data[num];
        uint32_t filtered_data[num];
        uint64_t time[num];
        

        for (int i = 0; i<num; i++){
            raw_data[i] = 0;
            read_bits(&raw_data[i]);

            filtered_data[i] = 0.1 * raw_data[i] + 0.9 * avg;
            time[i] = to_us_since_boot(get_absolute_time()) - t0;
        }


        uint32_t tot = 0;
        for (int i = 0; i<num; i++){
            tot += raw_data[i];

            printf("%llu\t%d\t%d\n", time[i], (int32_t)raw_data[i], (int32_t)filtered_data[i]);
        }

        uint32_t average = tot / num;
        //printf("Average: %d\n", (int32_t)average);

        sleep_ms(1000); // Keep the program running to allow time for the output to be printed
    }


}
