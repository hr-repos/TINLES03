import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_joystick/flutter_joystick.dart';
// import 'package:camera/camera.dart';
import 'dart:math';
import 'package:mqtt_client/mqtt_browser_client.dart'; // For Web
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart'; // For mobile
import 'package:flutter/foundation.dart'; // To check for platform

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  // final cameras = await availableCameras();
  // final firstCamera = cameras[2];
  runApp(MyApp(
      // camera: firstCamera,
      ));
}

class MyApp extends StatefulWidget {
  // final CameraDescription camera;
  MyApp();

  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  bool showMap = true;
  bool isCameraPage = false;
  double joystickX = 0.0;
  double joystickY = 0.0;
  // CameraController? _cameraController;
  // Future<void>? _initializeControllerFuture;

  double north = 1083.09;
  double east = 686.97;
  double south = 77.24;
  double west = 41.87;
  double x = 12.3;
  double y = 8.6;
  double z = 14.2;

  late dynamic client;

  @override
  void initState() {
    super.initState();
    // _cameraController =
    //     CameraController(widget.camera, ResolutionPreset.medium);
    // _initializeControllerFuture =
    //     _cameraController!.initialize().catchError((e) {
    //   print("Camera initialization error: $e");
    // });

    _connectMQTT();
  }

  @override
  void dispose() {
    // _cameraController?.dispose();
    if (client != null) {
      client.disconnect();
    }
    super.dispose();
  }

  // Connect to MQTT with fallback
  Future<void> _connectMQTT() async {
    final wsUrl = 'ws://192.168.11.70';
    final tcpUrl = '192.168.11.70';

    if (kIsWeb) {
      client = MqttBrowserClient.withPort(wsUrl, 'flutter_client', 9001);
    } else {
      client = MqttServerClient(tcpUrl, 'flutter_client');
    }

    client.logging(on: true);
    client.onConnected = _onConnected;
    client.onDisconnected = _onDisconnected;
    client.onSubscribed = _onSubscribed;
    client.pongCallback = _pong;

    final connMessage = MqttConnectMessage()
        .withClientIdentifier('flutter_client')
        .startClean();
    client.connectionMessage = connMessage;

    try {
      await client.connect();
    } catch (e) {
      print("MQTT connection to port 9001 failed: $e");
      // Fallback attempt on port 1883
      if (kIsWeb) {
        print("Web platform: skipping TCP fallback");
      } else {
        print("Trying fallback on port 1883...");
        client = MqttServerClient(tcpUrl, 'flutter_client');
        client.logging(on: true);
        client.onConnected = _onConnected;
        client.onDisconnected = _onDisconnected;
        client.onSubscribed = _onSubscribed;
        client.pongCallback = _pong;
        client.port = 1883;
        client.connectionMessage = connMessage;
        try {
          await client.connect();
        } catch (e) {
          print("MQTT fallback connection failed: $e");
        }
      }
    }
  }

  void _onConnected() {
    print('Connected to MQTT broker');
    client.subscribe('sensors/recalculated', MqttQos.atMostOnce);
  }

  void _onDisconnected() {
    print('Disconnected from MQTT broker');
  }

  void _onSubscribed(String topic) {
    print('Subscribed to $topic');
  }

  void _pong() {
    print('Ping response received');
  }

  void _updateSensorValues(Map<String, dynamic> jsonData) {
    setState(() {
      north = jsonData['n'] ?? north;
      east = jsonData['e'] ?? east;
      south = jsonData['s'] ?? south;
      west = jsonData['w'] ?? west;
    });
  }

  void _onMessageReceived(MqttMessage message) {
    try {
      final payload = message as MqttPublishMessage;
      final jsonString =
          MqttPublishPayload.bytesToStringAsString(payload.payload.message);
      print("Received payload: $jsonString");
      final jsonData = json.decode(jsonString);
      if (jsonData is Map<String, dynamic>) {
        _updateSensorValues(jsonData);
      } else {
        print("Invalid JSON structure: $jsonData");
      }
    } catch (e) {
      print("Error processing message: $e");
    }
  }

