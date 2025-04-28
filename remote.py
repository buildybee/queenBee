import tkinter as tk
import serial
import json  # Import JSON module for serialization

# Initialize serial port
ser = serial.Serial('/dev/ttyUSB2', 115200, timeout=1)

def send_serial_data():
    """Send the joystick values through the serial port as a structured dictionary."""
    data = {
        'P': sliders[0].get(),  # Pitch
        'R': sliders[1].get(),  # Roll
        'Y': sliders[2].get(),  # Yaw
        'T': sliders[3].get(),  # Throttle
        'A1': sliders[4].get(), # Auxiliary 1
        'A2': sliders[5].get(), # Auxiliary 2
        'A3': sliders[6].get(), # Auxiliary 3
        'A4': sliders[7].get()  # Auxiliary 4
    }
    data_str = json.dumps(data)  # Serialize the dictionary to a JSON string
    ser.write(data_str.encode())  # Send as bytes

def update_serial():
    """Update serial data at regular intervals."""
    send_serial_data()
    root.after(5, update_serial)  # Call this function every 5 ms

def reset_to_center(event, slider):
    """Reset the slider to the middle value when mouse button is released."""
    slider.set(122.5)

# Initialize the main window
root = tk.Tk()
root.title("8-Channel Joystick Simulator")

slider_Name = ["Pitch","Roll","Throttle","Yaw","Aux1","Aux2","Aux3","Aux4"]
# Create sliders for 8 channels
sliders = []
for i in range(8):
    if i == 1 or i == 3 or i == 0:  # Channels 2 and 3 as joystick buttons
        slider = tk.Scale(root, from_=0, to=255, orient=tk.HORIZONTAL, label=f'Channel {slider_Name[i]}')
        slider.set(122.5)  # Set the initial value to the middle
        slider.bind("<ButtonRelease-1>", lambda event, s=slider: reset_to_center(event, s))  # Return to center on release
        slider.pack(fill=tk.X)
        sliders.append(slider)
    else:
        slider = tk.Scale(root, from_=0, to=255, orient=tk.HORIZONTAL, label=f'Channel {slider_Name[i]}')
        slider.pack(fill=tk.X)
        sliders.append(slider)

# Start sending serial data
update_serial()

# Run the main loop
root.mainloop()

# Close the serial port when the program ends
ser.close()
