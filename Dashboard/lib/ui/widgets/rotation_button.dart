import 'package:flutter/material.dart';

class RotationButton extends StatelessWidget {
  final IconData icon;
  final bool isActive;
  final VoidCallback onTapDown;
  final VoidCallback onTapUp;

  const RotationButton({
    Key? key,
    required this.icon,
    required this.isActive,
    required this.onTapDown,
    required this.onTapUp,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTapDown: (_) => onTapDown(),
      onTapUp: (_) => onTapUp(),
      onTapCancel: onTapUp,
      child: Container(
        width: 60,
        height: 60,
        decoration: BoxDecoration(
          color: isActive ? Colors.blue : Colors.blue.withOpacity(0.5),
          shape: BoxShape.circle,
          border: Border.all(color: Colors.white, width: 2),
        ),
        child: Icon(icon, color: Colors.white, size: 30),
      ),
    );
  }
}
