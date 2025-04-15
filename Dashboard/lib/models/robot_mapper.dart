import 'dart:math';

class RobotMapper {
  // Change from private to public by removing underscores
  final List<Point<double>> northWall = [];
  final List<Point<double>> eastWall = [];
  final List<Point<double>> southWall = [];
  final List<Point<double>> westWall = [];
  final List<Point<double>> robotPath = [];

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

    final rad = orientation * (pi / 180);
    final cosVal = cos(rad);
    final sinVal = sin(rad);

    if (north > 0) {
      final x = robotX + (-north * sinVal);
      final y = robotY + (north * cosVal);
      northWall.add(Point(x, y));
    }

    if (east > 0) {
      final x = robotX + (east * cosVal);
      final y = robotY + (east * sinVal);
      eastWall.add(Point(x, y));
    }

    if (south > 0) {
      final x = robotX + (south * sinVal);
      final y = robotY + (-south * cosVal);
      southWall.add(Point(x, y));
    }

    if (west > 0) {
      final x = robotX + (-west * cosVal);
      final y = robotY + (-west * sinVal);
      westWall.add(Point(x, y));
    }
  }

  void moveRobot(double dx, double dy) {
    robotX += dx;
    robotY += dy;
    robotPath.add(Point(robotX, robotY));
  }
}
