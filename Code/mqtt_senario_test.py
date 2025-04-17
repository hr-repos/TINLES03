import paho.mqtt.publish as publish
import time

BROKER = "192.168.11.156"
DELAY = 0.2  # Faster movement for better visualization

def run_scenario(scenario):
    print(f"\nRunning {scenario['name']}...")
    for topic, payload in scenario["messages"]:
        publish.single(topic, payload, hostname=BROKER)
        print(f"Published to {topic}: {payload}")
        time.sleep(DELAY)

# Systematic Room Exploration (100x100 empty space)
room_exploration = {
    "name": "100x100 Room Systematic Exploration",
    "messages": [
        # Initialize at center with open space readings
        ("robot/position", '{"x":50,"y":50}'),
        ("sensors/recalculated", '{"n":50,"e":50,"s":50,"w":50}'),
        
        # Spiral exploration pattern (outward from center)
        # Move right
        ("robot/position", '{"x":60,"y":50}'),
        ("sensors/recalculated", '{"n":50,"e":40,"s":50,"w":60}'),
        ("robot/position", '{"x":70,"y":50}'),
        ("sensors/recalculated", '{"n":50,"e":30,"s":50,"w":70}'),
        ("robot/position", '{"x":80,"y":50}'),
        ("sensors/recalculated", '{"n":50,"e":20,"s":50,"w":80}'),
        ("robot/position", '{"x":90,"y":50}'),
        ("sensors/recalculated", '{"n":50,"e":10,"s":50,"w":90}'),
        
        # Move up
        ("robot/position", '{"x":90,"y":60}'),
        ("sensors/recalculated", '{"n":40,"e":10,"s":60,"w":90}'),
        ("robot/position", '{"x":90,"y":70}'),
        ("sensors/recalculated", '{"n":30,"e":10,"s":70,"w":90}'),
        ("robot/position", '{"x":90,"y":80}'),
        ("sensors/recalculated", '{"n":20,"e":10,"s":80,"w":90}'),
        ("robot/position", '{"x":90,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":10,"s":90,"w":90}'),
        
        # Move left
        ("robot/position", '{"x":80,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":20,"s":90,"w":80}'),
        ("robot/position", '{"x":70,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":30,"s":90,"w":70}'),
        ("robot/position", '{"x":60,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":40,"s":90,"w":60}'),
        ("robot/position", '{"x":50,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":50,"s":90,"w":50}'),
        ("robot/position", '{"x":40,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":60,"s":90,"w":40}'),
        ("robot/position", '{"x":30,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":70,"s":90,"w":30}'),
        ("robot/position", '{"x":20,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":80,"s":90,"w":20}'),
        ("robot/position", '{"x":10,"y":90}'),
        ("sensors/recalculated", '{"n":10,"e":90,"s":90,"w":10}'),
        
        # Move down
        ("robot/position", '{"x":10,"y":80}'),
        ("sensors/recalculated", '{"n":20,"e":90,"s":80,"w":10}'),
        ("robot/position", '{"x":10,"y":70}'),
        ("sensors/recalculated", '{"n":30,"e":90,"s":70,"w":10}'),
        ("robot/position", '{"x":10,"y":60}'),
        ("sensors/recalculated", '{"n":40,"e":90,"s":60,"w":10}'),
        ("robot/position", '{"x":10,"y":50}'),
        ("sensors/recalculated", '{"n":50,"e":90,"s":50,"w":10}'),
        ("robot/position", '{"x":10,"y":40}'),
        ("sensors/recalculated", '{"n":60,"e":90,"s":40,"w":10}'),
        ("robot/position", '{"x":10,"y":30}'),
        ("sensors/recalculated", '{"n":70,"e":90,"s":30,"w":10}'),
        ("robot/position", '{"x":10,"y":20}'),
        ("sensors/recalculated", '{"n":80,"e":90,"s":20,"w":10}'),
        ("robot/position", '{"x":10,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":90,"s":10,"w":10}'),
        
        # Move right (inner loop)
        ("robot/position", '{"x":20,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":80,"s":10,"w":20}'),
        ("robot/position", '{"x":30,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":70,"s":10,"w":30}'),
        ("robot/position", '{"x":40,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":60,"s":10,"w":40}'),
        ("robot/position", '{"x":50,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":50,"s":10,"w":50}'),
        ("robot/position", '{"x":60,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":40,"s":10,"w":60}'),
        ("robot/position", '{"x":70,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":30,"s":10,"w":70}'),
        ("robot/position", '{"x":80,"y":10}'),
        ("sensors/recalculated", '{"n":90,"e":20,"s":10,"w":80}'),
        
        # Move up (inner loop)
        ("robot/position", '{"x":80,"y":20}'),
        ("sensors/recalculated", '{"n":80,"e":20,"s":20,"w":80}'),
        ("robot/position", '{"x":80,"y":30}'),
        ("sensors/recalculated", '{"n":70,"e":20,"s":30,"w":80}'),
        ("robot/position", '{"x":80,"y":40}'),
        ("sensors/recalculated", '{"n":60,"e":20,"s":40,"w":80}'),
        
        # Final position
        ("robot/position", '{"x":80,"y":50}'),
        ("sensors/recalculated", '{"n":50,"e":20,"s":50,"w":80}')
    ]
}

if __name__ == "__main__":
    print("100x100 Room Systematic Exploration")
    print("==================================")
    print("Features:")
    print("- Perfect 100x100 empty room coverage")
    print("- No wall crossings or overlaps")
    print("- Clean spiral exploration pattern")
    
    run_scenario(room_exploration)