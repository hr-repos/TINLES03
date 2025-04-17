import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_browser_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'package:flutter/foundation.dart';
import 'dart:math';
import '../models/robot_mapper.dart';
import 'dart:convert';

class MqttService {
  late MqttClient client;
  final RobotMapper mapper;
  final VoidCallback onData; // Callback to notify UI

  MqttService(this.mapper, this.onData);

  Future<void> connect() async {
    const clientId = 'flutter_client';
    final wsUrl = 'ws://192.168.4.1';
    final tcpUrl = '192.168.4.1';

    if (kIsWeb) {
      client = MqttBrowserClient.withPort(wsUrl, clientId, 9001);
    } else {
      client = MqttServerClient(tcpUrl, clientId);
    }

    client.logging(on: true);

    client.onConnected = onConnected;
    client.onDisconnected = onDisconnected;

    final connMessage = MqttConnectMessage()
        .withClientIdentifier(clientId)
        .startClean()
        .withWillQos(MqttQos.atLeastOnce);

    client.connectionMessage = connMessage;

    try {
      await client.connect();
    } catch (e) {
      print("MQTT connection failed: $e");
    }
  }

  void onConnected() {
    print('Connected to MQTT broker');
    client.subscribe('sensors/recalculated', MqttQos.atMostOnce);
    client.subscribe('sensors/uwb', MqttQos.atMostOnce);

    client.updates!.listen((List<MqttReceivedMessage<MqttMessage>> messages) {
      for (var message in messages) {
        _processMessage(message);
      }
    });
  }

  void _processMessage(MqttReceivedMessage<MqttMessage> message) {
    final payload = message.payload as MqttPublishMessage;
    final jsonString =
        MqttPublishPayload.bytesToStringAsString(payload.payload.message);

    try {
      final jsonData = jsonDecode(jsonString);
      if (message.topic == 'sensors/recalculated') {
        mapper.processSensorData(jsonData);
      } else if (message.topic == 'sensors/uwb') {
        final x = jsonData['a']?.toDouble() ?? 0.0;
        final y = jsonData['b']?.toDouble() ?? 0.0;
        print("coords: x: $x, y: $y");
        final coords = calculate_coords(x, y, 1.0);
        print(coords);
        mapper.updatePosition(coords[0], coords[1]);
      } else if (message.topic == 'robot/position') {
        final x = jsonData['a']?.toDouble() ?? 0.0;
        final y = jsonData['b']?.toDouble() ?? 0.0;
        mapper.updatePosition(x, y);
      }

      onData(); // Notify UI to rebuild
    } catch (e) {
      print("Error processing message: $e");
    }
  }

  void sendCommand(String command) {
    final builder = MqttClientPayloadBuilder();
    builder.addString(command);
    client.publishMessage(
        'robot/command', MqttQos.atLeastOnce, builder.payload!);
    print('Sent command: $command');
  }

  void onDisconnected() {
    print('Disconnected from MQTT broker');
  }

  void disconnect() {
    client.disconnect();
  }

  List<double> calculate_coords(
      double anchor_B, double anchor_A, double distance_A_B) {
    double cosA = (anchor_B * anchor_B +
            distance_A_B * distance_A_B -
            anchor_A * anchor_A) /
        (2 * anchor_B * distance_A_B);
    print(cosA);
    double x = anchor_B * cosA;
    double y = anchor_B * sqrt(1 - cosA * cosA);
    print("coords: x: $x, y: $y");

    return [x, y];
  }
}
