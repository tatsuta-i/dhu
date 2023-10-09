import serial
import time
import pyautogui

# csv形式の文字列をint型配列に変換
def csv_to_int_list(csv_string):
    """
    Convert a CSV formatted string to a list of integers.
    
    Parameters:
    - csv_string: String formatted as CSV.
    
    Returns:
    - List of integers.
    """
    return [int(x) for x in csv_string.split(',')]



ser = serial.Serial('/dev/ttyUSB0', 115200)  # Adjust the device and baud rate accordingly
ser.write(b'Hello ESP32\n')

while True:
    
    command = input()
    time.sleep(0.1)
    pyautogui.press('enter')
    if(command == 'l'):
        ser.write(b'Listing_completed.\n')
        

    time.sleep(0.9)
    
    while ser.in_waiting:
    #if True:
        data = ser.readline().decode('utf-8').strip()
        # data = "sendCode:1,2,3"
        print(f"Received: {data}")
        if 'sendCode:' in data:
            code = data[data.find(":")+1 : ]
            #print(code)
            print(csv_to_int_list(code))



# 使用例
# csv_string = "1,2,3,4,5"
# int_list = csv_to_int_list(csv_string)
# print(int_list)  # 出力: [1, 2, 3, 4, 5]
# print("sum: {}".format(sum(int_list)))