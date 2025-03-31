import paho.mqtt.client as mqtt
import json
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Configuratie
MQTT_BROKER = "192.168.11.70"
MQTT_TOPIC = "robot/sensors"

# Globale variabelen
fig, ax = plt.subplots()
scatter = ax.scatter([], [], c='red', s=50)
robot_arrow = ax.arrow(0, 0, 0.5, 0.5, head_width=0.2, color='blue')
walls = []

def on_connect(client, userdata, flags, rc):
    print(f"Verbonden met MQTT broker (code: {rc})")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    global walls
    try:
        data = json.loads(msg.payload.decode())
        
        # Basis SLAM-logica
        x, y = data['pos']['x'], data['pos']['y']
        yaw_rad = np.radians(data['yaw'])
        
        # Bereken muren
        new_walls = []
        for direction in ['n', 'e', 's', 'w']:
            dist = data['distances'][direction]
            angle = {'n': 0, 'e': 90, 's': 180, 'w': 270}[direction]
            wall_x = x + dist * np.cos(yaw_rad + np.radians(angle))
            wall_y = y + dist * np.sin(yaw_rad + np.radians(angle))
            new_walls.append((wall_x, wall_y))
        
        walls = new_walls
        update_plot(x, y, yaw_rad)
        
    except Exception as e:
        print(f"Fout bij verwerken bericht: {e}")

def update_plot(x, y, yaw):
    ax.clear()
    
    # Plot robot
    ax.arrow(x, y, 
             0.5*np.cos(yaw), 0.5*np.sin(yaw), 
             head_width=0.2, color='blue')
    
    # Plot muren
    if walls:
        wall_x, wall_y = zip(*walls)
        ax.scatter(wall_x, wall_y, c='red', s=50)
    
    # Plot instellingen
    ax.set_xlim(-10, 10)
    ax.set_ylim(-10, 10)
    ax.grid(True)
    ax.set_title(f"Robot Positie: ({x:.2f}, {y:.2f})")
    plt.draw()

def animate(i):
    pass  # Lege animatie voor realtime updates

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, 1883, 60)
ani = FuncAnimation(fig, animate, interval=100)
plt.show()

try:
    client.loop_forever()
except KeyboardInterrupt:
    print("Afsluiten...")
    client.disconnect()