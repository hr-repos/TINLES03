import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import json
from matplotlib.animation import FuncAnimation
import platform
import keyboard  # For WASD controls
from collections import defaultdict
import socket
from matplotlib.animation import FuncAnimation

class RobotMapper:
    def __init__(self):
        # World state
        self.obstacles = defaultdict(list)  # Store obstacles with directions
        self.robot_path = []
        
        # Current state
        self.origin_set = False
        self.origin_x = 0
        self.origin_y = 0
        self.robot_x = 0
        self.robot_y = 0
        self.orientation = 0
        self.new_data = False
        
        # Visualization
        self.fig, self.ax = plt.subplots(figsize=(10, 10))
        self.setup_plot()
        
        # Keyboard controls (Windows compatible)
        if platform.system() == 'Windows':
            self.setup_keyboard()
        
    def setup_plot(self):
        self.ax.clear()
        self.ax.grid()
        self.ax.set_aspect('equal')
        self.ax.set_title("Robot Map - Waiting for origin...")
        self.obstacles_scatter = self.ax.scatter([], [], c='red', s=50, label='Obstacles')
        self.path_plot, = self.ax.plot([], [], 'b-', alpha=0.5, label='Path')
        self.robot_plot = self.ax.scatter([], [], c='green', s=100, label='Robot')
        self.ax.legend()
        
    def setup_keyboard(self):
        try:
            keyboard.on_press_key('w', lambda _: self.move_robot(0, 5))
            keyboard.on_press_key('a', lambda _: self.move_robot(-5, 0))
            keyboard.on_press_key('s', lambda _: self.move_robot(0, -5))
            keyboard.on_press_key('d', lambda _: self.move_robot(5, 0))
        except Exception as e:
            print(f"Keyboard setup failed (might not work on this platform): {e}")
        
    def update_position(self, x, y):
        if not self.origin_set:
            self.origin_x = x
            self.origin_y = y
            self.origin_set = True
            print(f"Origin set to: ({self.origin_x}, {self.origin_y})")
        
        # Calculate relative position
        self.robot_x = x - self.origin_x
        self.robot_y = y - self.origin_y
        self.robot_path.append((self.robot_x, self.robot_y))
        self.update_plot()
        
    def process_sensor_data(self, sensor_data):
        """Convert relative sensor distances to absolute world coordinates"""
        obstacles_rel = {
            'north': (0, sensor_data["n"]),
            'east': (sensor_data["e"], 0),
            'south': (0, -sensor_data["s"]),
            'west': (-sensor_data["w"], 0)
        }
        
        for direction, (dx, dy) in obstacles_rel.items():
            # Convert to world coordinates based on robot orientation
            rad = math.radians(self.orientation)
            x_rot = dx * math.cos(rad) - dy * math.sin(rad)
            y_rot = dx * math.sin(rad) + dy * math.cos(rad)
            
            world_x = self.robot_x + x_rot
            world_y = self.robot_y + y_rot
            
            self.obstacles[(world_x, world_y)].append(direction)
        
        self.move_robot(5)  # Simulated movement
        self.new_data = True
        
    def move_robot(self, dx, dy):
        """Manual movement with WASD"""
        self.robot_x += dx
        self.robot_y += dy
        self.robot_path.append((self.robot_x, self.robot_y))
        print(f"Manual move to: ({self.robot_x}, {self.robot_y})")
        self.update_plot()
        
    def update_plot(self, frame):
        if not self.new_data:
            return self.obstacles_scatter, self.path_plot, self.robot_plot
        
        if self.obstacles:
            obs_x, obs_y = zip(*self.obstacles.keys())
            self.obstacles_scatter.set_offsets(list(zip(obs_x, obs_y)))
        
        # Update path
        if len(self.robot_path) > 0:
            path_x, path_y = zip(*self.robot_path)
            self.path_plot.set_data(path_x, path_y)
        
        # Update robot position
        self.robot_plot.set_offsets([[self.robot_x, self.robot_y]])
        self.ax.set_title(f"Robot Position: ({self.robot_x:.1f}, {self.robot_y:.1f})")
        
        # Auto-scale view
        if self.obstacles or len(self.robot_path) > 1:
            all_x = [self.robot_x] + (list(obs_x) if self.obstacles else [])
            all_y = [self.robot_y] + (list(obs_y) if self.obstacles else [])
            padding = max(max(all_x)-min(all_x), max(all_y)-min(all_y)) * 0.2
            padding = max(padding, 5)  # Minimum padding
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

try:
    print(f"Attempting to connect to {BROKER_ADDRESS}:{BROKER_PORT}...")
    client.connect(BROKER_ADDRESS, BROKER_PORT, 60)
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
