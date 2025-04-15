import 'package:flutter/material.dart';
import '../../models/robot_mapper.dart';

class StatusLabels extends StatelessWidget {
  final RobotMapper mapper;
  final String lastCommand;

  const StatusLabels({
    Key? key,
    required this.mapper,
    required this.lastCommand,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        _buildLabel('N: ${mapper.north.toStringAsFixed(2)} cm'),
        _buildLabel('E: ${mapper.east.toStringAsFixed(2)} cm'),
        _buildLabel('S: ${mapper.south.toStringAsFixed(2)} cm'),
        _buildLabel('W: ${mapper.west.toStringAsFixed(2)} cm'),
        const SizedBox(height: 20),
        _buildLabel('X: ${mapper.robotX.toStringAsFixed(2)}'),
        _buildLabel('Y: ${mapper.robotY.toStringAsFixed(2)}'),
        _buildLabel('Orientation: ${mapper.orientation.toStringAsFixed(0)}°'),
        const SizedBox(height: 20),
        _buildLabel('Last Command: $lastCommand'),
      ],
    );
  }

  Widget _buildLabel(String text) {
    return Text(text,
        style: const TextStyle(color: Colors.white, fontSize: 18));
  }
}
