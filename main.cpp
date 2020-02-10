/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "can_md_peripheral.h"
#include "mbed.h"

static const int offset_id_number = 0x300; // 0 ~ 0x7E0

CAN can(PA_11, PA_12); // RD, TD
// BusIn dip_sw(PA_4, PA_3, PA_1, PA_0); //下位ビット ~ 上位ビット
BusIn dip_sw(PB_3, PB_4, PB_5, PB_6); //下位ビット ~ 上位ビット
/**/
static const int total_motor = 1;

CANMDPeripheral motor[total_motor] = {
    //    CANMDPeripheral(can, PA_5, PA_6, PA_7, PB_3, PB_4, PA_0, PA_1),    // pwm3/2
    //    CANMDPeripheral(can, PA_8, PA_9, PA_10, PB_0, PB_1, PA_2, PA_4),   // pwm1/3
    CANMDPeripheral(can, PB_0, PA_6, PA_7, PB_1, PA_5, PA_10, PA_9), // Version 1.2.0 ~ はPA_10とPA_9を交換
};

Serial pc(USBTX, USBRX);

CANMessage msg;

Ticker ticker;

void set_up_id();
void get_data();

void adjust();

int main()
{
    pc.baud(115200);
    pc.printf("CAN MD Peripheral Solo 1.0.0\n");

    set_up_id();
    for (int i = 0; i < total_motor; i++)
    {
        motor[i].connect();
    }

    can.attach(&get_data);
    ticker.attach(&adjust, 0.001);
    // ticker.attach(&adjust, 0.001);

    while (true)
    {
        wait_us(250000);
        for (int i = 0; i < total_motor; i++)
        {
            if (motor[i].has_recieved_initial_value() == false)
            {
                motor[i].connect();
                // printf("Please Connect!");
            }
        }
        
    }
}

void set_up_id()
{
    dip_sw.mode(PullDown);
    const int dip_id = dip_sw;

    // TODO can filterを使う
    int rx_filter_id = dip_id * 16 + offset_id_number;
    //can.filter(rx_filter_id, 0x7F0, CANAny, 1); // handleは0にしたほうがいいかも

    for (int i = 0; i < total_motor; i++)
    {
        int main_id = rx_filter_id + (i * 2);
        motor[i].id(main_id);
    }
    debug("Motor-0 base id is %d.\n", motor[0].id());
    debug("dip: %d\n", dip_id);
}

void get_data()
{
    //    printf("GET!");
    can.read(msg); // can.read(msg, 1);
    for (int i = 0; i < total_motor; i++)
    {
        if (msg.id == motor[i].id())
        {
            // debug("receive: %d\n", i);
            motor[i].decode_can_message(msg.data);
            // debug("duty: %1.3f, state: %d\r", motor[i].duty_cycle(), motor[i].state());
            break;
        }
    }
}

void adjust()
{
    for (int i = 0; i < total_motor; i++)
    {
        motor[i].adjust();
        motor[i].release_time_dec();
    }
}