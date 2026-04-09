import board
import pwmio # get access to PWM
import time

servo = pwmio.PWMOut(board.GP13, variable_frequency=True)
servo.frequency = 50
servo.duty_cycle = 0 # initially off, at 16bit number so max on is 65535

servo_max = (int)(65535*(6400/50000))
servo_min = (int)(65535*(1400/50000))

while True:
    # start duty cycle at 0, every loop increase by 100 
    # until getting to the max of 65535
    for i in range(servo_min, servo_max, 2):
        servo.duty_cycle = i
        time.sleep(0.001)
    for i in range(servo_max, servo_min, -2):
        servo.duty_cycle = i
        time.sleep(0.001)