  @override
  Widget build(BuildContext context) {
    client.updates!.listen((List<MqttReceivedMessage<MqttMessage>>? messages) {
      for (var message in messages!) {
        _onMessageReceived(message.payload);
      }
    });

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
    final screenWidth = MediaQuery.of(context).size.width;
    final screenHeight = MediaQuery.of(context).size.height;

    return Container(
      width: screenWidth,
      height: screenHeight,
      decoration: BoxDecoration(
        color: Colors.black,
        border: Border.all(color: Colors.white),
        borderRadius: BorderRadius.circular(10),
      ),
      child: Stack(
        children: [
          Container(
            width: screenWidth * 0.85,
            height: screenHeight * 0.7,
            margin: EdgeInsets.only(top: 20),
            decoration: BoxDecoration(
              border: Border(
                top: BorderSide(color: Colors.white, width: 2),
                left: BorderSide(color: Colors.white, width: 2),
                right: BorderSide(color: Colors.white, width: 2),
                bottom: BorderSide(color: Colors.white, width: 6),
              ),
              borderRadius: BorderRadius.circular(12),
            ),
            child: Center(
              child: Text('MAP',
                  style: TextStyle(color: Colors.white, fontSize: 28)),
            ),
          ),
          Positioned(
            bottom: -10,
            left: screenWidth * 0.45,
            child: Joystick(
              listener: (details) {
                setState(() {
                  joystickX = details.x;
                  joystickY = details.y;
                });
                print("Joystick X: $joystickX, Y: $joystickY");
              },
            ),
          ),
          Positioned(
            bottom: 65,
            left: screenWidth * 0.35,
            child: GestureDetector(
              onTap: () {
                print('Left triangle pressed');
              },
              child: CustomPaint(
                size: Size(50, 50),
                painter: TrianglePainter(color: Colors.white, angle: 270),
              ),
            ),
          ),
          Positioned(
            bottom: 65,
            right: screenWidth * 0.32,
            child: GestureDetector(
              onTap: () {
                print('Right triangle pressed');
              },
              child: CustomPaint(
                size: Size(50, 50),
                painter: TrianglePainter(color: Colors.white, angle: 90),
              ),
            ),
          ),
          Positioned(
            top: 50,
            right: 20,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                _buildLabel('N: ${north} cm', fontSize: 18),
                _buildLabel('E: ${east} cm', fontSize: 18),
                _buildLabel('S: ${south} cm', fontSize: 18),
                _buildLabel('W: ${west} cm', fontSize: 18),
                SizedBox(height: 20),
                _buildLabel('X: $x', fontSize: 18),
                _buildLabel('Y: $y', fontSize: 18),
                _buildLabel('Z: $z', fontSize: 18),
              ],
            ),
          ),
          Positioned(
            bottom: 20,
            right: 20,
            child: ElevatedButton(
              onPressed: () {
                setState(() {
                  isCameraPage = !isCameraPage;
                  showMap = !showMap;
                });
                // if (!showMap && _cameraController != null) {
                //   _cameraController!.dispose();
                //   _cameraController =
                //       CameraController(widget.camera, ResolutionPreset.medium);
                //   _initializeControllerFuture = _cameraController!.initialize();
                // }
              },
              child: Text(isCameraPage ? 'Show Map' : 'Show Camera'),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildLabel(String text, {double fontSize = 14}) {
    return Text(
      text,
      style: TextStyle(color: Colors.white, fontSize: fontSize),
    );
  }
}

class TrianglePainter extends CustomPainter {
  final Color color;
  final double angle;

  TrianglePainter({required this.color, required this.angle});

  @override
  void paint(Canvas canvas, Size size) {
    final Paint paint = Paint()..color = color;
    final Path path = Path();

    final double width = size.width;
    final double height = size.height;

    path.moveTo(width / 2, 0);

    if (angle == 90) {
      path.lineTo(width, height);
      path.lineTo(0, height);
    } else if (angle == 270) {
      path.lineTo(0, height);
      path.lineTo(width, height);
    }

    path.close();

    canvas.save();
    canvas.translate(width / 2, height / 2);
    canvas.rotate(angle * pi / 180);
    canvas.translate(-width / 2, -height / 2);

    canvas.drawPath(path, paint);
    canvas.restore();
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) {
    return false;
  }
}
