import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import math
import json
import time
from collections import defaultdict
import socket
from matplotlib.animation import FuncAnimation

class RobotMapper:
    def __init__(self):
        # World state
        self.obstacles = defaultdict(list)
        self.robot_path = []
        
        # Current state
        self.robot_x = 0
        self.robot_y = 0
        self.orientation = 0
        self.new_data = False
        
        # Visualization
        self.fig, self.ax = plt.subplots(figsize=(10, 10))
        self.setup_plot()
        
    def setup_plot(self):
        self.ax.clear()
        self.ax.grid()
        self.ax.set_aspect('equal')
        self.ax.set_title("Robot Map - Initializing...")
        self.obstacles_scatter = self.ax.scatter([], [], c='red', s=50, label='Obstacles')
        self.path_plot, = self.ax.plot([], [], 'b-', alpha=0.5, label='Path')
        self.robot_plot = self.ax.scatter([], [], c='green', s=100, label='Robot')
        self.ax.legend()
        
    def update_sensors(self, sensor_data):
        """Convert relative sensor distances to absolute world coordinates"""
        obstacles_rel = {
            'north': (0, sensor_data["n"]),
            'east': (sensor_data["e"], 0),
            'south': (0, -sensor_data["s"]),
            'west': (-sensor_data["w"], 0)
        }
        
        for direction, (dx, dy) in obstacles_rel.items():
            rad = math.radians(self.orientation)
            x_rot = dx * math.cos(rad) - dy * math.sin(rad)
            y_rot = dx * math.sin(rad) + dy * math.cos(rad)
            
            world_x = self.robot_x + x_rot
            world_y = self.robot_y + y_rot
            
            self.obstacles[(world_x, world_y)].append(direction)
        
        self.move_robot(5)  # Simulated movement
        self.new_data = True
        
    def move_robot(self, distance):
        rad = math.radians(self.orientation)
        self.robot_x += distance * math.sin(rad)
        self.robot_y += distance * math.cos(rad)
        self.robot_path.append((self.robot_x, self.robot_y))
        
    def update_plot(self, frame):
        if not self.new_data:
            return self.obstacles_scatter, self.path_plot, self.robot_plot
        
        if self.obstacles:
            obs_x, obs_y = zip(*self.obstacles.keys()) if self.obstacles else ([], [])
            self.obstacles_scatter.set_offsets(list(zip(obs_x, obs_y)))
            
        if len(self.robot_path) > 1:
            path_x, path_y = zip(*self.robot_path)
            self.path_plot.set_data(path_x, path_y)
        
        self.robot_plot.set_offsets([[self.robot_x, self.robot_y]])
        self.ax.set_title(f"Robot Position: ({self.robot_x:.1f}, {self.robot_y:.1f})")
        
        if self.obstacles or self.robot_path:
            all_x = [self.robot_x] + (list(obs_x) if self.obstacles else [])
            all_y = [self.robot_y] + (list(obs_y) if self.obstacles else [])
            padding = max(max(all_x)-min(all_x), max(all_y)-min(all_y)) * 0.2
            self.ax.set_xlim(min(all_x)-padding, max(all_x)+padding)
            self.ax.set_ylim(min(all_y)-padding, max(all_y)+padding)
        
        self.new_data = False
        return self.obstacles_scatter, self.path_plot, self.robot_plot

# Updated to handle both V1 and V2 callback APIs
def on_connect(client, userdata, flags, rc, *args):
    if rc == 0:
        print("Connected to MQTT Broker!")
        client.subscribe("sensors/recalculated", qos=1)
    else:
        print(f"Connection failed with code {rc}")

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        print(f"Received sensor data: {data}")
        mapper.update_sensors(data)
    except Exception as e:
        print(f"Error processing message: {e}")

# Initialize mapper
mapper = RobotMapper()

# Use the newer MQTT client API version
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

# Set callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connection parameters - MODIFY THESE!
BROKER_ADDRESS = "192.168.4.1"  # Fixed IP address (no spaces)
BROKER_PORT = 1883
BROKER_TIMEOUT = 60

try:
    print(f"Attempting to connect to {BROKER_ADDRESS}:{BROKER_PORT}...")
    
    # Add error handling for connection
    client.connect(BROKER_ADDRESS, BROKER_PORT, BROKER_TIMEOUT)
    client.loop_start()
    
    # Use FuncAnimation with explicit save_count to avoid warning
    ani = FuncAnimation(
        mapper.fig, 
        mapper.update_plot, 
        interval=100,
        cache_frame_data=False,  # Disable caching to avoid warning
        save_count=100  # Limit the number of frames stored
    )
    print("Running visualization. Close window to exit.")
    plt.show()
    
except KeyboardInterrupt:
    print("\nShutting down gracefully...")
except socket.gaierror:
    print("\nERROR: Could not resolve broker address. Please check:")
    print(f"- IP address is correct: {BROKER_ADDRESS}")
    print("- Network connection is active")
    print("- Broker is running and accessible")
except ConnectionRefusedError:
    print("\nERROR: Connection refused. Please check:")
    print(f"- Broker is running on port {BROKER_PORT}")
    print("- No firewall blocking the connection")
    print("- Authentication requirements (if any)")
except Exception as e:
    print(f"\nERROR: {str(e)}")
finally:
    client.loop_stop()