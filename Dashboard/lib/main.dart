import 'dart:async';
import 'dart:convert';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:flutter_joystick/flutter_joystick.dart';
import 'package:mqtt_client/mqtt_browser_client.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'package:flutter/foundation.dart';

// Command constants
const _commandForward = 'w';
const _commandBackward = 's';
const _commandLeft = 'a';
const _commandRight = 'd';
const _commandRotateLeft = 'q';
const _commandRotateRight = 'e';
const _commandStop = ' ';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  bool showMap = true;
  late MqttClient client;
  late RobotMapper mapper;

  String _lastCommand = '';
  Timer? _commandTimer;
  bool _joystickActive = false;

  @override
  void initState() {
    super.initState();
    mapper = RobotMapper();
    _connectMQTT();
  }

  Future<void> _connectMQTT() async {
    const clientId = 'flutter_client';
    final wsUrl = 'ws://192.168.4.1';
    final tcpUrl = '192.168.4.1';

    if (kIsWeb) {
      client = MqttBrowserClient.withPort(wsUrl, clientId, 9001);
    } else {
      client = MqttServerClient(tcpUrl, clientId);
    }
    client.logging(on: true);

    client.onConnected = _onConnected;
    client.onDisconnected = _onDisconnected;

    final connMessage = MqttConnectMessage()
        .withClientIdentifier(clientId)
        .startClean()
        .withWillQos(MqttQos.atLeastOnce);

    client.connectionMessage = connMessage;

    try {
      await client.connect();
    } catch (e) {
      print("MQTT connection to port 9001 failed: $e");
    }
  }

  void _onConnected() {
    client.subscribe('sensors/recalculated', MqttQos.atMostOnce);
    client.subscribe('robot/position', MqttQos.atMostOnce);
    client.updates!.listen((List<MqttReceivedMessage<MqttMessage>> messages) {
      for (var message in messages) {
        final payload = message.payload as MqttPublishMessage;
        final jsonString =
            MqttPublishPayload.bytesToStringAsString(payload.payload.message);

        try {
          final jsonData = jsonDecode(jsonString);
          if (message.topic == 'sensors/recalculated') {
            mapper.processSensorData(jsonData);
          } else if (message.topic == 'robot/position') {
            final x = jsonData['x']?.toDouble() ?? 0.0;
            final y = jsonData['y']?.toDouble() ?? 0.0;
            mapper.updatePosition(x, y);
          }
          setState(() {});
        } catch (e) {
          print("Error processing message: $e");
        }
      }
    });
  }

  void _onDisconnected() {
    print('Disconnected from MQTT broker');
  }

  void _sendMovementCommand(String command) {
    if (command == _lastCommand) return;

    _lastCommand = command;
    final builder = MqttClientPayloadBuilder();
    builder.addString(command);
    client.publishMessage(
        'robot/command', MqttQos.atLeastOnce, builder.payload!);

    print('Sent command: $command');
  }

  void _handleJoystick(StickDragDetails details) {
    // Virtual movement
    mapper.moveRobot(details.x * 5, details.y * 5);
    setState(() {});

    // Determine real command
    String newCommand = _commandStop;
    final deadZone = 0.2;

    if (details.x.abs() > deadZone || details.y.abs() > deadZone) {
      _joystickActive = true;

      // Forward/backward takes priority
      if (details.y < -deadZone) {
        newCommand = _commandForward;
      } else if (details.y > deadZone) {
        newCommand = _commandBackward;
      }
      // Left/right if not moving forward/backward
      else if (details.x < -deadZone) {
        newCommand = _commandLeft;
      } else if (details.x > deadZone) {
        newCommand = _commandRight;
      }
    } else {
      if (_joystickActive) {
        _joystickActive = false;
        newCommand = _commandStop;
      }
    }

    // Send command immediately when changed
    if (newCommand != _lastCommand) {
      _sendMovementCommand(newCommand);
    }

    // Setup periodic sending while joystick is active
    _commandTimer?.cancel();
    if (_joystickActive) {
      _commandTimer =
          Timer.periodic(const Duration(milliseconds: 500), (timer) {
        _sendMovementCommand(newCommand);
      });
    }
  }

  @override
  void dispose() {
    _commandTimer?.cancel();
    client.disconnect();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: Scaffold(
        backgroundColor: Colors.black,
        body: Center(
          child: _buildDashboard(),
        ),
      ),
    );
  }

  Widget _buildDashboard() {
    final screenSize = MediaQuery.of(context).size;

    return Container(
      width: screenSize.width,
      height: screenSize.height,
      child: Stack(
        children: [
          // Map Container
          Container(
            width: screenSize.width * 0.85,
            height: screenSize.height * 0.7,
            margin: const EdgeInsets.only(top: 20),
            decoration: const BoxDecoration(
              border: Border(
                top: BorderSide(color: Colors.white),
                left: BorderSide(color: Colors.white),
                right: BorderSide(color: Colors.white),
              ),
            ),
            child: showMap ? _buildMapWidget() : _buildCameraPlaceholder(),
          ),

          Positioned(
            bottom: 20,
            left: screenSize.width * 0.45,
            child: Joystick(
              listener: _handleJoystick,
            ),
          ),

          Positioned(
            top: 50,
            right: 20,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                _buildLabel('N: ${mapper.north.toStringAsFixed(2)} cm'),
                _buildLabel('E: ${mapper.east.toStringAsFixed(2)} cm'),
                _buildLabel('S: ${mapper.south.toStringAsFixed(2)} cm'),
                _buildLabel('W: ${mapper.west.toStringAsFixed(2)} cm'),
                const SizedBox(height: 20),
                _buildLabel('X: ${mapper.robotX.toStringAsFixed(2)}'),
                _buildLabel('Y: ${mapper.robotY.toStringAsFixed(2)}'),
                _buildLabel(
                    'Orientation: ${mapper.orientation.toStringAsFixed(0)}°'),
                const SizedBox(height: 20),
                _buildLabel('Last Command: $_lastCommand'),
              ],
            ),
          ),

          Positioned(
            bottom: 20,
            right: 20,
            child: ElevatedButton(
              onPressed: () => setState(() => showMap = !showMap),
              child: Text(showMap ? 'Show Camera' : 'Show Map'),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildMapWidget() {
    return LayoutBuilder(
      builder: (context, constraints) {
        return CustomPaint(
          size: Size(constraints.maxWidth, constraints.maxHeight),
          painter: MapPainter(mapper),
        );
      },
    );
  }

  Widget _buildCameraPlaceholder() {
    return const Center(
      child: Text('CAMERA PLACEHOLDER',
          style: TextStyle(color: Colors.white, fontSize: 28)),
    );
  }

  Widget _buildLabel(String text) {
    return Text(text,
        style: const TextStyle(color: Colors.white, fontSize: 18));
  }
}

class RobotMapper {
  final Map<Point<double>, List<String>> obstacles = {};
  final List<Point<double>> robotPath = [];
  final List<Point<double>> allObstacles = [];

  double robotX = 0;
  double robotY = 0;
  double orientation = 0;

  double north = 0;
  double east = 0;
  double south = 0;
  double west = 0;

  void updatePosition(double x, double y) {
    robotX = x;
    robotY = y;
    robotPath.add(Point(x, y));
  }

  void processSensorData(Map<String, dynamic> data) {
    north = data['n']?.toDouble() ?? north;
    east = data['e']?.toDouble() ?? east;
    south = data['s']?.toDouble() ?? south;
    west = data['w']?.toDouble() ?? west;

    obstacles.clear();

    final rad = orientation * (pi / 180);
    final cosVal = cos(rad);
    final sinVal = sin(rad);

    if (north > 0) {
      final x = robotX + (-north * sinVal);
      final y = robotY + (north * cosVal);
      final point = Point(x, y);
      obstacles[point] = ['north'];
      allObstacles.add(point);
    }

    if (east > 0) {
      final x = robotX + (east * cosVal);
      final y = robotY + (east * sinVal);
      final point = Point(x, y);
      obstacles[point] = ['east'];
      allObstacles.add(point);
    }

    if (south > 0) {
      final x = robotX + (south * sinVal);
      final y = robotY + (-south * cosVal);
      final point = Point(x, y);
      obstacles[point] = ['south'];
      allObstacles.add(point);
    }

    if (west > 0) {
      final x = robotX + (-west * cosVal);
      final y = robotY + (-west * sinVal);
      final point = Point(x, y);
      obstacles[point] = ['west'];
      allObstacles.add(point);
    }
  }

  void moveRobot(double dx, double dy) {
    robotX += dx;
    robotY += dy;
    robotPath.add(Point(robotX, robotY));
  }
}

class MapPainter extends CustomPainter {
  final RobotMapper mapper;

  MapPainter(this.mapper);

  @override
  void paint(Canvas canvas, Size size) {
    final allPoints = [
      ...mapper.allObstacles,
      ...mapper.robotPath,
      Point(mapper.robotX, mapper.robotY)
    ];

    if (allPoints.isEmpty) {
      _drawWaiting(canvas, size);
      return;
    }

    double minX = allPoints.map((p) => p.x).reduce(min);
    double maxX = allPoints.map((p) => p.x).reduce(max);
    double minY = allPoints.map((p) => p.y).reduce(min);
    double maxY = allPoints.map((p) => p.y).reduce(max);

    double padding = max(maxX - minX, maxY - minY) * 0.2;
    padding = max(padding, 5.0);
    minX -= padding;
    maxX += padding;
    minY -= padding;
    maxY += padding;

    final scaleX = size.width / (maxX - minX);
    final scaleY = size.height / (maxY - minY);
    final scale = min(scaleX, scaleY) * 0.9;

    Offset toCanvas(Point<double> point) {
      return Offset(
        (point.x - minX) * scale,
        size.height - (point.y - minY) * scale,
      );
    }

    // Draw all historical obstacles
    final obstaclePaint = Paint()..color = Colors.red.withOpacity(0.5);
    for (final obstacle in mapper.allObstacles) {
      canvas.drawCircle(toCanvas(obstacle), 5, obstaclePaint);
    }

    // Draw current obstacles more prominently
    final currentObstaclePaint = Paint()..color = Colors.red;
    for (final obstacle in mapper.obstacles.keys) {
      canvas.drawCircle(toCanvas(obstacle), 8, currentObstaclePaint);
    }

    // Draw robot path
    if (mapper.robotPath.length > 1) {
      final pathPaint = Paint()
        ..color = Colors.blue.withOpacity(0.5)
        ..strokeWidth = 2
        ..style = PaintingStyle.stroke;

      final path = Path();
      path.moveTo(toCanvas(mapper.robotPath.first).dx,
          toCanvas(mapper.robotPath.first).dy);

      for (final point in mapper.robotPath.skip(1)) {
        path.lineTo(toCanvas(point).dx, toCanvas(point).dy);
      }

      canvas.drawPath(path, pathPaint);
    }

    // Draw robot
    final robotPos = toCanvas(Point(mapper.robotX, mapper.robotY));
    final robotPaint = Paint()..color = Colors.green;
    canvas.drawCircle(robotPos, 10, robotPaint);

    // Draw orientation
    final orientationRad = mapper.orientation * (pi / 180);
    final orientationPaint = Paint()
      ..color = Colors.white
      ..strokeWidth = 2;

    final endPos = Offset(
      robotPos.dx + 20 * cos(orientationRad),
      robotPos.dy - 20 * sin(orientationRad),
    );
    canvas.drawLine(robotPos, endPos, orientationPaint);
  }

  void _drawWaiting(Canvas canvas, Size size) {
    final text = TextSpan(
      text: 'Waiting for robot data...',
      style: TextStyle(color: Colors.white, fontSize: 20),
    );

    final textPainter = TextPainter(
      text: text,
      textDirection: TextDirection.ltr,
    );

    textPainter.layout();
    textPainter.paint(
      canvas,
      Offset(size.width / 2 - textPainter.width / 2, size.height / 2),
    );
  }

  @override
  bool shouldRepaint(MapPainter oldDelegate) => true;
}
