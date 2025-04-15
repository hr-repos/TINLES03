import paho.mqtt.publish as publish
import time

BROKER = "192.168.11.156"  # Change to your broker IP
DELAY = 0.3  # Slightly faster delay for smoother animation

def run_scenario(scenario):
    print(f"\nRunning {scenario['name']}...")
    for topic, payload in scenario["messages"]:
        publish.single(topic, payload, hostname=BROKER)
        print(f"Published to {topic}: {payload}")
        time.sleep(DELAY)

# Mega Scenario: Full Exploration (Combines multiple scenarios)
full_exploration = {
    "name": "Mega Scenario - Full Exploration",
    "messages": [
        # Starting in open space
        ("robot/position", '{"x":0,"y":0}'),
        ("sensors/recalculated", '{"n":50,"e":50,"s":50,"w":50}'),
        
        # Moving north through corridor
        ("robot/position", '{"x":0,"y":10}'),
        ("sensors/recalculated", '{"n":40,"e":20,"s":60,"w":20}'),
        ("robot/position", '{"x":0,"y":20}'),
        ("sensors/recalculated", '{"n":30,"e":20,"s":70,"w":20}'),
        ("robot/position", '{"x":0,"y":30}'),
        ("sensors/recalculated", '{"n":20,"e":20,"s":80,"w":20}'),
        
        # Turning east into a room
        ("robot/position", '{"x":5,"y":30}'),
        ("sensors/recalculated", '{"n":20,"e":15,"s":80,"w":25}'),
        ("robot/position", '{"x":10,"y":30}'),
        ("sensors/recalculated", '{"n":20,"e":10,"s":80,"w":30}'),
        ("robot/position", '{"x":15,"y":30}'),
        ("sensors/recalculated", '{"n":20,"e":5,"s":80,"w":35}'),
        
        # Moving south along east wall
        ("robot/position", '{"x":15,"y":25}'),
        ("sensors/recalculated", '{"n":25,"e":5,"s":75,"w":35}'),
        ("robot/position", '{"x":15,"y":20}'),
        ("sensors/recalculated", '{"n":30,"e":5,"s":70,"w":35}'),
        ("robot/position", '{"x":15,"y":15}'),
        ("sensors/recalculated", '{"n":35,"e":5,"s":65,"w":35}'),
        
        # Moving west through doorway
        ("robot/position", '{"x":10,"y":15}'),
        ("sensors/recalculated", '{"n":35,"e":10,"s":65,"w":30}'),
        ("robot/position", '{"x":5,"y":15}'),
        ("sensors/recalculated", '{"n":35,"e":15,"s":65,"w":25}'),
        ("robot/position", '{"x":0,"y":15}'),
        ("sensors/recalculated", '{"n":35,"e":20,"s":65,"w":20}'),
        
        # Diagonal movement to center
        ("robot/position", '{"x":5,"y":10}'),
        ("sensors/recalculated", '{"n":40,"e":15,"s":60,"w":25}'),
        ("robot/position", '{"x":10,"y":5}'),
        ("sensors/recalculated", '{"n":45,"e":10,"s":55,"w":30}'),
        
        # Final position in open space
        ("robot/position", '{"x":15,"y":0}'),
        ("sensors/recalculated", '{"n":50,"e":5,"s":50,"w":35}')
    ]
}

# Extended Corridor Navigation (Longer version of scenario 5)
extended_corridor = {
    "name": "Extended Corridor Navigation",
    "messages": [
        ("robot/position", '{"x":0,"y":0}'),
        ("sensors/recalculated", '{"n":30,"e":50,"s":30,"w":50}'),
        
        # First segment
        ("robot/position", '{"x":0,"y":10}'),
        ("sensors/recalculated", '{"n":20,"e":50,"s":40,"w":50}'),
        ("robot/position", '{"x":0,"y":20}'),
        ("sensors/recalculated", '{"n":10,"e":50,"s":50,"w":50}'),
        ("robot/position", '{"x":0,"y":30}'),
        ("sensors/recalculated", '{"n":5,"e":50,"s":55,"w":50}'),
        
        # Turn right
        ("robot/position", '{"x":5,"y":30}'),
        ("sensors/recalculated", '{"n":5,"e":45,"s":55,"w":55}'),
        ("robot/position", '{"x":10,"y":30}'),
        ("sensors/recalculated", '{"n":5,"e":40,"s":55,"w":60}'),
        ("robot/position", '{"x":15,"y":30}'),
        ("sensors/recalculated", '{"n":5,"e":35,"s":55,"w":65}'),
        
        # Continue down new corridor
        ("robot/position", '{"x":15,"y":25}'),
        ("sensors/recalculated", '{"n":10,"e":35,"s":50,"w":65}'),
        ("robot/position", '{"x":15,"y":20}'),
        ("sensors/recalculated", '{"n":15,"e":35,"s":45,"w":65}'),
        ("robot/position", '{"x":15,"y":15}'),
        ("sensors/recalculated", '{"n":20,"e":35,"s":40,"w":65}'),
        
        # Final turn
        ("robot/position", '{"x":10,"y":15}'),
        ("sensors/recalculated", '{"n":20,"e":40,"s":40,"w":60}'),
        ("robot/position", '{"x":5,"y":15}'),
        ("sensors/recalculated", '{"n":20,"e":45,"s":40,"w":55}'),
        ("robot/position", '{"x":0,"y":15}'),
        ("sensors/recalculated", '{"n":20,"e":50,"s":40,"w":50}')
    ]
}

