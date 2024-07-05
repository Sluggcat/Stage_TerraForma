String sendMessage;
String nextMessage;
String receivedMessage;
unsigned int time;

void setup() {
  Serial.begin(9600);    // Initialize the Serial monitor for debugging
  Serial1.begin(9600);   // Initialize Serial1 for sending data

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  while (Serial1.available() > 0) {
    char receivedChar = Serial1.read();
    if (receivedChar == '\n') {
      Serial.println("Message received : \"" + receivedMessage + "\"\n");  // Print the received message in the Serial monitor
      receivedMessage = "";  // Reset the received message
    } else {
      receivedMessage += receivedChar;  // Append characters to the received message
    }
  }

  if (Serial.available() > 0) {
    char inputChar = Serial.read();
    if (inputChar == '\n') {
      //Serial1.println(sendMessage);  // Send the message through Serial1 with a newline character
      //Serial.print("Message send : \"" + sendMessage + "\"\n");
      sendMessage = nextMessage;
      nextMessage = "";  // Reset the message
    } else {
      nextMessage += inputChar;  // Append characters to the message
    }
  }

  if (time % 2000 == 0) {
      Serial1.println(sendMessage);  // Send the message through Serial1 with a newline character
      //Serial.print("Message send : \"" + sendMessage + "\"\n");
      //Serial.print(Serial1.readBytes());
  }


  if (time % 250 == 0) {
    if (digitalRead(LED_BUILTIN) == 1)
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    else
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on by making the voltage HIGH
    if (time >= 100000)
      time = 0;
  }
  time++;
  delay(1);
}