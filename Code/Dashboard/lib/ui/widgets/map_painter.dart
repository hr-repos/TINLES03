import 'dart:math';
import 'package:flutter/material.dart';
import '../../models/robot_mapper.dart';

class MapPainter extends CustomPainter {
  final RobotMapper mapper;

  MapPainter(this.mapper);

  @override
  void paint(Canvas canvas, Size size) {
    final allPoints = <Point<double>>[
      ...mapper.northWall,
      ...mapper.eastWall,
      ...mapper.southWall,
      ...mapper.westWall,
      ...mapper.robotPath,
      Point(mapper.robotX, mapper.robotY)
    ];

    if (allPoints.isEmpty) {
      _drawWaiting(canvas, size);
      return;
    }

    final (scale, toCanvas) = _calculateScaleAndTransform(allPoints, size);

    _drawWallLines(canvas, toCanvas);
    _drawRobotPath(canvas, toCanvas);
    _drawRobot(canvas, toCanvas);
  }

  (double, Offset Function(Point<double>)) _calculateScaleAndTransform(
      List<Point<double>> allPoints, Size size) {
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

    return (scale, toCanvas);
  }

  void _drawWallLines(Canvas canvas, Offset Function(Point<double>) toCanvas) {
    _drawWallLine(canvas, mapper.northWall, Colors.red, toCanvas);
    _drawWallLine(canvas, mapper.eastWall, Colors.green, toCanvas);
    _drawWallLine(canvas, mapper.southWall, Colors.blue, toCanvas);
    _drawWallLine(canvas, mapper.westWall, Colors.yellow, toCanvas);
  }

  void _drawWallLine(Canvas canvas, List<Point<double>> points, Color color,
      Offset Function(Point<double>) toCanvas) {
    if (points.length < 2) return;

    final wallPaint = Paint()
      ..color = color
      ..strokeWidth = 3
      ..style = PaintingStyle.stroke;

    final path = Path();
    path.moveTo(toCanvas(points.first).dx, toCanvas(points.first).dy);

    for (final point in points.skip(1)) {
      path.lineTo(toCanvas(point).dx, toCanvas(point).dy);
    }

    canvas.drawPath(path, wallPaint);
  }

  void _drawRobotPath(Canvas canvas, Offset Function(Point<double>) toCanvas) {
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
  }

  void _drawRobot(Canvas canvas, Offset Function(Point<double>) toCanvas) {
    final robotPos = toCanvas(Point(mapper.robotX, mapper.robotY));
    final robotPaint = Paint()..color = Colors.green;
    canvas.drawCircle(robotPos, 10, robotPaint);

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