# Room Exploration Scenario
room_exploration = {
    "name": "Room Exploration",
    "messages": [
        # Enter room from doorway
        ("robot/position", '{"x":0,"y":0}'),
        ("sensors/recalculated", '{"n":20,"e":10,"s":60,"w":10}'),
        ("robot/position", '{"x":0,"y":10}'),
        ("sensors/recalculated", '{"n":10,"e":10,"s":70,"w":10}'),
        ("robot/position", '{"x":0,"y":20}'),
        ("sensors/recalculated", '{"n":5,"e":1000,"s":75,"w":1000}'),
        
        # Move into room
        ("robot/position", '{"x":5,"y":20}'),
        ("sensors/recalculated", '{"n":5,"e":95,"s":75,"w":95}'),
        ("robot/position", '{"x":10,"y":20}'),
        ("sensors/recalculated", '{"n":5,"e":90,"s":75,"w":90}'),
        
        # Explore left side
        ("robot/position", '{"x":10,"y":25}'),
        ("sensors/recalculated", '{"n":0,"e":90,"s":80,"w":90}'),
        ("robot/position", '{"x":10,"y":30}'),
        ("sensors/recalculated", '{"n":0,"e":90,"s":85,"w":90}'),
        
        # Move to right wall
        ("robot/position", '{"x":20,"y":30}'),
        ("sensors/recalculated", '{"n":0,"e":80,"s":85,"w":100}'),
        ("robot/position", '{"x":25,"y":30}'),
        ("sensors/recalculated", '{"n":0,"e":75,"s":85,"w":105}'),
        
        # Follow wall down
        ("robot/position", '{"x":25,"y":25}'),
        ("sensors/recalculated", '{"n":5,"e":75,"s":80,"w":105}'),
        ("robot/position", '{"x":25,"y":20}'),
        ("sensors/recalculated", '{"n":10,"e":75,"s":75,"w":105}'),
        
        # Return to center
        ("robot/position", '{"x":15,"y":20}'),
        ("sensors/recalculated", '{"n":10,"e":85,"s":75,"w":85}'),
        
        # Exit room
        ("robot/position", '{"x":10,"y":20}'),
        ("sensors/recalculated", '{"n":10,"e":90,"s":75,"w":90}'),
        ("robot/position", '{"x":5,"y":20}'),
        ("sensors/recalculated", '{"n":10,"e":95,"s":75,"w":95}'),
        ("robot/position", '{"x":0,"y":20}'),
        ("sensors/recalculated", '{"n":10,"e":1000,"s":75,"w":1000}')
    ]
}

if __name__ == "__main__":
    print("Enhanced MQTT Scenario Testing Script")
    print("====================================")
    print("Now with larger, more impressive demonstrations!")
    
    scenarios = {
        "1": full_exploration,
        "2": extended_corridor,
        "3": room_exploration
    }
    
    while True:
        print("\nAvailable Enhanced Scenarios:")
        print("1 - Full Exploration (Combined Mega Scenario)")
        print("2 - Extended Corridor Navigation")
        print("3 - Room Exploration")
        print("q - Quit")
        
        choice = input("Select scenario to run (1-3) or 'q' to quit: ").strip().lower()
        
        if choice == 'q':
            break
        elif choice in scenarios:
            run_scenario(scenarios[choice])
        else:
            print("Invalid selection. Please try again.")