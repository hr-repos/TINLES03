import 'dart:async'; // Add this import for Timer
import 'package:flutter/material.dart';
import 'package:flutter_joystick/flutter_joystick.dart';
import '../../../utils/constants.dart'; // Import constants
import 'rotation_button.dart'; // Import RotationButton widget

class JoystickControls extends StatefulWidget {
  final Function(String) onCommand;

  const JoystickControls({Key? key, required this.onCommand}) : super(key: key);

  @override
  _JoystickControlsState createState() => _JoystickControlsState();
}

class _JoystickControlsState extends State<JoystickControls> {
  bool _isRotatingLeft = false;
  bool _isRotatingRight = false;
  Timer? _rotationTimer;
  Timer? _commandTimer;
  bool _joystickActive = false;

  void _handleJoystick(StickDragDetails details) {
    String newCommand = commandStop;
    final deadZone = 0.2;

    if (details.x.abs() > deadZone || details.y.abs() > deadZone) {
      _joystickActive = true;

      if (details.y < -deadZone) {
        newCommand = commandForward;
      } else if (details.y > deadZone) {
        newCommand = commandBackward;
      } else if (details.x < -deadZone) {
        newCommand = commandLeft;
      } else if (details.x > deadZone) {
        newCommand = commandRight;
      }
    } else {
      if (_joystickActive) {
        _joystickActive = false;
        newCommand = commandStop;
      }
    }

    widget.onCommand(newCommand);

    _commandTimer?.cancel();
    if (_joystickActive) {
      _commandTimer =
          Timer.periodic(const Duration(milliseconds: 500), (timer) {
        widget.onCommand(newCommand);
      });
    }
  }

  void _startRotation(String command) {
    setState(() {
      if (command == commandRotateLeft) {
        _isRotatingLeft = true;
      } else {
        _isRotatingRight = true;
      }
    });

    widget.onCommand(command);

    _rotationTimer = Timer.periodic(const Duration(milliseconds: 200), (timer) {
      widget.onCommand(command);
    });
  }

  void _stopRotation() {
    _rotationTimer?.cancel();
    setState(() {
      _isRotatingLeft = false;
      _isRotatingRight = false;
    });
    widget.onCommand(commandStop);
  }

  @override
  void dispose() {
    _commandTimer?.cancel();
    _rotationTimer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        RotationButton(
          icon: Icons.rotate_left,
          isActive: _isRotatingLeft,
          onTapDown: () => _startRotation(commandRotateLeft),
          onTapUp: _stopRotation,
        ),
        const SizedBox(width: 20),
        Joystick(listener: _handleJoystick),
        const SizedBox(width: 20),
        RotationButton(
          icon: Icons.rotate_right,
          isActive: _isRotatingRight,
          onTapDown: () => _startRotation(commandRotateRight),
          onTapUp: _stopRotation,
        ),
      ],
    );
  }
}
