import 'package:flutter/material.dart';
import '../models/robot_mapper.dart';
import '../mqtt/mqtt_service.dart';
import 'widgets/joystick_controls.dart';
import 'widgets/map_painter.dart'; // Add this import
import 'widgets/status_labels.dart';

class DashboardScreen extends StatefulWidget {
  @override
  _DashboardScreenState createState() => _DashboardScreenState();
}

_DashboardScreenState createState() => _DashboardScreenState();

class _DashboardScreenState extends State<DashboardScreen> {
  late MqttService _mqttService;
  late RobotMapper _mapper;
  String _lastCommand = '';

  @override
  void initState() {
    super.initState();
    _mapper = RobotMapper();
    _mqttService = MqttService(_mapper, () {
      if (mounted) setState(() {}); // Triggers UI rebuild
    });
    _mqttService.connect();
  }

  @override
  void dispose() {
    _mqttService.disconnect();
    super.dispose();
  }

  void _sendCommand(String command) {
    if (command == _lastCommand) return;
    _lastCommand = command;
    _mqttService.sendCommand(command);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      body: Center(
        child: _buildDashboard(),
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
            child: _buildMapWidget(),
          ),

          // Controls
          Positioned(
            bottom: 20,
            left: screenSize.width * 0.35,
            child: JoystickControls(
              onCommand: _sendCommand,
            ),
          ),

          // Status labels
          Positioned(
            top: 50,
            right: 20,
            child: StatusLabels(
              mapper: _mapper,
              lastCommand: _lastCommand,
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
          painter: MapPainter(_mapper),
        );
      },
    );
  }
}
